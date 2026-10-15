#include "scd41_driver.h"

#include <Arduino.h>
#include <Wire.h>

bool Scd41Driver::begin() {
  sensor_.begin(Wire);
  // A fresh boot may land mid-periodic-measurement (warm reset); stop first so
  // the serial-number probe works (it is refused while measuring). The library
  // already blocks the datasheet's 500 ms stop time internally.
  sensor_.stopPeriodicMeasurement();
  uint16_t s0, s1, s2;
  present_ = (sensor_.getSerialNumber(s0, s1, s2) == 0);
  return present_;
}

void Scd41Driver::configure(bool continuous, uint32_t /*intervalMs*/) {
  if (!present_) return;
  if (continuous && !continuous_) {
    sensor_.startPeriodicMeasurement();
  } else if (!continuous && continuous_) {
    sensor_.stopPeriodicMeasurement();  // blocks 500 ms inside the library
  }
  continuous_ = continuous;
}

void Scd41Driver::onCycleStart() {
  if (!present_ || continuous_) return;
  // The library blocks ~5 s inside measureSingleShot (no non-blocking variant
  // in 0.4.0). The scheduler stages this start so the block lands inside the
  // cycle's warm-up window; serial input is unserviced during those 5 s.
  sensor_.measureSingleShot();
}

uint32_t Scd41Driver::warmupMs() const {
  // Continuous: periodic data is already flowing — no extra wait.
  return continuous_ ? 0 : 5100;
}

bool Scd41Driver::read(Readings& out) {
  if (!present_) return false;
  bool ready = false;
  if (sensor_.getDataReadyFlag(ready) != 0 || !ready) return false;
  uint16_t co2 = 0;
  float temp = NAN, rh = NAN;
  if (sensor_.readMeasurement(co2, temp, rh) != 0 || co2 == 0) return false;
  out.co2Ppm = static_cast<float>(co2);
  out.tempC = temp;
  out.rhPct = rh;
  out.scd41Valid = true;
  lastRh_ = rh;
  lastT_ = temp;
  return true;
}

void Scd41Driver::onCycleEnd() {
  // Single-shot mode powers down on its own; nothing to stop.
}

bool Scd41Driver::lastRhT(float& rhPct, float& tempC) const {
  if (isnan(lastRh_) || isnan(lastT_)) return false;
  rhPct = lastRh_;
  tempC = lastT_;
  return true;
}
