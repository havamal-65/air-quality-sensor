# AirSense - Air Quality Monitor

Monitor CO2, VOC, NOx, temperature, and humidity in real-time with your wearable sensor.

## How to Use

### Option 1: Web Browser (Easiest)

1. Open https://havamal-65.github.io/air-quality-sensor on your phone
2. Tap "Connect Sensor"
3. Select your AirSense device from the list
4. View real-time air quality data

**Works on:**
- ✅ Android (Chrome or Edge browser)
- ✅ Desktop computers (demo mode only)
- ❌ iPhone (demo mode only - Apple blocks Bluetooth in browsers)

### Option 2: Expo Go App (Full Features)

1. Install "Expo Go" from App Store (iPhone) or Play Store (Android)
2. Ask the app owner to start the development server
3. Scan the QR code shown on their computer
4. App launches with full Bluetooth support

**Works on:**
- ✅ iPhone and iPad
- ✅ Android phones and tablets

### Option 3: Standalone App (Not Available Yet)

A standalone app that works anywhere is coming soon.

## What You See

- **Air Quality Score**: Overall rating from 0-100
- **Live Readings**: CO2, VOC, NOx, Temperature, Humidity
- **History Charts**: View trends over 1 hour to 30 days
- **Recommendations**: Tips to improve your air quality
- **Alerts**: Notifications when levels are unhealthy

## Troubleshooting

**"Can't connect to device"**
- Make sure Bluetooth is turned on
- Keep sensor within 30 feet
- Try refreshing the page

**"Demo mode" message**
- You're seeing simulated data
- Use Android Chrome/Edge for real data
- Or use Expo Go app for full features

**App shows old data**
- Pull down to refresh
- Reconnect to sensor

## Air Quality Levels

| CO2 Level | What it Means |
|-----------|---------------|
| <600 ppm | Excellent - Fresh air |
| 600-800 | Good - Normal indoor |
| 800-1000 | Moderate - Consider ventilation |
| 1000-1500 | Poor - Open windows |
| >1500 | Bad - Ventilate immediately |

## Questions?

- Web app: https://havamal-65.github.io/air-quality-sensor
- Source code: https://github.com/havamal-65/air-quality-sensor
