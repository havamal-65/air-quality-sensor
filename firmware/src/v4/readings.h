#pragma once
#include <stdint.h>
#include <math.h>

// One sample of every quantity the device reports (spec AC-14 list).
// A driver fills only its own fields and sets the matching valid flag.
struct Readings {
  // SCD41
  float co2Ppm = NAN;
  float tempC = NAN;
  float rhPct = NAN;
  bool scd41Valid = false;

  // SGP41 (+ gas index algorithm)
  int32_t vocIndex = -1;
  int32_t noxIndex = -1;
  bool sgp41Valid = false;

  // SEN50
  float pm1_0 = NAN;
  float pm2_5 = NAN;
  float pm10 = NAN;
  bool sen50Valid = false;

  // BMP384/BMP390 (shared BMP3xx API)
  float pressurePa = NAN;
  bool bmp3xxValid = false;
};
