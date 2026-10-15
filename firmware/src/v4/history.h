#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdint.h>

#include "readings.h"

// Ring buffer of recent samples for BLE history sync (ticket 06).
// One record per completed measurement cycle; oldest records are overwritten
// when full. 720 records cover 12 h at the 60 s default cadence (~17 KB RAM).
namespace history {

// Wire format of one record: 24 bytes, little-endian (ESP32-C3 is
// little-endian, so the packed struct is sent as raw bytes). Documented in
// app/constants/ble-protocol.md — any change here must change that document.
struct __attribute__((packed)) Record {
  uint32_t tSeconds;    // uptime seconds at sample time (wraps at ~136 years)
  uint16_t co2Ppm;      // 0xFFFF = invalid
  int16_t temp100;      // degC x100, 0x7FFF = invalid
  uint16_t rh100;       // %RH x100, 0xFFFF = invalid
  uint16_t vocIndex;    // 0 = invalid/blackout (Sensirion semantics)
  uint16_t noxIndex;    // 0 = invalid/blackout
  uint16_t pm1x10;      // ug/m3 x10, 0xFFFF = invalid
  uint16_t pm25x10;     // ug/m3 x10, 0xFFFF = invalid
  uint16_t pm10x10;     // ug/m3 x10, 0xFFFF = invalid
  uint32_t pressPax10;  // Pa x10, 0xFFFFFFFF = invalid
};
static_assert(sizeof(Record) == 24, "history record wire format is 24 bytes");

constexpr uint16_t kCapacity = 720;

class Buffer {
 public:
  void push(const Readings& r, uint32_t tMs);
  uint16_t count() const { return count_; }
  uint16_t capacity() const { return kCapacity; }
  // idx 0 = oldest stored record. Precondition: idx < count(). Loop-task
  // only (same task as push); the BLE task must use snapshot() instead.
  const Record& at(uint16_t idx) const;
  // Consistent status snapshot, safe from the Bluedroid callback task while
  // the loop task may be mid-push.
  void snapshot(uint16_t& count, uint32_t& oldestT, uint32_t& newestT);

 private:
  Record records_[kCapacity];
  uint16_t head_ = 0;  // next write slot
  uint16_t count_ = 0;
  portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
};

}  // namespace history
