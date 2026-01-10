# AirSense Mobile App

React Native mobile application for the AirSense wearable air quality sensor.

## Features

- **Real-time Monitoring:** Live sensor data updates via BLE
- **Visual Dashboard:** Color-coded air quality indicators
- **Historical Charts:** Track trends over time (1H to 30D)
- **Smart Alerts:** Push notifications for critical air quality levels
- **Recommendations:** Context-aware suggestions for improving air quality
- **Customizable Settings:** Adjust alert thresholds and preferences

## Tech Stack

- **Framework:** React Native with Expo
- **State Management:** Zustand
- **BLE Communication:** react-native-ble-plx
- **Charts:** react-native-chart-kit
- **Navigation:** Expo Router
- **Storage:** AsyncStorage

## Prerequisites

1. **Node.js** (v18 or later)
2. **npm** or **yarn**
3. **Expo CLI:**
   ```bash
   npm install -g expo-cli
   ```
4. **Expo Go app** on your phone (for testing)
   - [iOS App Store](https://apps.apple.com/app/expo-go/id982107779)
   - [Google Play Store](https://play.google.com/store/apps/details?id=host.exp.exponent)

## Installation

1. Install dependencies:
   ```bash
   cd app
   npm install
   ```

2. Start the development server:
   ```bash
   npm start
   ```

3. Scan the QR code with:
   - **iOS:** Camera app
   - **Android:** Expo Go app

## Project Structure

```
app/
├── app/                    # Expo Router screens
│   ├── (tabs)/            # Bottom tab navigation
│   │   ├── index.tsx      # Home/Dashboard screen
│   │   ├── history.tsx    # Historical data charts
│   │   ├── recommendations.tsx  # Tips and alerts
│   │   └── settings.tsx   # App settings
│   └── connection.tsx     # BLE device connection
├── components/            # Reusable UI components
│   ├── SensorCard.tsx     # Individual sensor display
│   ├── OverallScore.tsx   # Air quality score widget
│   ├── MiniChart.tsx      # Dashboard mini chart
│   └── AlertCard.tsx      # Alert notification card
├── store/                 # Zustand state management
│   └── useSensorStore.ts  # Global app state
├── utils/                 # Utility functions
│   ├── bleManager.ts      # BLE communication logic
│   └── notifications.ts   # Push notification handlers
├── constants/             # App constants
│   ├── ble.ts            # BLE UUIDs and config
│   └── thresholds.ts     # Air quality thresholds
└── types/                # TypeScript type definitions
    └── sensor.ts         # Sensor data types
```

## BLE Communication

The app connects to the AirSense device using Bluetooth Low Energy (BLE).

### Service UUIDs

- **Environmental Sensing Service:** `0000181A-0000-1000-8000-00805f9b34fb`
  - CO₂: `00002BD2-0000-1000-8000-00805f9b34fb`
  - Temperature: `00002A6E-0000-1000-8000-00805f9b34fb`
  - Humidity: `00002A6F-0000-1000-8000-00805f9b34fb`

- **Custom Service:** `12345678-1234-5678-1234-56789abcdef0`
  - VOC Index: `12345678-1234-5678-1234-56789abcdef1`
  - NOx Index: `12345678-1234-5678-1234-56789abcdef2`

### Data Formats

- **CO₂:** uint16 (ppm)
- **Temperature:** int16 (°C × 100)
- **Humidity:** uint16 (% × 100)
- **VOC/NOx:** uint16 (index 0-500)

## Permissions

### iOS

Add to `Info.plist`:
```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>This app needs Bluetooth to connect to your AirSense device.</string>
```

### Android

Required in `AndroidManifest.xml`:
- `BLUETOOTH`
- `BLUETOOTH_ADMIN`
- `BLUETOOTH_CONNECT`
- `BLUETOOTH_SCAN`
- `ACCESS_FINE_LOCATION`

## Building for Production

### iOS

```bash
npm install -g eas-cli
eas login
eas build --platform ios
```

### Android

```bash
eas build --platform android
```

## Testing Without Hardware

For development without a physical AirSense device:

1. Modify `utils/bleManager.ts` to use mock data
2. Uncomment the mock data generator in `store/useSensorStore.ts`
3. Use the connection simulator in dev mode

## Troubleshooting

### BLE Connection Issues

- Ensure Bluetooth is enabled on your phone
- Check that location services are enabled (Android)
- Keep the AirSense device within 10m range
- Restart the app and try reconnecting

### App Not Updating

- Shake your device to open the developer menu
- Select "Reload" or press `r` in the terminal

### Build Errors

```bash
# Clear cache and reinstall
rm -rf node_modules
npm install
expo start -c
```

## Development Tips

1. **Hot Reload:** Changes appear instantly in Expo Go
2. **Debug Menu:** Shake device or press `Cmd+D` (iOS) / `Cmd+M` (Android)
3. **Logs:** Use `console.log()` - visible in terminal
4. **Redux DevTools:** Install `react-native-debugger` for state inspection

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

MIT

## Support

For issues and questions:
- Create an issue on GitHub
- Email: support@airsense.example.com
