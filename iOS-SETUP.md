# iOS Development Setup Guide

This guide covers setting up and building the AirSense app for iOS devices.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Development Environment Setup](#development-environment-setup)
- [Local Development](#local-development)
- [Building for iOS](#building-for-ios)
- [TestFlight Distribution](#testflight-distribution)
- [App Store Submission](#app-store-submission)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Accounts & Memberships
1. **Apple Developer Account** ($99/year)
   - Enroll at: https://developer.apple.com
   - Required for:
     - TestFlight distribution
     - App Store submission
     - Push notifications (APNS)
     - Code signing certificates

2. **Expo Account** (Free)
   - Sign up at: https://expo.dev
   - Required for EAS Build

### Hardware Requirements
- **Mac computer** running macOS 12.0 or later
- **iPhone or iPad** (optional, for physical device testing)
- USB cable for device connection

### Software Requirements
- **Xcode 14.0 or later** (from Mac App Store)
- **Node.js 18+** and npm or yarn
- **Git** for version control

## Development Environment Setup

### 1. Install Xcode
```bash
# Install Xcode from Mac App Store
# After installation, install command line tools:
xcode-select --install
```

### 2. Install Node.js and Package Managers
```bash
# Using Homebrew (recommended)
brew install node

# Verify installation
node --version
npm --version
```

### 3. Install Expo CLI and EAS CLI
```bash
npm install -g expo-cli eas-cli

# Login to Expo
eas login
```

### 4. Clone the Repository
```bash
git clone https://github.com/Havamal-65/air-quality-sensor.git
cd air-quality-sensor/app
```

### 5. Install Dependencies
```bash
npm install
# or
yarn install
```

## Local Development

### Running in iOS Simulator
The iOS Simulator runs on your Mac and emulates an iPhone or iPad.

```bash
# Start the development server and launch iOS simulator
npm run ios

# Or specify a specific simulator
npm run ios:simulator
```

**First-time setup:**
- Xcode will automatically download iOS simulator images
- Choose your preferred device (iPhone 15, iPad Pro, etc.) from Xcode

**Note:** BLE functionality in simulator is limited. For full BLE testing, use a physical device.

### Running on Physical iPhone/iPad

#### Option 1: Using Expo Go (Easiest)
1. Install **Expo Go** from the App Store on your iPhone
2. Start the development server:
   ```bash
   npm start
   ```
3. Scan the QR code with your iPhone camera
4. App opens in Expo Go with full BLE support

#### Option 2: Direct Device Installation
```bash
# Connect iPhone via USB
# Trust the computer when prompted on iPhone

npm run ios:device
```

**If you encounter signing errors:**
1. Open `ios/airsenseapp.xcworkspace` in Xcode
2. Select the project in left sidebar
3. Go to "Signing & Capabilities"
4. Select your Apple ID team
5. Xcode will automatically provision the app

## Building for iOS

### Build Profiles
The app has three build profiles configured in `eas.json`:

1. **Development** - For internal testing with development client
2. **Preview** - For TestFlight internal distribution
3. **Production** - For App Store submission

### Configure Apple Credentials

First time only, configure your Apple Developer credentials:

```bash
cd app
eas credentials
```

Follow the prompts to:
- Select iOS platform
- Choose automatic or manual credential management
- Sign in with your Apple ID (if automatic)

### Development Build

Create a development build for testing on your device:

```bash
npm run build:ios
# or
eas build --platform ios --profile development
```

This build:
- Includes developer tools
- Can be installed via USB or TestFlight
- Supports iOS Simulator

**Installation:**
```bash
# Download the .ipa file from EAS dashboard
# Install using Apple Configurator or TestFlight
```

### Preview Build

Create a preview build for internal beta testing:

```bash
npm run build:ios:preview
# or
eas build --platform ios --profile preview
```

This build:
- Optimized but not for App Store
- Ideal for TestFlight beta testing
- Includes analytics and debugging

### Production Build

Create a production build for App Store submission:

```bash
npm run build:ios:prod
# or
eas build --platform ios --profile production
```

This build:
- Fully optimized for release
- Auto-increments version number
- Ready for App Store review

## TestFlight Distribution

TestFlight allows you to distribute your app to beta testers before App Store release.

### 1. Build for Preview/Production
```bash
npm run build:ios:preview
```

### 2. Submit to TestFlight
```bash
npm run submit:ios
# or
eas submit --platform ios
```

You'll be prompted for:
- Apple ID (Apple Developer account email)
- App-specific password (generate at appleid.apple.com)

### 3. Configure TestFlight in App Store Connect

1. Go to https://appstoreconnect.apple.com
2. Select your app
3. Go to **TestFlight** tab
4. Add internal testers (up to 100 people in your team)
5. Add external testers (up to 10,000 people, requires Apple review)

### 4. Distribute to Testers

**Internal Testers:**
- Receive builds immediately
- No Apple review required
- Ideal for team testing

**External Testers:**
- Requires Apple review (1-2 days)
- Can create public test links
- Collect feedback from users

## App Store Submission

### 1. Prepare App Store Listing

In App Store Connect, provide:
- App name and description
- Screenshots (iPhone 6.7", 6.5", and iPad Pro)
- App icon (1024x1024)
- Privacy policy URL
- Support URL
- App category (Health & Fitness or Utilities)
- Age rating

### 2. Build and Submit
```bash
# Build production version
npm run build:ios:prod

# Submit to App Store
npm run submit:ios
```

### 3. Complete App Store Information

In App Store Connect:
1. Add screenshots for all required device sizes
2. Write compelling app description
3. Add keywords for search optimization
4. Set pricing (Free)
5. Select countries/regions

### 4. Submit for Review

1. Click **Submit for Review**
2. Answer App Privacy questions
3. Answer Export Compliance questions (No encryption beyond HTTPS)
4. Submit

**Review timeline:** Usually 24-48 hours

### 5. After Approval

- App appears on App Store
- Update the GitHub README with App Store link
- Update the iOSBanner component to link to App Store

## Troubleshooting

### Build Fails with "No valid code signing certificates"

**Solution:**
```bash
eas credentials
# Select "Set up new iOS credentials"
# Choose "Automatic" management
```

### "Bluetooth not available" in Simulator

**Cause:** iOS Simulator doesn't support real Bluetooth.

**Solution:** Test BLE functionality on a physical iPhone using Expo Go or a development build.

### App crashes on launch after build

**Check:**
1. Verify bundle identifier matches: `com.havamal65.airsenseapp`
2. Check crash logs in Xcode → Window → Devices and Simulators
3. Ensure all native dependencies are compatible with Expo SDK 54

### "Location permission required for BLE scanning"

**Normal behavior on iOS 13+.** When app requests Bluetooth access, iOS automatically prompts for location permission. This is required for BLE scanning.

### Build succeeds but TestFlight submission fails

**Common causes:**
1. Missing privacy policy URL
2. Incorrect export compliance answers
3. App icon issues (must be exactly 1024x1024, no transparency)

**Solution:** Check email from Apple for specific rejection reason.

### Can't install development build on device

**Check:**
1. Device is registered in Apple Developer portal
2. Provisioning profile includes your device UDID
3. Trust the developer certificate: Settings → General → Device Management

## Additional Resources

- [Expo Documentation](https://docs.expo.dev/)
- [EAS Build Documentation](https://docs.expo.dev/build/introduction/)
- [Apple Developer Documentation](https://developer.apple.com/documentation/)
- [App Store Review Guidelines](https://developer.apple.com/app-store/review/guidelines/)
- [TestFlight Beta Testing](https://developer.apple.com/testflight/)

## Getting Help

If you encounter issues:

1. Check [Expo Forums](https://forums.expo.dev/)
2. Search [Stack Overflow](https://stackoverflow.com/questions/tagged/expo)
3. Open an issue on [GitHub](https://github.com/Havamal-65/air-quality-sensor/issues)
4. Join the [Expo Discord](https://discord.gg/expo)

---

**Next Steps:**
- Review the main [README.md](README.md) for app functionality
- Check [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines
- Start building: `npm run build:ios`
