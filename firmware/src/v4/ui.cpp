#include "ui.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>

#include "pins.h"

namespace {

constexpr uint8_t kOledAddr = 0x3C;
constexpr int16_t kWidth = 128;
constexpr int16_t kHeight = 64;
constexpr uint32_t kDebounceMs = 30;
constexpr uint32_t kLongPressMs = 1200;
constexpr uint32_t kScreenTimeoutMs = 30000;

Adafruit_SSD1306 display(kWidth, kHeight, &Wire, /*rst_pin=*/-1);

constexpr uint8_t kPageCount = 4;

// Cadence menu: order mirrors cadence::Mode (0..2) + Custom submenu entry.
constexpr uint8_t kMenuCount = 4;
const char* const kMenuLabels[kMenuCount] = {"Continuous", "60 s", "300 s",
                                             "Custom..."};
constexpr uint8_t kCustomCount = 5;
constexpr uint32_t kCustomSeconds[kCustomCount] = {10, 30, 600, 1800, 3600};
const char* const kCustomLabels[kCustomCount] = {"10 s", "30 s", "10 min",
                                                 "30 min", "1 h"};

void printValue(float v, uint8_t decimals, const char* unit) {
  if (isnan(v)) {
    display.print("--");
  } else {
    display.print(v, decimals);
  }
  display.print(unit);
}

}  // namespace

bool Ui::begin() {
  present_ = display.begin(SSD1306_SWITCHCAPVCC, kOledAddr);
  if (!present_) return false;
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 16);
  display.println("AirSense");
  display.setCursor(10, 36);
  display.println("v4 boot");
  display.display();
  return true;
}

void Ui::sleepAfterBoot() {
  if (!present_) return;
  mode_ = Mode::Off;
  display.clearDisplay();
  display.display();
  Serial.println("UI screen=off");
}

void Ui::setData(const Readings& r, uint32_t tMs) {
  last_ = r;
  lastTMs_ = tMs;
  if (mode_ == Mode::Pages) redraw();
}

void Ui::setStatus(const cadence::Cadence& persisted, bool bleConnected) {
  // Called every loop iteration: redraw only on a real change (the status
  // page otherwise shows a stale cadence until the next data cycle).
  bool changed = persisted.mode != persisted_.mode ||
                 persisted.customSeconds != persisted_.customSeconds ||
                 bleConnected != bleConnected_;
  persisted_ = persisted;
  bleConnected_ = bleConnected;
  if (changed && mode_ == Mode::Pages) redraw();
}

void Ui::tick() {
  if (!present_) return;
  uint32_t now = millis();

  bool pressed = (digitalRead(pins::kButton) == LOW);
  if (pressed != rawPressed_ && now - lastEdgeMs_ >= kDebounceMs) {
    rawPressed_ = pressed;
    lastEdgeMs_ = now;
    if (pressed) {
      pressedSince_ = now;
      longFired_ = false;
    } else if (!longFired_) {
      handleShort();
      lastActivityMs_ = now;
    }
  }
  // Long press fires while still held (snappier menu entry); the matching
  // release is then ignored.
  if (rawPressed_ && !longFired_ && now - pressedSince_ >= kLongPressMs) {
    longFired_ = true;
    handleLong();
    lastActivityMs_ = now;
  }

  if (mode_ != Mode::Off && now - lastActivityMs_ >= kScreenTimeoutMs) {
    sleep();
  }
}

void Ui::handleShort() {
  switch (mode_) {
    case Mode::Off:
      wake();
      break;
    case Mode::Pages:
      pageIdx_ = (pageIdx_ + 1) % kPageCount;
      Serial.print("UI page=");
      Serial.println(pageIdx_);
      redraw();
      break;
    case Mode::Menu:
      menuIdx_ = (menuIdx_ + 1) % kMenuCount;
      Serial.print("UI menu opt=");
      Serial.println(kMenuLabels[menuIdx_]);
      redraw();
      break;
    case Mode::CustomMenu:
      customIdx_ = (customIdx_ + 1) % kCustomCount;
      Serial.print("UI custom opt=");
      Serial.println(kCustomLabels[customIdx_]);
      redraw();
      break;
  }
}

void Ui::handleLong() {
  switch (mode_) {
    case Mode::Off:
      wake();  // a long press from off only wakes
      break;
    case Mode::Pages:
      mode_ = Mode::Menu;
      // Start on the currently persisted mode.
      menuIdx_ = static_cast<uint8_t>(persisted_.mode);
      if (menuIdx_ >= kMenuCount) menuIdx_ = 0;
      Serial.println("UI menu open");
      redraw();
      break;
    case Mode::Menu:
      if (menuIdx_ == 3) {
        mode_ = Mode::CustomMenu;
        customIdx_ = 0;
        Serial.println("UI custom open");
      } else {
        cadence::Cadence c = persisted_;
        c.mode = static_cast<cadence::Mode>(menuIdx_);
        Serial.print("UI menu select mode=");
        Serial.println(c.modeName());
        if (cadenceCb_ != nullptr) cadenceCb_(c);
        mode_ = Mode::Pages;
      }
      redraw();
      break;
    case Mode::CustomMenu: {
      cadence::Cadence c = persisted_;
      c.mode = cadence::Mode::Custom;
      c.customSeconds = kCustomSeconds[customIdx_];
      Serial.print("UI menu select mode=custom ");
      Serial.print(c.customSeconds);
      Serial.println("s");
      if (cadenceCb_ != nullptr) cadenceCb_(c);
      mode_ = Mode::Pages;
      redraw();
      break;
    }
  }
}

void Ui::wake() {
  mode_ = Mode::Pages;
  pageIdx_ = 0;
  Serial.println("UI screen=on page=0");
  redraw();
}

void Ui::sleep() {
  mode_ = Mode::Off;
  display.clearDisplay();
  display.display();
  Serial.println("UI screen=off");
}

void Ui::redraw() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (mode_ == Mode::Menu || mode_ == Mode::CustomMenu) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(mode_ == Mode::Menu ? "Cadence" : "Custom interval");
    uint8_t count = (mode_ == Mode::Menu) ? kMenuCount : kCustomCount;
    uint8_t sel = (mode_ == Mode::Menu) ? menuIdx_ : customIdx_;
    for (uint8_t i = 0; i < count; ++i) {
      display.setCursor(0, 14 + i * 10);
      display.print(i == sel ? "> " : "  ");
      display.println(mode_ == Mode::Menu ? kMenuLabels[i] : kCustomLabels[i]);
    }
    display.display();
    return;
  }

  // Value pages
  display.setTextSize(1);
  display.setCursor(0, 0);
  switch (pageIdx_) {
    case 0:
      display.println("Air  [1/4]");
      display.setTextSize(2);
      display.setCursor(0, 14);
      if (last_.scd41Valid) {
        display.print(static_cast<int>(last_.co2Ppm));
        display.println(" ppm");
      } else {
        display.println("-- ppm");
      }
      display.setTextSize(1);
      display.setCursor(0, 38);
      display.print("Temp ");
      printValue(last_.tempC, 1, " C");
      display.setCursor(0, 50);
      display.print("RH   ");
      printValue(last_.rhPct, 0, " %");
      break;
    case 1:
      display.println("PM ug/m3  [2/4]");
      display.setCursor(0, 16);
      display.print("PM1.0  ");
      printValue(last_.pm1_0, 1, "");
      display.setCursor(0, 30);
      display.print("PM2.5  ");
      printValue(last_.pm2_5, 1, "");
      display.setCursor(0, 44);
      display.print("PM10   ");
      printValue(last_.pm10, 1, "");
      break;
    case 2:
      display.println("Gas + pressure  [3/4]");
      display.setCursor(0, 16);
      display.print("VOC idx ");
      if (last_.sgp41Valid) display.print(last_.vocIndex);
      else display.print("--");
      display.setCursor(0, 30);
      display.print("NOx idx ");
      if (last_.sgp41Valid) display.print(last_.noxIndex);
      else display.print("--");
      display.setCursor(0, 44);
      display.print("Press  ");
      printValue(isnan(last_.pressurePa) ? NAN : last_.pressurePa / 100.0f, 1,
                 " hPa");
      break;
    case 3:
      display.println("Status  [4/4]");
      display.setCursor(0, 16);
      display.print("Cadence ");
      display.print(persisted_.modeName());
      if (persisted_.mode == cadence::Mode::Custom) {
        display.print(' ');
        display.print(persisted_.customSeconds);
        display.print('s');
      }
      display.setCursor(0, 30);
      display.print("BLE ");
      display.print(bleConnected_ ? "connected" : "advertising");
      display.setCursor(0, 44);
      display.print("Up ");
      display.print(lastTMs_ / 1000);
      display.print("s  scrn=cont");
      break;
  }
  display.display();
}
