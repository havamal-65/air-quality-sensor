#include "scheduler.h"

#include <Arduino.h>

namespace {
constexpr uint32_t kPowerSettleMs = 1000;  // boost rail + SEN50 boot settle

void printFloat(const char* key, float v, uint8_t decimals) {
  Serial.print(key);
  Serial.print('=');
  if (isnan(v)) {
    Serial.print("nan");
  } else {
    Serial.print(v, decimals);
  }
}
}  // namespace

void CadenceScheduler::applyCadence(const cadence::Cadence& c,
                                    uint32_t firstDelayMs) {
  cadence_ = c;
  bool continuous = c.isContinuous();
  // Boost rail: continuous and short-interval modes keep the SEN50 powered;
  // duty mode powers it per cycle. Power must be UP before configure() so the
  // SEN50 start/stop commands reach a powered chip; drop the rail only after.
  bool keepPowered = !c.dutyCycled();
  if (keepPowered) {
    setBoost(true);
    delay(kPowerSettleMs);  // same rail + SEN50 boot settle the cycle path uses
  }
  for (size_t i = 0; i < count_; ++i) {
    drivers_[i]->configure(continuous, c.intervalMs());
  }
  if (!keepPowered) setBoost(false);
  state_ = State::Idle;
  // Continuous: delay the first read one interval so the SCD41's first
  // periodic sample (5 s) exists before we read; duty modes start at once.
  nextCycleAt_ = millis() + (continuous ? c.intervalMs() : firstDelayMs);
  Serial.print("CADENCE mode=");
  Serial.print(c.modeName());
  if (c.mode == cadence::Mode::Custom) {
    Serial.print(' ');
    Serial.print(c.customSeconds);
    Serial.print('s');
  }
  Serial.print(" interval=");
  Serial.print(c.intervalMs());
  Serial.println("ms");
}

void CadenceScheduler::triggerNow() { triggerPending_ = true; }

void CadenceScheduler::tick() {
  uint32_t now = millis();
  switch (state_) {
    case State::Idle:
      if (triggerPending_ || static_cast<int32_t>(now - nextCycleAt_) >= 0) {
        triggerPending_ = false;
        cycleStart_ = now;
        startedMask_ = 0;
        if (cadence_.dutyCycled()) {
          setBoost(true);
          state_ = State::PowerSettle;
        } else {
          state_ = State::Warmup;
        }
        stateSince_ = now;
      }
      break;

    case State::PowerSettle:
      if (now - stateSince_ >= kPowerSettleMs) {
        state_ = State::Warmup;
        stateSince_ = now;
      }
      break;

    case State::Warmup: {
      // Staged starts: fire each driver's onCycleStart so its own warm-up
      // completes exactly at the shared read instant (see scheduler.h). A
      // driver must never sit in its startup state longer than its warmupMs
      // (SGP41 conditioning has a 10 s datasheet maximum).
      uint32_t maxW = maxWarmupMs();
      uint32_t elapsed = now - stateSince_;
      for (size_t i = 0; i < count_; ++i) {
        uint32_t bit = 1UL << i;
        if ((startedMask_ & bit) == 0 &&
            elapsed + drivers_[i]->warmupMs() >= maxW) {
          drivers_[i]->onCycleStart();
          startedMask_ |= bit;
        }
      }
      if (elapsed >= maxW) {
        state_ = State::ReadReport;
        stateSince_ = now;
      }
      break;
    }

    case State::ReadReport: {
      Readings r;
      for (size_t i = 0; i < count_; ++i) drivers_[i]->read(r);
      uint32_t tMs = millis();
      report(r, tMs);
      if (hook_ != nullptr) hook_(r, tMs);
      for (size_t i = 0; i < count_; ++i) drivers_[i]->onCycleEnd();
      if (cadence_.dutyCycled()) setBoost(false);
      ++cycleCount_;
      nextCycleAt_ = cycleStart_ + cadence_.intervalMs();
      // If the cycle overran the interval, start the next one immediately.
      if (static_cast<int32_t>(nextCycleAt_ - millis()) < 0) {
        nextCycleAt_ = millis();
      }
      Serial.print("CYCLE n=");
      Serial.print(cycleCount_);
      Serial.print(" next_in=");
      Serial.print(static_cast<int32_t>(nextCycleAt_ - millis()) / 1000);
      Serial.print("s mode=");
      Serial.println(cadence_.modeName());
      state_ = State::Idle;
      break;
    }
  }
}

void CadenceScheduler::setBoost(bool on) {
  digitalWrite(boostEnPin_, on ? HIGH : LOW);
}

uint32_t CadenceScheduler::maxWarmupMs() const {
  uint32_t m = 0;
  for (size_t i = 0; i < count_; ++i) {
    uint32_t w = drivers_[i]->warmupMs();
    if (w > m) m = w;
  }
  return m;
}

void CadenceScheduler::report(const Readings& r, uint32_t tMs) {
  // Reference serial format for ticket 09 (spec AC-14: every quantity).
  Serial.print("DATA t=");
  Serial.print(tMs);
  Serial.print(' ');
  printFloat("co2", r.co2Ppm, 0);
  Serial.print(' ');
  printFloat("temp", r.tempC, 2);
  Serial.print(' ');
  printFloat("rh", r.rhPct, 1);
  Serial.print(" voc=");
  if (r.sgp41Valid) Serial.print(r.vocIndex); else Serial.print("nan");
  Serial.print(" nox=");
  if (r.sgp41Valid) Serial.print(r.noxIndex); else Serial.print("nan");
  Serial.print(' ');
  printFloat("pm1_0", r.pm1_0, 1);
  Serial.print(' ');
  printFloat("pm2_5", r.pm2_5, 1);
  Serial.print(' ');
  printFloat("pm10", r.pm10, 1);
  Serial.print(' ');
  printFloat("press", r.pressurePa, 0);
  Serial.println();
}
