# Wearable Air Quality Sensor

A small wearable air quality sensor that connects to a phone via Bluetooth Low Energy (BLE). Monitors CO2, particulate matter (PM1.0/PM2.5/PM10), VOCs, NOx, temperature, humidity, and barometric pressure.

## Features

- Real-time CO2 monitoring (400-5000 ppm)
- Particulate matter: PM1.0 / PM2.5 / PM10 (AirSense v4)
- VOC and NOx index detection
- Temperature, humidity, and barometric pressure sensing
- BLE connectivity for mobile app, with on-device history sync (v4)
- Configurable sampling cadence (continuous / 60 s / 300 s / custom, v4)
- OLED display with button-driven pages and cadence menu
- Portable battery power (1-2 day runtime)

## Hardware

### Core Components
- **MCU:** Seeed XIAO ESP32-C3 (RISC-V, BLE 5.0)
- **CO2 Sensor:** Adafruit SCD41 (NDIR CO2 + temp + humidity)
- **VOC Sensor:** Adafruit SGP41 (VOC Index + NOx Index)
- **Display:** SSD1306 OLED 0.96" 128x64 I2C
- **Battery:** LiPo 3.7V 500mAh
- **Total Cost:** ~$109

### I2C Address Map
- SCD41: 0x62
- SGP41: 0x59
- SSD1306: 0x3C

## Project Structure

```
air-quality-sensor/
├── firmware/           # ESP32-C3 firmware (PlatformIO)
│   ├── platformio.ini  #   envs: seeed_xiao_esp32c3 (Phase 2), airsense_v4
│   ├── src/            #   Phase-2 firmware
│   ├── src/v4/         #   AirSense v4: sensor stack, cadence, BLE, OLED UI
│   └── wokwi-v4/       #   v4 simulation: custom chip models, scenarios
├── app/                # Mobile app (React Native / Expo)
│   └── constants/ble-protocol.md  # authoritative v4 BLE contract
├── hardware/           # KiCad projects, PCB docs, BOM
│   ├── kicad_mcp/      #   v3 board (KiCad MCP design iteration)
│   └── v4/             #   AirSense v4 rev B board + fab pack + order BOM
├── scripts/            # ble_verify.py — v4 BLE protocol verification harness
├── enclosure/          # 3D printable case files
└── docs/               # Documentation and wiring diagrams
```

## Getting Started

### Prerequisites

1. Install PlatformIO (VS Code extension or CLI):
   ```bash
   pip install platformio
   ```

2. For mobile app development:
   - React Native: Node.js + Expo CLI
   - OR Flutter: Flutter SDK

### Firmware Development

```bash
cd firmware
pio run -t upload      # Build and upload firmware
pio device monitor     # View serial output
```

### Development Phases

- **Phase 1:** ✅ Basic sensor reading (SCD41) - COMPLETE
- **Phase 2:** ✅ BLE communication with mobile app - COMPLETE
- **Phases 3-5** (VOC sensor, battery power, OLED) are superseded by the
  **AirSense v4** revision below, which delivers all of them plus PM and
  pressure sensing on a custom PCB - design and sim verification COMPLETE
- **Next:** v4 parts order + hardware bring-up, then enclosure design

### Current Status (Phase 2 Complete)

**Working Features:**
- ✅ SCD41 CO2/Temperature/Humidity sensor readings
- ✅ BLE advertising and communication
- ✅ React Native mobile app with Expo SDK 54
- ✅ Real-time data transmission over BLE
- ✅ Web demo deployed to GitHub Pages
- ✅ Web Bluetooth support (Android/Desktop browsers)
- ✅ Historical data charts and alerts
- ✅ Demo mode for testing without hardware
- ✅ Android development build ready

**Technology Stack:**
- Expo SDK 54.0.0
- React Native 0.81.5
- React 19.1.0
- Expo Router 6.0.21
- BLE Library: @sfourdrinier/react-native-ble-plx (RN 0.81 compatible)

**Live Demos:**
- 🌐 **Web App**: https://havamal-65.github.io/air-quality-sensor
  - ✅ Works on Android (Chrome/Edge) with real BLE
  - ✅ Works on Desktop with demo mode
  - ⚠️ iOS Safari: Demo mode only (Apple blocks Web Bluetooth)
- 📱 **Mobile (Expo Go)**: Run `npm start` and scan QR code
  - ✅ Full BLE support on iOS and Android
  - ⚠️ Requires computer running

**How to Use:**

1. **Web Browser (No Install Required)**
   - Open https://havamal-65.github.io/air-quality-sensor on Android Chrome/Edge
   - Tap "Connect Sensor" and select your AirSense device
   - Real-time data appears instantly

2. **Expo Go App (Development)**
   ```bash
   cd app
   npm install
   npm start
   ```
   - Scan QR code with Expo Go app
   - Full native BLE support

3. **Standalone App (Coming Soon)**
   - Build with EAS: `npx eas-cli build --platform ios/android`
   - Install via TestFlight (iOS) or APK (Android)
   - Works anywhere without computer

**Next Steps:**
- Build standalone iOS/Android apps with EAS
- Connect SGP41 VOC/NOx sensor (Phase 3)
- See `firmware/PHASE2-BLE-INSTRUCTIONS.md` for detailed setup

### Hardware Design Status (KiCad MCP, v3 board)

- KiCad project path: `hardware/kicad_mcp/air-quality-sensor-mcp.kicad_pro`
- Schematic: populated and wired (`19` symbols incl. PMSA003I PM sensor and 5 V boost), ERC `0` errors
- PCB: `19` components placed, nets synced (`65` pad nets), routing complete (`160` tracks), DRC `0` errors / `0` warnings
- Manufacturing outputs: Gerbers, drill, BOM, and PnP generated under `hardware/kicad_mcp/manufacturing/`
- Details: `hardware/kicad_mcp/README.md` and `hardware/kicad_mcp/BOARD-DESIGN-NOTES.md`
- Lessons learned: `docs/LESSONS-LEARNED.md`

## AirSense v4 (rev B) — current hardware/firmware track

Full product revision designed and verified 2026-10: custom 100 × 40 mm
2-layer board (ESP32-C3-WROOM-02, SCD41 CO2, SGP41 VOC/NOx, SEN50 PM,
BMP384 pressure, SSD1306 OLED, USB-C, LiPo charge + 5 V boost) with the
firmware, BLE protocol, and verification tooling to match.

**Hardware (`hardware/v4/`)**
- `airsense_v4.kicad_pro/sch/pcb` — DRC-clean board (0 errors), designed
  end to end through KiCad MCP tools with project-local footprints for
  SGP41 and BMP384 (`footprint-verification.md` records the datasheet
  checks; `design-rules.md` records the PCBWay rule set)
- `fab/` — full fab pack: Gerbers (+ PCBWay-profile zip), drill, BOM,
  pick-and-place, STEP model
- `order-bom.md` — priced order list (~$96/device at qty 1, live-priced
  2026-10-13)

**Firmware (`firmware/src/v4/`, PlatformIO env `airsense_v4`)**
- Per-sensor drivers behind one `ISensorDriver` seam, staged warm-up
  scheduler (each sensor gets exactly its own datasheet warm-up), NVS-persisted
  sampling cadence (continuous / 60 s / 300 s / custom 10 s–24 h)
- BLE: serves the frozen Phase-2 contract unchanged, plus v4
  characteristics — pressure, PM (standard + full-resolution custom),
  writable cadence, and a 720-record history ring with framed dump sync.
  Contract documented in `app/constants/ble-protocol.md`
- OLED UI: 4 data pages, button wake/cycle, long-press cadence menu with
  custom-interval submenu, 30 s screen timeout; screen-on or an active BLE
  connection forces continuous sampling, the persisted cadence returns after
- Simulation: `firmware/wokwi-v4/` has custom Wokwi chip models
  (SCD41/SGP41/SEN50/BMP384 register-level, power-pin gated), three scenario
  scripts, OLED screenshot capture, and `check_cadence.py` which verifies
  sample spacing against the firmware's own interval prints

**Verification tooling**
- `scripts/ble_verify.py` — PC-side BLE harness (Python + bleak) asserting
  the whole protocol contract against a live device: GATT table, value
  formats, cadence write/readback, history dump framing, and duty-cadence
  observation via history after a disconnect gap; `--self-test` runs its
  decoder suite with no hardware

**Status:** design, firmware, and simulation verification complete
(schematic → layout → DRC → fab pack; firmware verified in Wokwi:
cadence spacing, NVS persistence across reboot, OLED pages/menu flows,
BLE boot). Next: parts order and hardware bring-up with `ble_verify.py`.

## Air Quality Thresholds

### CO2 Levels
| Level | Range (ppm) | Color | Action |
|-------|-------------|-------|--------|
| Excellent | <600 | Green | None |
| Good | 600-800 | Green | None |
| Moderate | 800-1000 | Yellow | Consider ventilation |
| Poor | 1000-1500 | Orange | Open windows |
| Bad | >1500 | Red | Ventilate immediately |

### VOC Index
| Level | Range | Color | Action |
|-------|-------|-------|--------|
| Good | 0-150 | Green | None |
| Moderate | 150-250 | Yellow | Check for sources |
| Poor | 250-400 | Orange | Ventilate |
| Bad | >400 | Red | Remove source |

## Documentation

See `/docs` for:
- Wiring diagrams
- BOM (Bill of Materials)
- Sensor specifications
- BLE service UUIDs

## Resources

- [SCD41 Datasheet](https://sensirion.com/media/documents/E0F04247/631EF271/CD_DS_SCD40_SCD41_Datasheet_D1.pdf)
- [SGP41 Datasheet](https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf)
- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)

## License

MIT
