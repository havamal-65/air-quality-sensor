#pragma once
#include <stdint.h>
#include <stddef.h>

#include "cadence.h"
#include "sensor_driver.h"

// CadenceScheduler: runs one measurement cycle per cadence interval.
//
// Duty cycle (interval >= 60 s):
//   IDLE --(nextCycleAt)--> POWER_SETTLE (boost rail on, 1 s)
//        --> WARMUP (staged onCycleStart; wait max driver warmup, ~30 s)
//        --> READ_REPORT (read all, print DATA line, onCycleEnd all,
//                         boost rail off) --> IDLE
// Continuous: boost stays on, warm-up 0, effective cycle ~5.5 s (just above
// the SCD41's 5 s periodic data rate).
//
// Warm-up is STAGED: each driver's onCycleStart() fires at
// (maxWarmup - driver.warmupMs()) into the warm-up window, so every driver
// finishes its own warm-up at the same read instant. This keeps the SGP41's
// conditioning at exactly its 10 s datasheet maximum even when the SEN50's
// 30 s fan warm-up dominates the cycle.
class CadenceScheduler {
 public:
  CadenceScheduler(ISensorDriver** drivers, size_t count, int boostEnPin)
      : drivers_(drivers), count_(count), boostEnPin_(boostEnPin) {}

  // Apply cadence: configures every driver, switches the rail for continuous
  // mode, schedules the first cycle `firstDelayMs` from now.
  void applyCadence(const cadence::Cadence& c, uint32_t firstDelayMs = 0);
  // Force a cycle now (serial `sample` command). If a cycle is already in
  // flight, another full cycle runs right after it; the regular cadence
  // re-anchors to the forced cycle's start.
  void triggerNow();
  // Call from loop(). Prints DATA/CYCLE lines when a cycle completes.
  void tick();

  // Called after every completed cycle with the fresh readings and the
  // millis() of the read instant (same t as the DATA line). Ticket 06 feeds
  // the BLE characteristics and the history ring buffer from here.
  using ReportHook = void (*)(const Readings& r, uint32_t tMs);
  void setReportHook(ReportHook hook) { hook_ = hook; }

  const cadence::Cadence& current() const { return cadence_; }

 private:
  enum class State : uint8_t { Idle, PowerSettle, Warmup, ReadReport };

  void setBoost(bool on);
  uint32_t maxWarmupMs() const;
  void report(const Readings& r, uint32_t tMs);

  ISensorDriver** drivers_;
  size_t count_;
  int boostEnPin_;
  cadence::Cadence cadence_;
  ReportHook hook_ = nullptr;
  State state_ = State::Idle;
  bool triggerPending_ = false;  // `sample` requested; consumed at cycle start
  uint32_t startedMask_ = 0;     // drivers whose onCycleStart fired this cycle
  uint32_t nextCycleAt_ = 0;   // millis() of next cycle start
  uint32_t stateSince_ = 0;    // millis() when current state entered
  uint32_t cycleStart_ = 0;    // millis() when this cycle began
  uint32_t cycleCount_ = 0;
};
