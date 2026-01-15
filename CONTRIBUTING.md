# Contributing to AirSense Air Quality Sensor

Thank you for your interest in contributing to the AirSense project! This document provides guidelines for contributing to both the firmware and mobile app.

## Table of Contents
- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Workflow](#development-workflow)
- [Code Standards](#code-standards)
- [Pull Request Process](#pull-request-process)
- [Reporting Issues](#reporting-issues)

## Code of Conduct

This project follows a simple code of conduct:
- Be respectful and professional
- Provide constructive feedback
- Focus on the code, not the person
- Help create a welcoming environment for all contributors

## Getting Started

### Prerequisites

**For App Development:**
- Node.js 18+ and npm/yarn
- For iOS: Mac with Xcode 14+ (see [iOS-SETUP.md](iOS-SETUP.md))
- For Android: Android Studio
- Expo CLI and EAS CLI installed globally

**For Firmware Development:**
- PlatformIO (VS Code extension or CLI)
- ESP32-C3 development board
- Required sensors (SCD41, SGP41)

### Fork and Clone

1. Fork the repository on GitHub
2. Clone your fork locally:
   ```bash
   git clone https://github.com/YOUR-USERNAME/air-quality-sensor.git
   cd air-quality-sensor
   ```
3. Add the upstream repository:
   ```bash
   git remote add upstream https://github.com/Havamal-65/air-quality-sensor.git
   ```

### Install Dependencies

**Mobile App:**
```bash
cd app
npm install
```

**Firmware:**
```bash
cd firmware
pio lib install
```

## Development Workflow

### 1. Create a Feature Branch

Always create a new branch for your work:
```bash
git checkout -b feature/your-feature-name
# or
git checkout -b fix/issue-description
```

**Branch Naming Conventions:**
- `feature/` - New features or enhancements
- `fix/` - Bug fixes
- `docs/` - Documentation updates
- `refactor/` - Code refactoring
- `test/` - Test additions or updates

### 2. Make Your Changes

- Follow the code standards outlined below
- Write clear, descriptive commit messages
- Test your changes thoroughly
- Update documentation as needed

### 3. Commit Your Changes

Use clear, descriptive commit messages:
```bash
git add .
git commit -m "Add: Brief description of what you added"
```

**Commit Message Format:**
- `Add:` - New features
- `Fix:` - Bug fixes
- `Update:` - Changes to existing features
- `Refactor:` - Code improvements without functionality changes
- `Docs:` - Documentation updates
- `Test:` - Test additions or modifications

**Examples:**
```
Add: iOS banner component for web users
Fix: BLE connection timeout on Android
Update: CO2 threshold values based on WHO guidelines
Refactor: Simplify BLE manager initialization logic
Docs: Add iOS build instructions to README
```

### 4. Keep Your Fork Updated

Regularly sync with the upstream repository:
```bash
git fetch upstream
git checkout main
git merge upstream/main
git push origin main
```

### 5. Push Your Changes

```bash
git push origin feature/your-feature-name
```

## Code Standards

### TypeScript/JavaScript (Mobile App)

**Style Guidelines:**
- Use TypeScript for type safety
- Use functional components with hooks (no class components)
- Follow React Native best practices
- Use meaningful variable and function names
- Add comments for complex logic only

**Code Formatting:**
- Indentation: 2 spaces
- Semicolons: Not required (rely on ASI)
- Quotes: Single quotes for strings
- Line length: Maximum 100 characters

**Example:**
```typescript
import React, { useState, useEffect } from 'react';
import { View, Text } from 'react-native';

interface SensorCardProps {
  value: number;
  unit: string;
  label: string;
}

export default function SensorCard({ value, unit, label }: SensorCardProps) {
  const [color, setColor] = useState('#4CAF50');

  useEffect(() => {
    // Update color based on value thresholds
    if (value > 1000) setColor('#F44336');
    else if (value > 800) setColor('#FF9800');
    else setColor('#4CAF50');
  }, [value]);

  return (
    <View style={{ backgroundColor: color }}>
      <Text>{label}: {value} {unit}</Text>
    </View>
  );
}
```

**Important Rules:**
- Never hardcode sensitive data (API keys, credentials)
- No hardcoded values; use constants or configuration
- Handle errors gracefully with try-catch
- Always clean up resources (BLE connections, timers, etc.)

### C++ (Firmware)

**Style Guidelines:**
- Follow Arduino/PlatformIO conventions
- Use descriptive variable names
- Comment hardware-specific code
- Add function headers for complex functions

**Code Formatting:**
- Indentation: 2 spaces
- Braces: Opening brace on same line
- Constants: UPPER_CASE_WITH_UNDERSCORES

**Example:**
```cpp
#define CO2_THRESHOLD_MODERATE 800

// Read CO2 value from SCD41 sensor and return as ppm
uint16_t readCO2() {
  if (scd41.readMeasurement()) {
    uint16_t co2 = scd41.getCO2();

    // Validate reading is within sensor range
    if (co2 >= 400 && co2 <= 5000) {
      return co2;
    }
  }

  return 0; // Return 0 on error
}
```

### File Organization

**Mobile App Structure:**
```
app/
├── app/              # Screens (Expo Router)
├── components/       # Reusable UI components
├── utils/           # Utility functions (BLE, helpers)
├── store/           # State management (Zustand)
├── constants/       # Configuration and constants
├── types/           # TypeScript type definitions
└── assets/          # Images, icons, fonts
```

**Firmware Structure:**
```
firmware/
├── src/
│   └── main.cpp     # Main firmware code
├── lib/             # Custom libraries
└── platformio.ini   # PlatformIO configuration
```

## Pull Request Process

### Before Submitting

1. **Test thoroughly:**
   - Mobile app: Test on both iOS and Android
   - Firmware: Test on actual hardware
   - Verify no regressions

2. **Update documentation:**
   - Update README if adding features
   - Add code comments for complex logic
   - Update iOS-SETUP.md if changing build process

3. **Clean commit history:**
   - Squash trivial commits
   - Write clear commit messages
   - Rebase on latest main if needed

### Submitting a Pull Request

1. Push your branch to your fork
2. Go to the original repository on GitHub
3. Click "New Pull Request"
4. Select your feature branch
5. Fill out the PR template:

```markdown
## Description
Brief description of what this PR does

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Refactoring

## Testing
Describe how you tested your changes:
- [ ] Tested on iOS
- [ ] Tested on Android
- [ ] Tested on physical hardware
- [ ] Tested on web

## Screenshots (if applicable)
Add screenshots or videos demonstrating the change

## Checklist
- [ ] Code follows project style guidelines
- [ ] No hardcoded values or sensitive data
- [ ] Documentation updated
- [ ] Tested thoroughly
- [ ] No breaking changes (or documented if necessary)
```

### Review Process

- Maintainers will review your PR within 3-5 days
- Address any requested changes
- Once approved, your PR will be merged
- You'll be credited in the release notes

## Reporting Issues

### Bug Reports

When reporting bugs, please include:

1. **Description:** Clear description of the bug
2. **Steps to Reproduce:**
   ```
   1. Open app
   2. Tap "Connect"
   3. Select device
   4. App crashes
   ```
3. **Expected Behavior:** What should happen
4. **Actual Behavior:** What actually happens
5. **Environment:**
   - Platform: iOS 17.1 / Android 14 / Web (Chrome)
   - App Version: 1.0.0
   - Device: iPhone 15 Pro / Pixel 8
6. **Screenshots/Logs:** If available
7. **Additional Context:** Any other relevant information

### Feature Requests

When requesting features, please include:

1. **Problem:** What problem does this solve?
2. **Proposed Solution:** How would you like it to work?
3. **Alternatives:** Any alternative solutions considered?
4. **Use Case:** When would this be useful?

**Template:**
```markdown
## Problem
Users can't export their historical data

## Proposed Solution
Add "Export to CSV" button in History screen

## Alternatives
- Export as JSON
- Email data directly

## Use Case
Long-term tracking and analysis in external tools
```

## Development Tips

### iOS Development

- Always test on physical iPhone for BLE functionality
- iOS Simulator doesn't support real Bluetooth
- See [iOS-SETUP.md](iOS-SETUP.md) for detailed iOS workflow
- Use Expo Go for quick testing during development

### Android Development

- Test on both physical device and emulator
- Check for permission issues (Bluetooth, Location)
- Verify BLE scanning works across Android versions

### BLE Development

- Always clean up connections on unmount
- Handle connection timeouts gracefully
- Test with actual sensor hardware when possible
- Demo mode should simulate realistic data

### Testing Without Hardware

The app includes a demo mode that:
- Simulates realistic sensor data
- Allows UI/UX testing without hardware
- Generates historical data automatically

Enable demo mode by running the app without connecting to a device.

## Questions?

If you have questions about contributing:

1. Check existing issues and PRs
2. Read the documentation (README, iOS-SETUP.md)
3. Open a new issue with the "question" label
4. Join discussions in existing issues

## Recognition

Contributors will be:
- Listed in release notes
- Credited in the repository
- Acknowledged in project documentation

Thank you for contributing to AirSense!
