# Building Standalone Apps

This guide explains how to build standalone iOS and Android apps that work anywhere without needing your computer running.

## Overview

Currently, the app works in three modes:
1. **Web Browser** (GitHub Pages) - Works on Android/Desktop with Web Bluetooth
2. **Expo Go** - Development mode requiring computer running
3. **Standalone App** - Native iOS/Android apps (this guide)

## Prerequisites

1. **Expo Account**
   - Create account at https://expo.dev
   - Free for basic usage

2. **EAS CLI**
   ```bash
   npm install -g eas-cli
   ```

3. **For iOS Builds**
   - Apple Developer Account ($99/year)
   - OR use EAS Build free tier for development builds

4. **For Android Builds**
   - No account needed for APK
   - Google Play account ($25 one-time) for Play Store

## Quick Start

### 1. Login to Expo

```bash
cd app
eas login
```

Enter your Expo credentials.

### 2. Configure EAS Build

```bash
eas build:configure
```

This creates `eas.json` with build profiles.

### 3. Build for Android (Recommended First)

**Development Build (Install directly):**
```bash
eas build --platform android --profile development
```

**Production APK:**
```bash
eas build --platform android --profile preview
```

**Google Play Store:**
```bash
eas build --platform android --profile production
```

### 4. Build for iOS

**Development Build (TestFlight):**
```bash
eas build --platform ios --profile development
```

**App Store:**
```bash
eas build --platform ios --profile production
```

## Build Profiles Explained

### Development Build
- Fast build time
- Includes developer tools
- Can be installed via APK/TestFlight
- Perfect for testing

### Preview Build
- Optimized APK for testing
- No developer tools
- Smaller file size
- Good for sharing with testers

### Production Build
- Fully optimized
- Ready for App Store/Play Store
- Smallest file size
- No debugging tools

## Installing Builds

### Android
1. Build completes and provides download URL
2. Download APK to phone
3. Enable "Install from Unknown Sources" in settings
4. Open APK and install

### iOS
1. Build completes and uploads to TestFlight automatically
2. Install TestFlight app from App Store
3. Open invitation link on your iPhone
4. Install app from TestFlight

## Configuration Tips

### Update app.json

Before building, update these fields in `app.json`:

```json
{
  "expo": {
    "name": "AirSense",
    "slug": "airsense-app",
    "version": "1.0.0",
    "ios": {
      "bundleIdentifier": "com.yourname.airsense",
      "buildNumber": "1"
    },
    "android": {
      "package": "com.yourname.airsense",
      "versionCode": 1
    }
  }
}
```

### BLE Permissions

Already configured in `app.json`:
- Android: BLUETOOTH, BLUETOOTH_SCAN, BLUETOOTH_CONNECT, ACCESS_FINE_LOCATION
- iOS: NSBluetoothAlwaysUsageDescription, NSBluetoothPeripheralUsageDescription

## Troubleshooting

### Build Fails with "No credentials"
- Run `eas credentials` to set up signing credentials
- For iOS: Upload Apple Developer credentials
- For Android: Auto-generates signing key

### Build Takes Too Long
- First build takes 15-30 minutes
- Subsequent builds are faster (cached dependencies)
- Use `--profile development` for faster builds

### App Crashes on Launch
- Check logs: `eas build:list` → View logs
- Ensure all native dependencies are compatible
- Test in Expo Go first

## Local Development Builds

If you prefer building locally (faster for testing):

### Android
```bash
npx expo run:android
```
Outputs APK to: `android/app/build/outputs/apk/debug/app-debug.apk`

### iOS (macOS only)
```bash
npx expo run:ios
```
Requires Xcode installed.

## Cost Breakdown

| Service | Cost | Notes |
|---------|------|-------|
| Expo Account | Free | Basic tier includes builds |
| EAS Build Credits | Free tier | 30 builds/month for personal accounts |
| Apple Developer | $99/year | Required for App Store/TestFlight |
| Google Play | $25 one-time | Optional, only for Play Store |

## Next Steps

1. Build development APK for Android testing
2. Share APK with users or install on your phone
3. Build iOS version once Android is tested
4. Submit to App Store/Play Store when ready

## Resources

- [EAS Build Documentation](https://docs.expo.dev/build/introduction/)
- [Expo Application Services](https://expo.dev/eas)
- [iOS App Store Submission](https://docs.expo.dev/submit/ios/)
- [Android Play Store Submission](https://docs.expo.dev/submit/android/)
