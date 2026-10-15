#pragma once
#include <stdint.h>

#include "cadence.h"
#include "readings.h"

// On-device UI (ticket 08, spec AC-11): SSD1306 OLED pages + user button.
//
// Model:
//   screen off  --press-->  pages (page 0)
//   pages: short press = next page, long press (>=1.2 s) = cadence menu
//   menu: short = next option, long = select; "Custom" opens a preset
//         submenu (10 s / 30 s / 10 min / 30 min / 1 h), same controls
//   30 s without a press -> screen off (and menu abandoned)
//
// While the screen is on the device samples continuously (same forcing as a
// BLE connection); main arbitrates via screenOn(). Every transition prints a
// "UI ..." serial line so simulation scenarios can assert the state machine.
class Ui {
 public:
  // A menu selection hands the chosen cadence to main (persist + apply).
  using CadenceSelect = void (*)(const cadence::Cadence& c);

  // Initialize the SSD1306 (0x3C, shared Wire) and show the boot splash.
  // Returns false when the display does not answer (headless operation:
  // every other method then no-ops safely).
  bool begin();
  // Blank the display at the end of setup (boot splash ends; screen starts
  // in the off state so the boot cadence is the persisted one).
  void sleepAfterBoot();
  // Button sampling, long-press detection, screen timeout, redraw. Call
  // every loop() pass.
  void tick();
  // Latest completed cycle's readings (shown on the value pages).
  void setData(const Readings& r, uint32_t tMs);
  // Status page inputs: the persisted cadence and BLE connection state.
  void setStatus(const cadence::Cadence& persisted, bool bleConnected);
  void setCadenceCallback(CadenceSelect cb) { cadenceCb_ = cb; }

  bool screenOn() const { return mode_ != Mode::Off; }

 private:
  enum class Mode : uint8_t { Off, Pages, Menu, CustomMenu };

  void handleShort();
  void handleLong();
  void wake();
  void sleep();
  void redraw();

  Mode mode_ = Mode::Off;
  bool present_ = false;
  uint8_t pageIdx_ = 0;
  uint8_t menuIdx_ = 0;
  uint8_t customIdx_ = 0;
  CadenceSelect cadenceCb_ = nullptr;

  // Button state (active low, debounced, long-press fires while held)
  bool rawPressed_ = false;
  uint32_t pressedSince_ = 0;
  uint32_t lastEdgeMs_ = 0;
  bool longFired_ = false;

  uint32_t lastActivityMs_ = 0;

  Readings last_;
  uint32_t lastTMs_ = 0;
  cadence::Cadence persisted_;
  bool bleConnected_ = false;
};
