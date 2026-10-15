#include "cadence.h"

#include <Preferences.h>

namespace cadence {
namespace {
constexpr const char* kNvsNamespace = "airsense";
constexpr const char* kKeyMode = "cad_mode";
constexpr const char* kKeyCustom = "cad_custom";
}  // namespace

uint32_t Cadence::intervalMs() const {
  switch (mode) {
    case Mode::Continuous:
      // Slightly above the SCD41's 5 s periodic data rate so each read finds
      // fresh data instead of beating exactly against the sample boundary.
      return 5500;
    case Mode::Sec60:
      return 60000;
    case Mode::Sec300:
      return 300000;
    case Mode::Custom: {
      uint32_t s = customSeconds;
      if (s < kMinCustomSeconds) s = kMinCustomSeconds;
      if (s > kMaxCustomSeconds) s = kMaxCustomSeconds;
      return s * 1000;
    }
  }
  return 60000;
}

const char* Cadence::modeName() const {
  switch (mode) {
    case Mode::Continuous:
      return "continuous";
    case Mode::Sec60:
      return "60s";
    case Mode::Sec300:
      return "300s";
    case Mode::Custom:
      return "custom";
  }
  return "?";
}

Cadence load() {
  Cadence c;
  Preferences prefs;
  // Read-write open: creates the namespace on first boot (a read-only open of
  // a missing namespace logs an NVS error).
  if (prefs.begin(kNvsNamespace, /*readOnly=*/false)) {
    uint8_t m = prefs.getUChar(kKeyMode, static_cast<uint8_t>(Mode::Sec60));
    if (m <= static_cast<uint8_t>(Mode::Custom)) c.mode = static_cast<Mode>(m);
    c.customSeconds = prefs.getUInt(kKeyCustom, 60);
    prefs.end();
  }
  if (c.customSeconds < kMinCustomSeconds) c.customSeconds = kMinCustomSeconds;
  if (c.customSeconds > kMaxCustomSeconds) c.customSeconds = kMaxCustomSeconds;
  return c;
}

void save(const Cadence& c) {
  Preferences prefs;
  if (prefs.begin(kNvsNamespace, /*readOnly=*/false)) {
    prefs.putUChar(kKeyMode, static_cast<uint8_t>(c.mode));
    prefs.putUInt(kKeyCustom, c.customSeconds);
    prefs.end();
  }
}

}  // namespace cadence
