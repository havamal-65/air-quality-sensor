# Air Quality Monitor - KiCad Custom Libraries

## Components (Based on Original BOM)

| Ref | Component | I2C Address | Footprint |
|-----|-----------|-------------|-----------|
| U1 | Seeed XIAO ESP32-C3 | - | XIAO_ESP32C3 |
| U2 | TP4056 USB-C Charger | - | TP4056_USB-C_Module |
| U3 | Adafruit SCD41 (CO2/Temp/Humidity) | 0x62 | Adafruit_SCD41 |
| U4 | SSD1306 OLED 128x64 | 0x3C | SSD1306_OLED_128x64 |
| U5 | Adafruit SGP41 (VOC/NOx) | 0x59 | Adafruit_SGP41 |
| U6 | Adafruit PMSA003I (PM2.5) | 0x12 | Adafruit_PMSA003I |
| BT1 | JST-PH 2-pin Battery | - | JST_PH_2pin_Battery |

## Library Files

- **Symbol Library**: `AirQuality_Sensors.kicad_sym`
- **Footprint Library**: `AirQuality.pretty/` (folder)

## How to Add Libraries to KiCad 9.0

### Add Symbol Library

1. Open KiCad project
2. Go to **Preferences → Manage Symbol Libraries**
3. Click **Project Specific Libraries** tab
4. Click the folder icon (📁) to add a library
5. Navigate to: `hardware/kicad_libraries/AirQuality_Sensors.kicad_sym`
6. Click **OK**

### Add Footprint Library

1. Open KiCad project  
2. Go to **Preferences → Manage Footprint Libraries**
3. Click **Project Specific Libraries** tab
4. Click the folder icon (📁) to add a library
5. Navigate to: `hardware/kicad_libraries/AirQuality.pretty`
6. Set **Nickname** to: `AirQuality`
7. Click **OK**

## Footprint Pinouts

### Adafruit I2C Sensors (SCD41, SGP41, PMSA003I)
All use standard STEMMA QT header pinout:
- Pin 1: VIN (3.3V-5V)
- Pin 2: GND
- Pin 3: SCL
- Pin 4: SDA

### SSD1306 OLED Display
- Pin 1: GND
- Pin 2: VCC (3.3V)
- Pin 3: SCL
- Pin 4: SDA

### XIAO ESP32-C3
14-pin SMD castellated edge connector with battery pads (B+, B-)

### TP4056 Charger Module
- Pins 1-2: USB Input (IN+, IN-)
- Pins 3-4: Battery (B+, B-)
- Pins 5-6: Output (OUT+, OUT-)

### JST-PH Battery Connector
- Pin 1: VBAT (+)
- Pin 2: GND (-)

## Notes

- These footprints are for **breakout boards with pin headers**, not raw sensor ICs
- All I2C sensors share the same bus (requires 4.7kΩ pull-up resistors)
- XIAO footprint includes battery pads for direct LiPo connection
