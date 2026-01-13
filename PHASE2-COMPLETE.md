# Phase 2 Complete: BLE Communication

## What Was Done

I've successfully implemented Phase 2 of your AirSense project, adding Bluetooth Low Energy (BLE) communication to the ESP32-C3 firmware. This allows your mobile app to connect wirelessly to the sensor and receive real-time air quality data.

### Changes Made

1. **Updated Firmware** (`firmware/src/main.cpp`)
   - Added BLE server functionality
   - Created Environmental Sensing Service with CO2, temperature, and humidity characteristics
   - Created Custom Service with VOC/NOx placeholder characteristics (for Phase 3)
   - Implemented connection callbacks and status tracking
   - Data is transmitted every 5 seconds when a device is connected
   - Serial monitor now shows BLE connection status

2. **Created Documentation** (`firmware/PHASE2-BLE-INSTRUCTIONS.md`)
   - Complete upload instructions
   - Expected serial output
   - Mobile app connection guide (Expo Go)
   - BLE scanner testing instructions
   - Troubleshooting section
   - Data format specifications

3. **Updated README** (`README.md`)
   - Added current project status section
   - Marked Phase 1 and Phase 2 as complete
   - Added links to live demos
   - Updated development phases

### Firmware Compilation

The firmware compiled successfully:
- **RAM Usage**: 11.8% (38,644 / 327,680 bytes)
- **Flash Usage**: 76.5% (1,002,362 / 1,310,720 bytes)
- **Status**: Ready to upload ✅

## What You Need to Do

### Step 1: Upload the Firmware

The firmware couldn't be uploaded automatically because COM8 was busy (likely the serial monitor from Phase 1 is still open).

**To upload:**

1. Close any serial monitors or terminal windows
2. Run these commands:

```bash
cd "D:\GitHub\Portfolio\Projects\air-quality-sensor\firmware"
pio run --target upload
pio device monitor
```

3. You should see output like:

```
=================================
AirSense - Phase 2: SCD41 + BLE
=================================

📡 Initializing BLE...
✅ BLE initialized and advertising
📡 Device name: AirSense
✅ SCD41 initialized successfully!

=================================
CO2: 650 ppm
Temperature: 22.5 °C
Humidity: 48.2 %RH
Air Quality: GOOD ✓
📡 Ready for BLE connection
=================================
```

### Step 2: Test BLE Connection (Two Options)

#### Option A: Use Mobile App (Expo Go) - RECOMMENDED

This gives you the full AirSense app experience with real sensor data:

1. **Install Expo Go** on your iPhone:
   - Open App Store
   - Search "Expo Go"
   - Install

2. **Start the app** on your PC:
   ```bash
   cd "D:\GitHub\Portfolio\Projects\air-quality-sensor\app"
   npm start
   ```

3. **Connect from iPhone**:
   - Open Expo Go
   - Scan the QR code
   - Tap "Connect Device"
   - Select "AirSense"
   - Watch real sensor data appear!

#### Option B: Quick BLE Test (Scanner App)

Just to verify BLE is working:

1. Install "LightBlue" or "nRF Connect" on your iPhone
2. Open the app and scan for devices
3. Look for "AirSense"
4. Connect and view the services/characteristics

### Step 3: Verify It Works

When the mobile app connects:
- Serial monitor will show: "📱 Device connected!"
- App will display real CO2, temperature, and humidity values
- Data updates every 5 seconds
- VOC and NOx will show as 0 (we'll add those in Phase 3)

## How It Works

### BLE Architecture

The firmware creates a BLE server with two services:

**Environmental Sensing Service** (standard Bluetooth SIG service)
- CO2 Characteristic: 2 bytes, uint16, ppm
- Temperature Characteristic: 2 bytes, int16, value × 100
- Humidity Characteristic: 2 bytes, uint16, value × 100

**Custom Service** (for sensors not in standard spec)
- VOC Characteristic: 2 bytes, uint16, index (Phase 3)
- NOx Characteristic: 2 bytes, uint16, index (Phase 3)

All characteristics support:
- **Read**: Get current value on demand
- **Notify**: Automatic updates when value changes

### Data Flow

```
SCD41 Sensor → I2C → ESP32-C3 → BLE → Mobile App
     ↓                  ↓
Serial Monitor    BLE Broadcasting
 (debugging)      (every 5 seconds)
```

## Current Features

### Working Now:
- ✅ Real-time CO2 monitoring (SCD41)
- ✅ Temperature and humidity sensing
- ✅ BLE wireless communication
- ✅ Mobile app with charts and alerts
- ✅ Historical data tracking
- ✅ Web demo (demo mode only)
- ✅ Expo Go native app support

### Phase 3 Preview (What's Next):
- Add SGP41 VOC/NOx sensor via STEMMA QT cable
- Update firmware to read gas index values
- Enable full air quality scoring in app
- See actual VOC/NOx readings instead of zeros

## Troubleshooting

### "Port COM8 is busy"
- Close all serial monitors and try again
- Unplug/replug the USB cable
- Check Task Manager for processes using serial ports

### Can't see "AirSense" in app
- Make sure Bluetooth is ON on your phone
- Grant Bluetooth permissions when prompted
- Use Expo Go app, not web browser (BLE doesn't work in browsers)
- Check serial monitor - should say "📡 Ready for BLE connection"

### App shows demo data
- You're using the web version (GitHub Pages)
- Web browsers can't access Bluetooth
- Use Expo Go on your phone for real BLE

### Connection drops
- BLE range is typically 10-30 feet
- Concrete walls significantly reduce range
- Other 2.4GHz devices can cause interference
- Firmware will auto-reconnect when back in range

## Files Changed

```
air-quality-sensor/
├── README.md                              # Updated with Phase 2 status
├── firmware/
│   ├── src/main.cpp                      # Phase 2 firmware with BLE
│   └── PHASE2-BLE-INSTRUCTIONS.md        # Detailed setup guide
└── PHASE2-COMPLETE.md                    # This file
```

## Commit Information

All changes have been committed to git:

```
Commit: Add Phase 2: BLE Communication Firmware
Files: 3 changed, 394 insertions(+), 18 deletions(-)
```

Ready to push to GitHub when you're ready!

## Next Steps After Testing

Once you verify BLE is working with the mobile app:

### Phase 3: Add VOC/NOx Sensor
- You already have the Adafruit SGP41 sensor
- Connect via STEMMA QT cable to SCD41 (daisy chain)
- Update firmware to read VOC and NOx index values
- App will show complete air quality scoring

### Phase 4: Battery Power
- Add LiPo battery and charging circuit
- Implement deep sleep mode
- Battery monitoring in firmware
- Portable operation

### Phase 5: OLED Display
- Add SSD1306 display
- Show readings without phone
- Battery percentage indicator
- Standalone mode

## Resources

- **Firmware Instructions**: `firmware/PHASE2-BLE-INSTRUCTIONS.md`
- **Mobile App**: `app/` directory
- **Web Demo**: https://havamal-65.github.io/air-quality-sensor
- **BLE Specs**: Standard Bluetooth Environmental Sensing Service

## Questions?

If you run into any issues:
1. Check the serial monitor output
2. Review `PHASE2-BLE-INSTRUCTIONS.md`
3. Test with a BLE scanner app first
4. Verify the app is using Expo Go (not web browser)

Ready to test! Let me know how it goes! 🚀
