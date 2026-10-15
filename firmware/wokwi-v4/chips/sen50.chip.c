// Wokwi custom chip: Sensirion SEN50 particulate sensor at I2C 0x69.
// The EN pin models the boosted 5 V rail (TPS61023 EN, GPIO3): when EN is
// low the chip is unpowered and NAKs every transaction.
// Commands: 0xD304 reset, 0x0021 start measurement, 0x0104 stop,
//   0x0202 data ready, 0x03C4 read measured values (8 words).
// Scripted PM (scaled x10): pm1.0 ~3.2, pm2.5 ~5.4, pm4 ~6.1, pm10 ~6.8 ug/m3
// + per-read drift. RH/T/VOC/NOx words = 0x7FFF (not available on SEN50).
#include "wokwi-api.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  pin_t en;
  uint8_t rx[16];
  uint32_t rx_len;
  uint16_t cmd;
  uint8_t resp[32];
  uint32_t resp_len;
  uint32_t resp_pos;
  int measuring;
  uint64_t start_at;  // sim nanos when measurement started
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
    case 0x0202: {  // data ready: new data ~1 s after start
      int ready = c->measuring && (get_sim_nanos() - c->start_at) > 1000000000ULL;
      put_word(c, ready ? 0x0001 : 0x0000);
      break;
    }
    case 0x03C4: {  // measured values: 4x PM uint16 x10, then 4x 0x7FFF
      uint16_t drift = (uint16_t)(c->n % 7);
      c->n++;
      put_word(c, (uint16_t)(32 + drift));  // PM1.0  -> 3.2+ ug/m3
      put_word(c, (uint16_t)(54 + drift));  // PM2.5  -> 5.4+
      put_word(c, (uint16_t)(61 + drift));  // PM4.0  -> 6.1+
      put_word(c, (uint16_t)(68 + drift));  // PM10   -> 6.8+
      put_word(c, 0x7FFF);                  // RH  (n/a on SEN50)
      put_word(c, 0x7FFF);                  // T   (n/a)
      put_word(c, 0x7FFF);                  // VOC (n/a)
      put_word(c, 0x7FFF);                  // NOx (n/a)
      break;
    }
    default:
      put_word(c, 0x0000);
      break;
  }
}

static bool on_connect(void *user_data, uint32_t address, bool read) {
  chip_t *c = (chip_t *)user_data;
  if (pin_read(c->en) == LOW) return false;  // unpowered: NAK
  if (read) {
    prepare_response(c);
  } else {
    c->rx_len = 0;
  }
  return true;
}

static bool on_write(void *user_data, uint8_t data) {
  chip_t *c = (chip_t *)user_data;
  if (pin_read(c->en) == LOW) return false;
  if (c->rx_len < sizeof(c->rx)) c->rx[c->rx_len++] = data;
  if (c->rx_len == 2) {
    c->cmd = (uint16_t)((c->rx[0] << 8) | c->rx[1]);
    switch (c->cmd) {
      case 0x0021:  // start measurement (fan on)
        c->measuring = 1;
        c->start_at = get_sim_nanos();
        break;
      case 0x0104:  // stop measurement
      case 0xD304:  // device reset
        c->measuring = 0;
        break;
      default:
        break;
    }
  }
  return true;
}

static uint8_t on_read(void *user_data) {
  chip_t *c = (chip_t *)user_data;
  if (c->resp_pos < c->resp_len) return c->resp[c->resp_pos++];
  return 0xFF;
}

static void on_disconnect(void *user_data) { (void)user_data; }

void chip_init(void) {
  chip_t *chip = (chip_t *)calloc(1, sizeof(chip_t));
  chip->en = pin_init("EN", INPUT);
  const i2c_config_t cfg = {
      .user_data = chip,
      .address = 0x69,
      .scl = pin_init("SCL", INPUT),
      .sda = pin_init("SDA", INPUT),
      .connect = on_connect,
      .read = on_read,
      .write = on_write,
      .disconnect = on_disconnect,
  };
  i2c_init(&cfg);
}
