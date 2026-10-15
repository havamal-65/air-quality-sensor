#pragma once
#include "sensor_driver.h"

#include <Adafruit_BMP3XX.h>

// BMP384 barometric pressure (Adafruit BMP3XX library covers 384/388/390).
// Forced mode: one conversion per read, sleeps between — no duty handling.
class Bmp3xxDriver : public ISensorDriver {
 public:
  const char* name() const override { return "BMP3xx"; }
  bool begin() override;
  void configure(bool continuous, uint32_t intervalMs) override;
  void onCycleStart() override {}
  uint32_t warmupMs() const override { return 0; }
  bool read(Readings& out) override;
  void onCycleEnd() override {}

 private:
  Adafruit_BMP3XX sensor_;
  bool present_ = false;
};
