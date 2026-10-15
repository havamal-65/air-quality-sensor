#pragma once
#include <stdint.h>

#include "cadence.h"
#include "history.h"
#include "readings.h"

// BLE GATT server for the AirSense contract (ticket 06).
//
// Serves the frozen Phase-2 layout unchanged (AC-8) plus the v4 additions
// (AC-9): pressure + standard PM in the Environmental Sensing service,
// full-resolution PM + cadence + history sync in the custom service. UUIDs
// and wire formats live in ble_protocol.h and app/constants/ble-protocol.md.
//
// Threading: Bluedroid callbacks only set flags/copy bytes. All real work
// (cadence changes, scheduler transitions, history dump notifications)
// happens in poll(), called from loop() — never concurrent with the sensor
// cycle or the I2C bus.
class BleService {
 public:
  // Actions for main to execute after poll() drains the pending flags.
  struct PollResult {
    bool cadenceChanged = false;  // newCadence is valid: save + maybe apply
    cadence::Cadence newCadence;
    bool connected = false;     // app connected: force continuous sampling
    bool disconnected = false;  // app left: restore the persisted cadence
  };

  // Start the GATT server and advertising. `persisted` seeds the cadence
  // characteristic; `buf` backs the history characteristic.
  void begin(const cadence::Cadence& persisted, history::Buffer* buf);
  // Push a completed cycle into every characteristic value (+ notify when a
  // client subscribed). tMs = millis() at the read instant.
  void publish(const Readings& r, uint32_t tMs);
  // Drain BLE-callback flags, restart advertising after a disconnect, and
  // stream pending history frames. Call every loop() pass.
  PollResult poll();
  // Reflect a cadence change (serial or BLE) into the characteristic.
  void setPersisted(const cadence::Cadence& c);
  bool isConnected() const;
  // True while a history dump streams. The caller skips history pushes then,
  // so the ring's oldest-record mapping stays frozen for the whole dump (the
  // app receives those cycles live over the value notifications anyway).
  bool isDumping() const;
};
