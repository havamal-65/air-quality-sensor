#include "sen50_driver.h"

#include <Arduino.h>
#include <Wire.h>

#include "cadence.h"

bool Sen50Driver::begin() {
  sensor_.begin(Wire);
  if (sensor_.deviceReset() != 0) {
    present_ = false;
    return false;
  }
  delay(100);  // datasheet: reset settle
  present_ = true;
  measuring_ = false;
  return true;
}

void Sen50Driver::configure(bool continuous, uint32_t intervalMs) {
  if (!present_) return;
  dutyCycled_ = !continuous && intervalMs >= cadence::kDutyThresholdMs;
  if (!dutyCycled_ && !measuring_) {
    // measuring_ only on success: a failed start (chip still booting, rail
    // glitch) must leave read() reporting nan, not silently claim a fan.
    measuring_ = (sensor_.startMeasurement() == 0);
  } else if (dutyCycled_ && measuring_) {
    sensor_.stopMeasurement();
    measuring_ = false;
  }
}

void Sen50Driver::onCycleStart() {
  if (!present_ || !dutyCycled_) return;
  measuring_ = (sensor_.startMeasurement() == 0);
}

uint32_t Sen50Driver::warmupMs() const {
  // Fan spin-up + flow stabilization before PM numbers are trustworthy.
  return dutyCycled_ ? 30000 : 0;
}

bool Sen50Driver::read(Readings& out) {
  if (!present_ || !measuring_) return false;
  bool ready = false;
  if (sensor_.readDataReady(ready) != 0 || !ready) return false;
  float pm1 = NAN, pm2_5 = NAN, pm4 = NAN, pm10 = NAN;
  float rh = NAN, temp = NAN, voc = NAN, nox = NAN;  // all NaN on SEN50
  if (sensor_.readMeasuredValues(pm1, pm2_5, pm4, pm10, rh, temp, voc, nox) !=
      0) {
    return false;
  }
  out.pm1_0 = pm1;
  out.pm2_5 = pm2_5;
  out.pm10 = pm10;
  out.sen50Valid = true;
  return true;
}

void Sen50Driver::onCycleEnd() {
  if (!present_ || !dutyCycled_) return;
  sensor_.stopMeasurement();
  measuring_ = false;
}
