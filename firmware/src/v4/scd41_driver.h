#pragma once
#include "sensor_driver.h"

#include <SensirionI2CScd4x.h>

// SCD41 CO2/temp/RH.
// Continuous: periodic measurement (new data every 5 s), read latest each cycle.
// Duty: single-shot per cycle (measureSingleShot in onCycleStart, data ~5 s later).
class Scd41Driver : public ISensorDriver {
 public:
  const char* name() const override { return "SCD41"; }
  bool begin() override;
  void configure(bool continuous, uint32_t intervalMs) override;
  void onCycleStart() override;
  uint32_t warmupMs() const override;
  bool read(Readings& out) override;
  void onCycleEnd() override;

  // Last temp/RH, used by SGP41 compensation (ticks). Valid after a good read.
  bool lastRhT(float& rhPct, float& tempC) const;

 private:
  SensirionI2CScd4x sensor_;
  bool continuous_ = false;
  bool present_ = false;
  float lastRh_ = NAN;
  float lastT_ = NAN;
};
