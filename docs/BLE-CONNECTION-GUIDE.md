# How the App Connects to the Sensor

## Connection Flow Overview

```
┌─────────────┐                                    ┌─────────────┐
│   Mobile    │                                    │  ESP32-C3   │
│     App     │         Bluetooth Low Energy       │   Sensor    │
│  (iPhone)   │◄──────────────────────────────────►│  (AirSense) │
└─────────────┘                                    └─────────────┘
```

## Step-by-Step Process

### 1. ESP32 Starts Advertising

**On device power-up:**

```cpp
// firmware/src/main.cpp - setup()

BLEDevice::init("AirSense");              // Create BLE device
pServer = BLEDevice::createServer();      // Create server
pServer->setCallbacks(new MyServerCallbacks());

// Create services and characteristics
BLEService *pEnvironmentalService = pServer->createService(ENVIRONMENTAL_SERVICE_UUID);
pCO2Characteristic = pEnvironmentalService->createCharacteristic(...);
// ... (temperature, humidity, VOC, NOx characteristics)

BLEDevice::startAdvertising();            // Start broadcasting "I'm here!"
```

**What's happening:**
- ESP32 broadcasts BLE advertisements containing:
  - Device name: "AirSense"
  - Service UUIDs it offers
  - Signal strength (RSSI)
- Broadcasts continuously every ~100ms
- Visible to any device scanning for BLE

**Serial output shows:**
```
📡 Initializing BLE...
✅ BLE initialized and advertising
📡 Device name: AirSense
📡 Ready for BLE connection
```

---

### 2. App Scans for Devices

**User taps "Connect Device" in app:**

```typescript
// app/app/connection.tsx - startScan()

const isReady = await bleService.initializeBluetooth();  // Check BT is on
if (!isReady) {
  alert('Bluetooth is not enabled');
  return;
}

await bleService.scanForDevices(
  (device) => {
    // Callback fires for each device found
    if (device.name.includes('AirSense')) {
      setDevices([...prev, device]);  // Add to list
    }
  },
  10000  // Scan for 10 seconds
);
```

**What's happening:**
- App activates iPhone's BLE radio
- Listens for BLE advertisements
- Filters for devices with "AirSense" in name
- Displays found devices with signal strength

**App screen shows:**
```
🔵 Connect to AirSense Device

[🟢 AirSense]
Signal: Strong • Ready
                    [CONNECT]
```

---

### 3. User Connects to Device

**User taps "CONNECT" button:**

```typescript
// app/app/connection.tsx - handleConnect()

await bleService.connectToDevice(device.id);

// BLE Manager does:
const device = await this.manager.connectToDevice(deviceId);
await device.discoverAllServicesAndCharacteristics();
```

**What's happening:**
- App sends connection request to ESP32
- ESP32 accepts connection
- Callback fires on ESP32:

```cpp
void onConnect(BLEServer *pServer) {
  deviceConnected = true;
  Serial.println("📱 Device connected!");
}
```

- App discovers available services and characteristics
- Stores device in app state
- Navigates to dashboard

---

### 4. App Subscribes to Data

**Dashboard loads after connection:**

```typescript
// app/app/(tabs)/index.tsx - useEffect

useEffect(() => {
  if (device?.isConnected) {
    bleService.subscribeToSensorData(updateReading);
  }
}, [device?.isConnected]);
```

**BLE Manager subscribes to each characteristic:**

```typescript
// app/utils/bleManager.ts - subscribeToSensorData()

// Subscribe to CO2
await this.device.monitorCharacteristicForService(
  ENVIRONMENTAL_SERVICE_UUID,
  CO2_CHARACTERISTIC_UUID,
  (error, characteristic) => {
    if (!error && characteristic?.value) {
      const co2 = parseCO2(characteristic);  // Convert binary to number
      onDataReceived({ co2, ... });
    }
  }
);

// Subscribe to Temperature
await this.device.monitorCharacteristicForService(
  ENVIRONMENTAL_SERVICE_UUID,
  TEMPERATURE_CHARACTERISTIC_UUID,
  (error, characteristic) => {
    const temp = parseTemperature(characteristic);
    onDataReceived({ temperature: temp, ... });
  }
);

// ... (humidity, VOC, NOx)
```

**What's happening:**
- App registers for notifications on 5 characteristics
- ESP32 now knows to send updates when values change
- App waits for notifications

---

### 5. ESP32 Broadcasts Sensor Data

**Every 5 seconds in loop():**

```cpp
// firmware/src/main.cpp - loop()

// Read sensor
error = scd4x.readMeasurement(co2, temperature, humidity);

if (deviceConnected) {
  // Update CO2 characteristic
  uint8_t co2Data[2];
  co2Data[0] = co2 & 0xFF;           // Low byte
  co2Data[1] = (co2 >> 8) & 0xFF;    // High byte
  pCO2Characteristic->setValue(co2Data, 2);
  pCO2Characteristic->notify();      // Send to app!

  // Update Temperature characteristic
  int16_t tempValue = (int16_t)(temperature * 100);
  uint8_t tempData[2];
  tempData[0] = tempValue & 0xFF;
  tempData[1] = (tempValue >> 8) & 0xFF;
  pTemperatureCharacteristic->setValue(tempData, 2);
  pTemperatureCharacteristic->notify();

  // ... (humidity, VOC, NOx)
}
```

**What's happening:**
- Reads CO2 sensor via I2C
- Converts values to binary (little-endian format)
- Updates BLE characteristic values
- `.notify()` triggers BLE notification to app
- App receives notification immediately

**Serial output shows:**
```
=================================
CO2: 650 ppm
Temperature: 22.5 °C
Humidity: 48.2 %RH
Air Quality: GOOD ✓
📱 Broadcasting to connected device
=================================
```

---

### 6. App Receives and Displays Data

**When notification arrives:**

```typescript
// app/utils/bleManager.ts - characteristic callback

private parseCO2(characteristic: any): number {
  const buffer = Buffer.from(characteristic.value, 'base64');
  return buffer.readUInt16LE(0);  // Read little-endian uint16
}

private parseTemperature(characteristic: any): number {
  const buffer = Buffer.from(characteristic.value, 'base64');
  return buffer.readInt16LE(0) / 100;  // Convert back from scaled value
}
```

**Data flows to store:**

```typescript
// app/store/useSensorStore.ts

updateReading: (reading) => {
  set((state) => ({
    currentReading: reading,
    historicalData: [...state.historicalData, reading].slice(-1000),
  }));
  // Auto-generate alerts based on thresholds
  checkAlerts(reading);
}
```

**UI automatically updates:**
- Dashboard shows current values
- Charts update with new data point
- Alerts trigger if thresholds exceeded
- History saved to AsyncStorage

---

## Data Format Explained

### Binary to Human-Readable Conversion

**CO2 Example:**
```
ESP32 sends:  [0x8A, 0x02]  (2 bytes, little-endian)
App reads:    0x028A = 650 decimal
Display:      "650 ppm"
```

**Temperature Example:**
```
Sensor:       22.5°C
ESP32 scales: 22.5 × 100 = 2250
ESP32 sends:  [0xCA, 0x08]  (little-endian)
App reads:    0x08CA = 2250
App converts: 2250 ÷ 100 = 22.5
Display:      "22.5°C"
```

**Why scale temperature/humidity?**
- BLE characteristics use integers, not floats
- Multiply by 100 preserves 2 decimal places
- Standard practice in Bluetooth specifications

---

## BLE Services and Characteristics

### Environmental Sensing Service
**UUID:** `0000181A-0000-1000-8000-00805f9b34fb` (standard)

| Characteristic | UUID | Type | Format | Example |
|---------------|------|------|--------|---------|
| CO2 | `00002BD2-...` | uint16 | ppm | 650 |
| Temperature | `00002A6E-...` | int16 | ×100 °C | 2250 → 22.5°C |
| Humidity | `00002A6F-...` | uint16 | ×100 %RH | 4820 → 48.2% |

### Custom Service
**UUID:** `12345678-1234-5678-1234-56789abcdef0`

| Characteristic | UUID | Type | Format | Example |
|---------------|------|------|--------|---------|
| VOC Index | `...-def1` | uint16 | index | 120 |
| NOx Index | `...-def2` | uint16 | index | 85 |

---

## Connection States

```
┌──────────────┐
│ Disconnected │
└──────┬───────┘
       │ User taps "Connect Device"
       ▼
┌──────────────┐
│   Scanning   │ ← Searching for "AirSense"
└──────┬───────┘
       │ Device found
       ▼
┌──────────────┐
│  Connecting  │ ← Establishing BLE connection
└──────┬───────┘
       │ Connection successful
       ▼
┌──────────────┐
│  Connected   │ ← Subscribing to characteristics
└──────┬───────┘
       │ Subscriptions active
       ▼
┌──────────────┐
│ Streaming    │ ← Receiving data every 5 seconds
└──────┬───────┘
       │ User disconnects OR out of range
       ▼
┌──────────────┐
│ Disconnected │ ← ESP32 restarts advertising
└──────────────┘
```

---

## Reconnection Behavior

### When Connection Drops

**ESP32 side:**
```cpp
void onDisconnect(BLEServer *pServer) {
  deviceConnected = false;
  Serial.println("📱 Device disconnected");
}

// In loop()
if (!deviceConnected && oldDeviceConnected) {
  delay(500);
  pServer->startAdvertising();
  Serial.println("📡 Restarting advertising...");
}
```

**App side:**
- Connection lost detected automatically
- User must manually reconnect (scan again)
- Could implement auto-reconnect in future

---

## Demo Mode vs Real Hardware

### Demo Mode (Web/No BLE)
```typescript
if (this.demoMode) {
  // Generate fake data
  setInterval(() => {
    onDataReceived({
      co2: 650 + random(-50, 50),
      temperature: 22.5 + random(-1, 1),
      humidity: 48 + random(-2, 2),
      voc: 120,
      nox: 85,
      timestamp: Date.now()
    });
  }, 5000);
}
```

### Real Hardware Mode
```typescript
// Subscribe to actual BLE characteristics
await this.device.monitorCharacteristicForService(...);
```

**How app decides:**
- Web browser → Always demo mode (BLE not available)
- Expo Go → Real BLE mode (native access)

---

## Troubleshooting Connection Issues

### "No devices found"
**Check:**
1. ESP32 powered on (red LED lit)
2. BLE initialized (serial shows "📡 Ready for BLE connection")
3. iPhone Bluetooth ON
4. Using Expo Go app (not web browser)
5. Within 30 feet of device

### "Connection failed"
**Check:**
1. Close BLE scanner apps (can interfere)
2. Restart ESP32 (reset button)
3. Close and reopen Expo Go app
4. Check serial monitor for errors

### "Connected but no data"
**Check:**
1. Serial shows "📱 Device connected!"
2. Sensor readings appearing in serial (CO2 not 0)
3. Check for BLE notification errors in app console

### "App shows demo data"
**Check:**
1. Using web browser? → Must use Expo Go
2. BLE permissions granted?
3. Check app console for "BLE not available" message

---

## Performance Characteristics

### Power Consumption
- ESP32 active: ~80-100mA
- BLE advertising: +10mA
- BLE connected: +5mA
- Total: ~95-115mA @ 5V

### Data Rate
- Update interval: 5 seconds
- Bytes per update: 10 bytes (5 characteristics × 2 bytes)
- Throughput: 2 bytes/second (very low)

### Latency
- BLE notification: <100ms
- Total app update: <200ms
- User sees data almost instantly

### Range
- Optimal: 10-30 feet
- Through walls: 5-15 feet
- Line of sight: up to 100 feet

---

## Next Steps

### Phase 3: Add SGP41 Sensor
Once you add the VOC/NOx sensor:
- VOC and NOx characteristics will send real values
- No app changes needed (already implemented!)
- Just update firmware to read SGP41

### Future Enhancements
- Auto-reconnect on connection loss
- OTA (Over-The-Air) firmware updates
- Battery level characteristic
- Device configuration via BLE
- Multiple device support
