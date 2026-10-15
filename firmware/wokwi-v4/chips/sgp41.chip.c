// Wokwi custom chip: Sensirion SGP41 VOC/NOx sensor at I2C 0x59.
// Commands: 0x280E self test, 0x2612 execute conditioning (6 param bytes),
//   0x2619 measure raw signals (6 param bytes), 0x3615 turn heater off.
// Scripted raw signals: sraw_voc = 26000 + 10*(n%50), sraw_nox = 15000 + 3*(n%30).
#include "wokwi-api.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint8_t rx[16];
  uint32_t rx_len;
  uint16_t cmd;
  uint8_t resp[16];
  uint32_t resp_len;
  uint32_t resp_pos;
  uint32_t n;
} chip_t;

#include "sensirion_common.h"

static void put_word(chip_t *c, uint16_t w) {
  sensirion_put_word(c->resp, &c->resp_len, w);
}

static void prepare_response(chip_t *c) {
  c->resp_len = 0;
  c->resp_pos = 0;
  switch (c->cmd) {
    case 0x280E:  // self test: 0xD400 = all sensors ok
      put_word(c, 0xD400);
      break;
    case 0x2612:  // conditioning: returns sraw_voc only
      put_word(c, (uint16_t)(26000 + 10 * (c->n % 50)));
      break;
    case 0x2619:  // measure raw: sraw_voc, sraw_nox
      put_word(c, (uint16_t)(26000 + 10 * (c->n % 50)));
      put_word(c, (uint16_t)(15000 + 3 * (c->n % 30)));
      c->n++;
      break;
    default:
      put_word(c, 0x0000);
      break;
  }
}

static bool on_connect(void *user_data, uint32_t address, bool read) {
  chip_t *c = (chip_t *)user_data;
  if (read) {
    prepare_response(c);
  } else {
    c->rx_len = 0;
  }
  return true;
}

static bool on_write(void *user_data, uint8_t data) {
  chip_t *c = (chip_t *)user_data;
  if (c->rx_len < sizeof(c->rx)) c->rx[c->rx_len++] = data;
  if (c->rx_len == 2) c->cmd = (uint16_t)((c->rx[0] << 8) | c->rx[1]);
  return true;  // ACK (param words after the command are accepted + ignored)
}

static uint8_t on_read(void *user_data) {
  chip_t *c = (chip_t *)user_data;
  if (c->resp_pos < c->resp_len) return c->resp[c->resp_pos++];
  return 0xFF;
}

static void on_disconnect(void *user_data) { (void)user_data; }

void chip_init(void) {
  chip_t *chip = (chip_t *)calloc(1, sizeof(chip_t));
  const i2c_config_t cfg = {
      .user_data = chip,
      .address = 0x59,
      .scl = pin_init("SCL", INPUT),
      .sda = pin_init("SDA", INPUT),
      .connect = on_connect,
      .read = on_read,
      .write = on_write,
      .disconnect = on_disconnect,
  };
  i2c_init(&cfg);
}
