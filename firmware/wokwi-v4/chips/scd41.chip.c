// Wokwi custom chip: Sensirion SCD41 CO2 sensor at I2C 0x62.
// Supports the commands the AirSense v4 firmware uses:
//   0x3F86 stop periodic, 0x3682 get serial, 0x21B1 start periodic,
//   0xE4B8 data ready, 0xEC05 read measurement, 0x219D measure single shot.
// Scripted values: co2 = 650 + 25*(n%8), t = 24.0 + 0.1*(n%5), rh = 40 + n%10.
#include "wokwi-api.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint8_t rx[16];
  uint32_t rx_len;
  uint16_t cmd;
  uint8_t resp[16];
  uint32_t resp_len;
  uint32_t resp_pos;
  int periodic;
  int single_pending;
  uint64_t next_ready_at;  // sim nanos when data becomes ready
  uint32_t n;              // script step
} chip_t;

#include "sensirion_common.h"

static void put_word(chip_t *c, uint16_t w) {
  sensirion_put_word(c->resp, &c->resp_len, w);
}

static int data_ready(chip_t *c) {
  if (c->periodic) return get_sim_nanos() >= c->next_ready_at;
  if (c->single_pending) return get_sim_nanos() >= c->next_ready_at;
  return 0;
}

static void prepare_response(chip_t *c) {
  c->resp_len = 0;
  c->resp_pos = 0;
  switch (c->cmd) {
    case 0x3682:  // get serial number
      put_word(c, 0x1111);
      put_word(c, 0x2222);
      put_word(c, 0x3333);
      break;
    case 0xE4B8:  // data ready flag: lower 11 bits nonzero = ready
      put_word(c, data_ready(c) ? 0x07FF : 0x0000);
      break;
    case 0xEC05: {  // read measurement
      uint16_t co2 = (uint16_t)(650 + 25 * (c->n % 8));
      float t = 24.0f + 0.1f * (float)(c->n % 5);
      float rh = 40.0f + (float)(c->n % 10);
      uint16_t traw = (uint16_t)((t + 45.0f) * 65535.0f / 175.0f);
      uint16_t rhraw = (uint16_t)(rh * 65535.0f / 100.0f);
      c->n++;
      put_word(c, co2);
      put_word(c, traw);
      put_word(c, rhraw);
      // Consume: single shot done; periodic re-arms 5 s out.
      c->single_pending = 0;
      if (c->periodic) c->next_ready_at = get_sim_nanos() + 5000000000ULL;
      break;
    }
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
  if (c->rx_len == 2) {
    c->cmd = (uint16_t)((c->rx[0] << 8) | c->rx[1]);
    switch (c->cmd) {
      case 0x21B1:  // start periodic measurement
        c->periodic = 1;
        c->next_ready_at = get_sim_nanos() + 5000000000ULL;
        break;
      case 0x3F86:  // stop periodic measurement
        c->periodic = 0;
        break;
      case 0x219D:  // measure single shot: ready ~4.5 s later
        c->single_pending = 1;
        c->next_ready_at = get_sim_nanos() + 4500000000ULL;
        break;
      default:
        break;
    }
  }
  return true;  // ACK
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
      .address = 0x62,
      .scl = pin_init("SCL", INPUT),
      .sda = pin_init("SDA", INPUT),
      .connect = on_connect,
      .read = on_read,
      .write = on_write,
      .disconnect = on_disconnect,
  };
  i2c_init(&cfg);
}
