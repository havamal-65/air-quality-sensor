#pragma once
#include <math.h>
#include <stdint.h>

#include "readings.h"

// AirSense BLE protocol constants + value packing (ticket 06).
// The authoritative protocol document lives beside the app constants:
//   app/constants/ble-protocol.md
// The Phase-2 block below is FROZEN (app/constants/ble.ts ships these UUIDs);
// v4 only adds characteristics, never changes existing ones.
namespace bleproto {

constexpr char kDeviceName[] = "AirSense";

// ---- Frozen Phase-2 contract -------------------------------------------
constexpr char kEnvServiceUuid[] = "0000181a-0000-1000-8000-00805f9b34fb";
constexpr char kCo2Uuid[] = "00002bd2-0000-1000-8000-00805f9b34fb";
constexpr char kTemperatureUuid[] = "00002a6e-0000-1000-8000-00805f9b34fb";
constexpr char kHumidityUuid[] = "00002a6f-0000-1000-8000-00805f9b34fb";

constexpr char kCustomServiceUuid[] = "12345678-1234-5678-1234-56789abcdef0";
constexpr char kVocUuid[] = "12345678-1234-5678-1234-56789abcdef1";
constexpr char kNoxUuid[] = "12345678-1234-5678-1234-56789abcdef2";

// ---- v4 additions (ticket 06) ------------------------------------------
// Standard Environmental Sensing characteristics. Pressure (0x2A6D) is
// uint32 LE in 0.1 Pa — full precision. The PM characteristics
// (0x2BD5/0x2BD6/0x2BD7) are medfloat16 in kg/m3 per the GATT Specification
// Supplement; medfloat16's exponent floor of 1e-8 kg/m3 makes their best
// resolution 10 ug/m3, so the app reads the custom PM characteristics below
// instead. The standard ones exist for generic ESS clients.
constexpr char kPressureUuid[] = "00002a6d-0000-1000-8000-00805f9b34fb";
constexpr char kPm1StdUuid[] = "00002bd5-0000-1000-8000-00805f9b34fb";
constexpr char kPm25StdUuid[] = "00002bd6-0000-1000-8000-00805f9b34fb";
constexpr char kPm10StdUuid[] = "00002bd7-0000-1000-8000-00805f9b34fb";

// Custom characteristics, continuing the shipped ...def block.
// PM: uint16 LE, ug/m3 x10 (0xFFFF = invalid).
constexpr char kPm1Uuid[] = "12345678-1234-5678-1234-56789abcdef3";
constexpr char kPm25Uuid[] = "12345678-1234-5678-1234-56789abcdef4";
constexpr char kPm10Uuid[] = "12345678-1234-5678-1234-56789abcdef5";
// Cadence: 5 bytes LE {uint8 mode (0=continuous,1=60s,2=300s,3=custom),
// uint32 custom_seconds}. READ returns the persisted setting; WRITE
// validates, persists, and applies once no app is connected (an active
// connection forces continuous sampling).
constexpr char kCadenceUuid[] = "12345678-1234-5678-1234-56789abcdef6";
// History sync: READ = 16-byte status, WRITE 0x01/0x00 = start/abort dump,
// NOTIFY = 26-byte record frames + 6-byte end frame (see ble-protocol.md).
constexpr char kHistoryUuid[] = "12345678-1234-5678-1234-56789abcdef7";

// Invalid-value sentinels for the v4 additions and history records. The
// frozen Phase-2 characteristics keep their original "always a number"
// semantics (0 before the first valid sample).
constexpr uint16_t kInvalidU16 = 0xFFFF;
constexpr int16_t kInvalidTemp = 0x7FFF;
constexpr uint32_t kInvalidU32 = 0xFFFFFFFF;

// ---- Packing (all multi-byte values little-endian on the wire) ----------

inline uint16_t clampU16(float v) {
  if (v < 0) return 0;
  if (v > 65534.0f) return 65534;
  return static_cast<uint16_t>(lroundf(v));
}

// Frozen characteristics: 0 when invalid (Phase-2 semantics).
inline uint16_t packCo2(const Readings& r) {
  return (r.scd41Valid && !isnan(r.co2Ppm)) ? clampU16(r.co2Ppm) : 0;
}
inline int16_t packTemp(const Readings& r) {
  if (!r.scd41Valid || isnan(r.tempC)) return 0;
  float v = r.tempC * 100.0f;
  if (v > 32766.0f) v = 32766.0f;
  if (v < -32768.0f) v = -32768.0f;
  return static_cast<int16_t>(lroundf(v));
}
inline uint16_t packHumidity(const Readings& r) {
  return (r.scd41Valid && !isnan(r.rhPct)) ? clampU16(r.rhPct * 100.0f) : 0;
}
inline uint16_t packVoc(const Readings& r) {
  return r.sgp41Valid ? clampU16(static_cast<float>(r.vocIndex)) : 0;
}
inline uint16_t packNox(const Readings& r) {
  return r.sgp41Valid ? clampU16(static_cast<float>(r.noxIndex)) : 0;
}

// v4 additions: sentinel when invalid.
inline uint16_t packPmX10(float ugm3, bool valid) {
  return (valid && !isnan(ugm3)) ? clampU16(ugm3 * 10.0f) : kInvalidU16;
}
inline uint32_t packPressurePax10(const Readings& r) {
  if (!r.bmp3xxValid || isnan(r.pressurePa)) return kInvalidU32;
  float v = r.pressurePa * 10.0f;
  if (v < 0) v = 0;
  return static_cast<uint32_t>(v + 0.5f);
}

// medfloat16 (IEEE 11073 SFLOAT): 4-bit base-10 exponent, 12-bit mantissa.
// PM in kg/m3 needs exponent -8 (the floor), mantissa = ug/m3 / 10.
// SEN50 tops out at 1000 ug/m3 (mantissa 100), far inside the 2045 limit.
inline uint16_t packPmSfloat(float ugm3, bool valid) {
  if (!valid || isnan(ugm3)) return 0x07FF;  // SFLOAT NaN
  long m = lroundf(ugm3 / 10.0f);
  if (m < 0) m = 0;
  if (m > 2045) m = 2045;
  return static_cast<uint16_t>((0x8u << 12) | (static_cast<uint16_t>(m) & 0x0FFF));
}

}  // namespace bleproto
