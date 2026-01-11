# Wearable Air Quality Sensor

A small wearable air quality sensor that connects to a phone via Bluetooth Low Energy (BLE). Monitors CO2, VOCs, NOx, temperature, and humidity.

## Features

- Real-time CO2 monitoring (400-5000 ppm)
- VOC and NOx index detection
- Temperature and humidity sensing
- BLE connectivity for mobile app
- OLED display for standalone viewing
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
│   ├── platformio.ini
│   ├── src/
│   └── lib/
├── app/               # Mobile app (React Native/Flutter)
├── enclosure/         # 3D printable case files
└── docs/              # Documentation and wiring diagrams
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
- **Phase 3:** Add VOC sensor (SGP41) - READY TO START
- **Phase 4:** Battery power system
- **Phase 5:** OLED display
- **Phase 6:** Enclosure design

### Current Status (Phase 2 Complete)

**Working Features:**
- ✅ SCD41 CO2/Temperature/Humidity sensor readings
- ✅ BLE advertising and communication
- ✅ React Native mobile app with Expo
- ✅ Real-time data transmission over BLE
- ✅ Web demo deployed to GitHub Pages
- ✅ Historical data charts and alerts
- ✅ Demo mode for testing without hardware

**Live Demos:**
- Web App: https://havamal-65.github.io/air-quality-sensor (demo mode)
- Mobile: Use Expo Go to scan QR code for real BLE connection

**Next Steps:**
- Connect SGP41 VOC/NOx sensor (Phase 3)
- See `firmware/PHASE2-BLE-INSTRUCTIONS.md` for detailed setup

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
