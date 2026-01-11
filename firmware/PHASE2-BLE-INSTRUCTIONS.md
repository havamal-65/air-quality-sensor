# Phase 2: BLE Firmware Upload Instructions

## What's New in Phase 2

Phase 2 firmware adds Bluetooth Low Energy (BLE) communication to your AirSense device. This allows the mobile app to connect wirelessly and receive real-time sensor data.

### Features Added:
- BLE advertising as "AirSense"
- Real-time CO2, temperature, and humidity transmission over BLE
- Connection status indicators in serial output
- Auto-reconnection support
- Placeholder characteristics for VOC/NOx (ready for Phase 3)

## Prerequisites

1. Close any serial monitors or terminal windows connected to COM8
2. Make sure the ESP32-C3 is still connected via USB
3. Keep the SCD41 sensor wired (same connections as Phase 1)

## Upload Instructions

### Option 1: Using PlatformIO CLI

```bash
cd "D:\GitHub\Portfolio\Projects\air-quality-sensor\firmware"
pio run --target upload
```

### Option 2: Using PlatformIO Monitor (Uploads + Shows Serial Output)

```bash
cd "D:\GitHub\Portfolio\Projects\air-quality-sensor\firmware"
pio run --target upload && pio device monitor
```

## Expected Serial Output

After successful upload, you should see:

```
=================================
AirSense - Phase 2: SCD41 + BLE
=================================

📡 Initializing BLE...
✅ BLE initialized and advertising
📡 Device name: AirSense
Serial: 0x[sensor serial number]
✅ SCD41 initialized successfully!
⏳ Waiting for first measurement (5 seconds)...

=================================
CO2: 650 ppm
Temperature: 22.5 °C
Humidity: 48.2 %RH
Air Quality: GOOD ✓
📡 Ready for BLE connection
=================================
```

## Testing BLE Connection

### Using the Mobile App (Expo Go)

1. **Install Expo Go** on your iPhone:
   - Open App Store
   - Search "Expo Go"
   - Install the app

2. **Start the app development server** on your PC:
   ```bash
   cd "D:\GitHub\Portfolio\Projects\air-quality-sensor\app"
   npm start
   ```

3. **Connect from your iPhone**:
   - Open Expo Go
   - Scan the QR code displayed in the terminal
   - Wait for the app to load

4. **Connect to AirSense**:
   - Tap "Connect Device" in the app
   - Look for "AirSense" in the device list
   - Tap to connect
   - You should see "📱 Device connected!" in the serial monitor
   - Real sensor data will appear in the app

### Using a BLE Scanner App (Quick Test)

If you want to verify BLE is working without the full app setup:

1. **Download a BLE scanner** on your phone:
   - iOS: "LightBlue" or "nRF Connect"
   - Android: "nRF Connect"

2. **Scan for devices**:
   - Open the BLE scanner app
   - Look for "AirSense" in the list
   - Connect to it

3. **View services**:
   - Environmental Sensing Service: `0000181A-0000-1000-8000-00805f9b34fb`
     - CO2: `00002BD2-...` (uint16, ppm)
     - Temperature: `00002A6E-...` (int16, 0.01°C units)
     - Humidity: `00002A6F-...` (uint16, 0.01% units)
   - Custom Service: `12345678-1234-5678-1234-56789abcdef0`
     - VOC: `12345678-...-def1` (placeholder, reads 0)
     - NOx: `12345678-...-def2` (placeholder, reads 0)

4. **Enable notifications** on characteristics to see live data updates every 5 seconds

## Troubleshooting

### "Could not open COM8, port is busy"

**Solution**: Close any serial monitors or applications using COM8, then try uploading again.

```bash
# Check what's using the port
pio device list
```

### BLE not advertising

**Possible causes**:
- ESP32-C3 not powered (red LED should be on)
- Firmware not uploaded successfully
- BLE interference from other devices

**Solution**: Try resetting the device (press the reset button) and check serial output.

### Can't see "AirSense" in mobile app

**Possible causes**:
- Bluetooth is off on your phone
- You're using the GitHub Pages web demo (BLE doesn't work in browsers)
- App doesn't have Bluetooth permissions

**Solution**:
1. Make sure Bluetooth is enabled on your phone
2. Use Expo Go app, not the web version
3. Grant Bluetooth permissions when prompted

### App shows demo data instead of real data

**Cause**: You're using the web version at https://havamal-65.github.io/air-quality-sensor

**Solution**: The web version only shows demo data. Use Expo Go on your phone to connect to real hardware via BLE.

### Serial output shows connection but app doesn't

**Possible cause**: Mismatched UUIDs or data format issue

**Solution**: Check that the firmware and app are using matching UUIDs (they should be by default).

## Data Format

The firmware sends data in little-endian format:

- **CO2**: 2 bytes, uint16, direct ppm value (e.g., 650 ppm = 0x028A)
- **Temperature**: 2 bytes, int16, value × 100 (e.g., 22.5°C = 2250 = 0x08CA)
- **Humidity**: 2 bytes, uint16, value × 100 (e.g., 48.2% = 4820 = 0x12D4)
- **VOC**: 2 bytes, uint16, index value (Phase 3)
- **NOx**: 2 bytes, uint16, index value (Phase 3)

## Next Steps

Once BLE is working:

### Phase 3: Add SGP41 VOC/NOx Sensor
- Connect the Adafruit SGP41 sensor via STEMMA QT cable
- Update firmware to read VOC and NOx values
- Real air quality scores in the app

### Phase 4: Add OLED Display
- Connect SSD1306 OLED display
- Show real-time readings on the device
- Battery percentage indicator

### Phase 5: Battery Power
- Add LiPo battery management
- Enable portable operation
- Deep sleep mode for extended battery life

## Notes

- The device broadcasts continuously and will reconnect automatically if disconnected
- Serial output continues to work while BLE is active (useful for debugging)
- Current power draw: ~80-100mA (USB powered)
- BLE range: typically 10-30 feet depending on environment
