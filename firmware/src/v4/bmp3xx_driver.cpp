#include "bmp3xx_driver.h"

#include <Wire.h>

#include "pins.h"

bool Bmp3xxDriver::begin() {
  present_ = sensor_.begin_I2C(kAddrBmp3xx, &Wire);
  if (present_) {
    sensor_.setTemperatureOversampling(BMP3_OVERSAMPLING_2X);
    sensor_.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    sensor_.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  }
  return present_;
}

void Bmp3xxDriver::configure(bool /*continuous*/, uint32_t /*intervalMs*/) {
  // Forced mode regardless of cadence; performReading() triggers a conversion.
}

bool Bmp3xxDriver::read(Readings& out) {
  if (!present_ || !sensor_.performReading()) return false;
  out.pressurePa = sensor_.pressure;  // Pa
  out.bmp3xxValid = true;
  return true;
}
