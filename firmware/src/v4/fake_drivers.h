#pragma once
#include <stdint.h>

#include "sensor_driver.h"

// Scripted stand-ins for every real driver (build with
// -DAIRSENSE_FAKE_SENSORS). They exercise the scheduler and serial protocol
// with no bus traffic at all — the abstraction-seam proof that a fake driver
// can replace a real one. Values ramp deterministically per cycle.
namespace fake {

class FakeScd41 : public ISensorDriver {
 public:
  const char* name() const override { return "SCD41(fake)"; }
  bool begin() override { return true; }
  void configure(bool, uint32_t) override {}
  void onCycleStart() override {}
  uint32_t warmupMs() const override { return 0; }
  bool read(Readings& out) override {
    out.co2Ppm = 650.0f + 25.0f * (n_ % 8);
    out.tempC = 24.0f + 0.1f * (n_ % 5);
    out.rhPct = 40.0f + (n_ % 10);
    out.scd41Valid = true;
    ++n_;
    return true;
  }
  void onCycleEnd() override {}

 private:
  uint32_t n_ = 0;
};

class FakeSgp41 : public ISensorDriver {
 public:
  const char* name() const override { return "SGP41(fake)"; }
  bool begin() override { return true; }
  void configure(bool, uint32_t) override {}
  void onCycleStart() override {}
  uint32_t warmupMs() const override { return 0; }
  bool read(Readings& out) override {
    out.vocIndex = 100 + (n_ % 20);
    out.noxIndex = 1 + (n_ % 3);
    out.sgp41Valid = true;
    ++n_;
    return true;
  }
  void onCycleEnd() override {}

 private:
  uint32_t n_ = 0;
};

class FakeSen50 : public ISensorDriver {
 public:
  const char* name() const override { return "SEN50(fake)"; }
  bool begin() override { return true; }
  void configure(bool, uint32_t) override {}
  void onCycleStart() override {}
  uint32_t warmupMs() const override { return 0; }
  bool read(Readings& out) override {
    out.pm1_0 = 3.2f + 0.1f * (n_ % 7);
    out.pm2_5 = 5.4f + 0.1f * (n_ % 7);
    out.pm10 = 6.8f + 0.1f * (n_ % 7);
    out.sen50Valid = true;
    ++n_;
    return true;
  }
  void onCycleEnd() override {}

 private:
  uint32_t n_ = 0;
};

class FakeBmp3xx : public ISensorDriver {
 public:
  const char* name() const override { return "BMP3xx(fake)"; }
  bool begin() override { return true; }
  void configure(bool, uint32_t) override {}
  void onCycleStart() override {}
  uint32_t warmupMs() const override { return 0; }
  bool read(Readings& out) override {
    out.pressurePa = 101325.0f + 10.0f * (n_ % 5);
    out.bmp3xxValid = true;
    ++n_;
    return true;
  }
  void onCycleEnd() override {}

 private:
  uint32_t n_ = 0;
};

}  // namespace fake
