#pragma once
#include <stdint.h>

// Sampling cadence (spec AC-10): continuous, 60 s (default), 300 s, custom.
// Persisted in NVS so the setting survives reboots.
namespace cadence {

enum class Mode : uint8_t { Continuous = 0, Sec60 = 1, Sec300 = 2, Custom = 3 };

// Duty-cycling (SEN50 fan + 5 V boost rail off between cycles) only pays off
// at intervals of at least this; below it the fan never finishes warming up.
constexpr uint32_t kDutyThresholdMs = 60000;
constexpr uint32_t kMinCustomSeconds = 10;
// Upper clamp: keeps customSeconds * 1000 far from uint32 overflow and bounds
// the interval to one day.
constexpr uint32_t kMaxCustomSeconds = 86400;

struct Cadence {
  Mode mode = Mode::Sec60;
  uint32_t customSeconds = 60;

  bool isContinuous() const { return mode == Mode::Continuous; }
  uint32_t intervalMs() const;
  // Duty-cycle (power the 5 V rail and fan down between cycles)?
  bool dutyCycled() const {
    return !isContinuous() && intervalMs() >= kDutyThresholdMs;
  }
  const char* modeName() const;
};

// NVS persistence (namespace "airsense").
Cadence load();
void save(const Cadence& c);

}  // namespace cadence
