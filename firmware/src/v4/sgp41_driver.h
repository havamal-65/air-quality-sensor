#pragma once
#include "sensor_driver.h"

#include <SensirionI2CSgp41.h>
#include <SensirionGasIndexAlgorithm.h>

class Scd41Driver;  // RH/T compensation source

// SGP41 VOC/NOx raw signals + Sensirion gas index algorithms.
// Boot: 10 s conditioning (datasheet requirement before first measure).
// Duty cycle follows Sensirion's documented SGP41 low-power mode:
//   onCycleStart -> one executeConditioning, 10 s warm-up,
//   read -> measureRawSignals, onCycleEnd -> turnHeaterOff.
// The scheduler stages onCycleStart so conditioning runs exactly warmupMs()
// (10 s, the datasheet maximum) before the read, even when another driver's
// longer warm-up (SEN50: 30 s) sets the cycle's total warm-up.
// Continuous: heater stays on, measure every cycle.
//
// Index-algorithm limitation (documented): the gas index algorithm supports
// sampling intervals up to 10 s. For cadences above 10 s we construct it with
// a 10 s interval anyway; the index is indicative, not reference-grade, at
// long cadences. Changing cadence recreates the algorithm (index restarts).
class Sgp41Driver : public ISensorDriver {
 public:
  explicit Sgp41Driver(const Scd41Driver& rhtSource) : rhtSource_(rhtSource) {}
  const char* name() const override { return "SGP41"; }
  bool begin() override;
  void configure(bool continuous, uint32_t intervalMs) override;
  void onCycleStart() override;
  uint32_t warmupMs() const override;
  bool read(Readings& out) override;
  void onCycleEnd() override;

 private:
  void compensationTicks(uint16_t& rhTicks, uint16_t& tTicks) const;

  const Scd41Driver& rhtSource_;
  SensirionI2CSgp41 sensor_;
  // Held by pointer: the algorithm class is non-copyable and the NOx subclass
  // has no interval constructor, so we construct the base class directly and
  // recreate on cadence change.
  SensirionGasIndexAlgorithm* voc_ = nullptr;
  SensirionGasIndexAlgorithm* nox_ = nullptr;
  bool continuous_ = false;
  bool present_ = false;
};
