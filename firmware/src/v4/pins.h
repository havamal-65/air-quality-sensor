#pragma once
#include <stdint.h>

// AirSense v4 (rev B) pin map — traced from hardware/v4/airsense_v4.kicad_sch
// (U4 = ESP32-C3-WROOM-02; module pin -> GPIO -> net)
namespace pins {
constexpr int kI2cSda  = 4;  // U4 pin 3  (IO4)  — net SDA
constexpr int kI2cScl  = 5;  // U4 pin 4  (IO5)  — net SCL
constexpr int kBoostEn = 3;  // U4 pin 15 (IO3)  — net BOOST_EN (TPS61023 EN, 5 V rail for SEN50)
constexpr int kButton  = 9;  // U4 pin 8  (IO9)  — net GPIO9_BTN (BOOT/USER, active low)
}  // namespace pins

// I2C addresses (bus: SDA/SCL above, 10 k pull-ups on board)
constexpr uint8_t kAddrScd41  = 0x62;
constexpr uint8_t kAddrSgp41  = 0x59;
constexpr uint8_t kAddrSen50  = 0x69;
constexpr uint8_t kAddrBmp3xx = 0x76;  // U7 SDO -> GND
constexpr uint8_t kAddrOled   = 0x3C;  // ticket 08
