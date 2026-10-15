#pragma once
#include "sensor_driver.h"

#include <SensirionI2CSen5x.h>

// SEN50 particulate matter, fan duty-cycled.
// The SEN50 hangs off the boosted 5 V rail (TPS61023, EN = pins::kBoostEn);
// the scheduler switches that rail. This driver handles the fan:
//   duty (interval >= 60 s): startMeasurement in onCycleStart, 30 s fan
//   warm-up (datasheet: stable PM after fan start), read, stopMeasurement.
//   continuous / short interval: fan stays on, read each cycle.
class Sen50Driver : public ISensorDriver {
 public:
  const char* name() const override { return "SEN50"; }
  bool begin() override;
  void configure(bool continuous, uint32_t intervalMs) override;
  void onCycleStart() override;
  uint32_t warmupMs() const override;
  bool read(Readings& out) override;
  void onCycleEnd() override;

 private:
  SensirionI2CSen5x sensor_;
  bool dutyCycled_ = false;  // true when the fan stops between cycles
  bool measuring_ = false;
  bool present_ = false;
};
