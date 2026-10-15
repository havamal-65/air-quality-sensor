#pragma once
#include <stdint.h>
#include "readings.h"

// Sensor abstraction seam (ticket 05): the scheduler talks only to this
// interface, so a fake driver can stand in for a real one.
//
// Cycle contract, driven by CadenceScheduler:
//   configure(continuous, intervalMs)  — on boot and on every cadence change
//   onCycleStart()                     — power is up; start/trigger measurement
//   warmupMs()                         — how long after onCycleStart() before
//                                        read() may be called (max over all
//                                        drivers is the cycle warm-up)
//   read(out)                          — fill own fields + valid flag
//   onCycleEnd()                       — duty-cycle shutdown (fan/heater off)
class ISensorDriver {
 public:
  virtual ~ISensorDriver() = default;
  virtual const char* name() const = 0;
  virtual bool begin() = 0;  // probe + first init; false = absent/failed
  virtual void configure(bool continuous, uint32_t intervalMs) = 0;
  virtual void onCycleStart() = 0;
  virtual uint32_t warmupMs() const = 0;
  virtual bool read(Readings& out) = 0;
  virtual void onCycleEnd() = 0;
};
