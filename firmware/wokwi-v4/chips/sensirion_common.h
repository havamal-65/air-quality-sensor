// Shared helpers for the Sensirion-protocol Wokwi chip models (SCD41, SGP41,
// SEN50): CRC-8 (poly 0x31, init 0xFF, per 2-byte word) and the word+CRC
// response encoder. Each chip compiles standalone to its own wasm; this
// header keeps the implementations from diverging.
#pragma once
#include <stdint.h>

static uint8_t sensirion_crc(uint8_t b0, uint8_t b1) {
  uint8_t crc = 0xFF;
  uint8_t data[2] = {b0, b1};
  for (int i = 0; i < 2; ++i) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; ++bit) {
      crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x31) : (uint8_t)(crc << 1);
    }
  }
  return crc;
}

// Appends word MSB, LSB, CRC to resp[], advancing *resp_len.
static void sensirion_put_word(uint8_t *resp, uint32_t *resp_len, uint16_t w) {
  resp[(*resp_len)++] = (uint8_t)(w >> 8);
  resp[(*resp_len)++] = (uint8_t)(w & 0xFF);
  resp[(*resp_len)++] = sensirion_crc((uint8_t)(w >> 8), (uint8_t)(w & 0xFF));
}
