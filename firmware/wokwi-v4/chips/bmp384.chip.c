// Wokwi custom chip: Bosch BMP384 barometer at I2C 0x76 (register model).
// First written byte sets the register pointer; reads auto-increment.
// CHIP_ID 0x50 (accepted by the bmp3 driver), STATUS 0x70 (cmd_rdy + both
// data-ready bits), raw data registers preloaded for 101325 Pa / 25 C using
// a trivial calibration:
//   T1=30000, T2=27000, T3=0  -> t_lin = (raw_t - 7680000) * (27000 / 2^30)
//   P1=32767, P5=5000, rest 0 -> comp_p = 40000 + raw_p * (16383 / 2^20)
#include "wokwi-api.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint8_t regs[256];
  uint32_t ptr;
  int have_ptr;  // first write byte of a transaction = register pointer
} chip_t;

// Trim CRC the Adafruit wrapper validates at init: CRC-8 poly 0x1D, seed
// 0xFF, over the 21 calibration bytes (0x31..0x45), final XOR 0xFF, stored
// at register 0x30. Mirrors cal_crc/validate_trimming_param in the library.
static uint8_t cal_crc(uint8_t seed, uint8_t data) {
  for (int i = 0; i < 8; i++) {
    uint8_t var2 = ((seed & 0x80) ^ (data & 0x80)) ? 1 : 0;
    seed = (uint8_t)((seed & 0x7F) << 1);
    data = (uint8_t)((data & 0x7F) << 1);
    seed = (uint8_t)(seed ^ (uint8_t)(0x1D * var2));
  }
  return seed;
}

static void store_trim_crc(chip_t *c) {
  uint8_t crc = 0xFF;
  for (int i = 0; i < 21; i++) {
    crc = cal_crc(crc, c->regs[0x31 + i]);
  }
  c->regs[0x30] = (uint8_t)(crc ^ 0xFF);
}

static void load_raw(chip_t *c, float temp_c, float press_pa) {
  uint32_t raw_t = (uint32_t)(7680000.0 + temp_c * 1073741824.0 / 27000.0);
  uint32_t raw_p = (uint32_t)((press_pa - 40000.0) * 1048576.0 / 16383.0);
  c->regs[0x04] = (uint8_t)(raw_p & 0xFF);         // press xlsb
  c->regs[0x05] = (uint8_t)((raw_p >> 8) & 0xFF);  // press lsb
  c->regs[0x06] = (uint8_t)((raw_p >> 16) & 0xFF); // press msb
  c->regs[0x07] = (uint8_t)(raw_t & 0xFF);         // temp xlsb
  c->regs[0x08] = (uint8_t)((raw_t >> 8) & 0xFF);  // temp lsb
  c->regs[0x09] = (uint8_t)((raw_t >> 16) & 0xFF); // temp msb
}

static bool on_connect(void *user_data, uint32_t address, bool read) {
  chip_t *c = (chip_t *)user_data;
  if (!read) c->have_ptr = 0;
  return true;
}

static bool on_write(void *user_data, uint8_t data) {
  chip_t *c = (chip_t *)user_data;
  if (!c->have_ptr) {
    c->ptr = data;
    c->have_ptr = 1;
  } else {
    // Keep ID/status/data/cal regs authoritative; store control regs as-is.
    if (c->ptr >= 0x1B && c->ptr <= 0x7E) c->regs[c->ptr] = data;
    c->ptr = (c->ptr + 1) & 0xFF;
  }
  return true;
}

static uint8_t on_read(void *user_data) {
  chip_t *c = (chip_t *)user_data;
  uint8_t v = c->regs[c->ptr];
  c->ptr = (c->ptr + 1) & 0xFF;
  return v;
}

static void on_disconnect(void *user_data) { (void)user_data; }

void chip_init(void) {
  chip_t *chip = (chip_t *)calloc(1, sizeof(chip_t));
  chip->regs[0x00] = 0x50;  // CHIP_ID: BMP384
  chip->regs[0x01] = 0x01;  // REV_ID
  chip->regs[0x02] = 0x00;  // ERR_REG
  chip->regs[0x03] = 0x70;  // STATUS: cmd_rdy | drdy_press | drdy_temp
  // Trivial calibration (NVM 0x31..0x45, little-endian words):
  chip->regs[0x31] = 0x30;  // T1 = 30000 (0x7530)
  chip->regs[0x32] = 0x75;
  chip->regs[0x33] = 0x78;  // T2 = 27000 (0x6978)
  chip->regs[0x34] = 0x69;
  chip->regs[0x35] = 0x00;  // T3 = 0
  chip->regs[0x36] = 0xFF;  // P1 = 32767 (0x7FFF)
  chip->regs[0x37] = 0x7F;
  // P2 reg = 16384: the library computes par_p2 = (reg - 16384)/2^29, so a
  // zero register would yield a nonzero parameter (-16384/2^29) that skews
  // pressure by ~3 kPa at 25 C. 16384 makes par_p2 exactly 0.
  chip->regs[0x38] = 0x00;  // P2 = 16384 (0x4000)
  chip->regs[0x39] = 0x40;
  // P3..P4 = 0 (0x3A..0x3B already zero; no offset on these)
  chip->regs[0x3C] = 0x88;  // P5 = 5000 (0x1388)
  chip->regs[0x3D] = 0x13;
  // P6..P11 = 0 (0x3E..0x45 already zero)
  store_trim_crc(chip);
  load_raw(chip, 25.0f, 101325.0f);

  const i2c_config_t cfg = {
      .user_data = chip,
      .address = 0x76,
      .scl = pin_init("SCL", INPUT),
      .sda = pin_init("SDA", INPUT),
      .connect = on_connect,
      .read = on_read,
      .write = on_write,
      .disconnect = on_disconnect,
  };
  i2c_init(&cfg);
}
