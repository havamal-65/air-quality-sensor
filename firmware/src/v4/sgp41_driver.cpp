#include "sgp41_driver.h"

#include <Arduino.h>
#include <Wire.h>

#include "scd41_driver.h"

namespace {
// Datasheet default compensation when no RH/T is available.
constexpr uint16_t kDefaultRhTicks = 0x8000;  // 50 %RH
constexpr uint16_t kDefaultTTicks = 0x6666;   // 25 C
constexpr float kMaxAlgoIntervalS = 10.0f;    // gas index algorithm ceiling
}  // namespace

bool Sgp41Driver::begin() {
  sensor_.begin(Wire);
  uint16_t testResult = 0;
  if (sensor_.executeSelfTest(testResult) != 0) {
    present_ = false;
    return false;
  }
  present_ = true;
  // Datasheet: 10 s of conditioning before the first NOx measurement.
  Serial.println("INIT SGP41 conditioning 10s...");
  uint16_t rhTicks, tTicks, srawVoc = 0;
  compensationTicks(rhTicks, tTicks);
  for (int i = 0; i < 10; ++i) {
    sensor_.executeConditioning(rhTicks, tTicks, srawVoc);
    delay(1000);
  }
  sensor_.turnHeaterOff();
  return true;
}

void Sgp41Driver::configure(bool continuous, uint32_t intervalMs) {
  if (!present_) return;
  continuous_ = continuous;
  // Recreate the index algorithms at the new sampling interval (capped at the
  // algorithm's 10 s ceiling — documented limitation; index restarts here).
  // The actual sample pace in every mode is intervalMs (continuous is ~5.5 s,
  // set by cadence::Cadence::intervalMs), capped at the algorithm's ceiling.
  float intervalS = intervalMs / 1000.0f;
  if (intervalS > kMaxAlgoIntervalS) intervalS = kMaxAlgoIntervalS;
  delete voc_;
  delete nox_;
  voc_ = new SensirionGasIndexAlgorithm(
      SensirionGasIndexAlgorithm::ALGORITHM_TYPE_VOC, intervalS);
  nox_ = new SensirionGasIndexAlgorithm(
      SensirionGasIndexAlgorithm::ALGORITHM_TYPE_NOX, intervalS);
  if (!continuous) sensor_.turnHeaterOff();
}

void Sgp41Driver::onCycleStart() {
  if (!present_ || continuous_) return;
  // Low-power mode: one conditioning pulse re-heats the hotplate.
  uint16_t rhTicks, tTicks, srawVoc = 0;
  compensationTicks(rhTicks, tTicks);
  sensor_.executeConditioning(rhTicks, tTicks, srawVoc);
}

uint32_t Sgp41Driver::warmupMs() const { return continuous_ ? 0 : 10000; }

bool Sgp41Driver::read(Readings& out) {
  if (!present_) return false;
  uint16_t rhTicks, tTicks;
  compensationTicks(rhTicks, tTicks);
  uint16_t srawVoc = 0, srawNox = 0;
  if (sensor_.measureRawSignals(rhTicks, tTicks, srawVoc, srawNox) != 0) {
    return false;
  }
  if (voc_ == nullptr || nox_ == nullptr) return false;
  out.vocIndex = voc_->process(static_cast<int32_t>(srawVoc));
  out.noxIndex = nox_->process(static_cast<int32_t>(srawNox));
  out.sgp41Valid = true;
  return true;
}

void Sgp41Driver::onCycleEnd() {
  if (!present_ || continuous_) return;
  sensor_.turnHeaterOff();
}

void Sgp41Driver::compensationTicks(uint16_t& rhTicks, uint16_t& tTicks) const {
  float rh, t;
  if (rhtSource_.lastRhT(rh, t)) {
    rhTicks = static_cast<uint16_t>(rh * 65535.0f / 100.0f);
    tTicks = static_cast<uint16_t>((t + 45.0f) * 65535.0f / 175.0f);
  } else {
    rhTicks = kDefaultRhTicks;
    tTicks = kDefaultTTicks;
  }
}
