/**
 * AirSense Firmware - Phase 2: SCD41 Sensor + BLE Communication
 *
 * Hardware:
 * - Seeed XIAO ESP32-C3
 * - Adafruit SCD41 CO2/Temperature/Humidity Sensor
 *
 * I2C Pins:
 * - SDA: GPIO6
 * - SCL: GPIO7
 *
 * Features:
 * - Reads CO2, temperature, and humidity from SCD41
 * - Broadcasts data via BLE
 * - Compatible with AirSense mobile app
 */

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2CScd4x.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// I2C Configuration
#define I2C_SDA 6
#define I2C_SCL 7

// BLE UUIDs (must match app constants)
#define DEVICE_NAME "AirSense"

// Environmental Sensing Service (standard)
#define ENVIRONMENTAL_SERVICE_UUID "0000181A-0000-1000-8000-00805f9b34fb"
#define CO2_CHARACTERISTIC_UUID "00002BD2-0000-1000-8000-00805f9b34fb"
#define TEMPERATURE_CHARACTERISTIC_UUID "00002A6E-0000-1000-8000-00805f9b34fb"
#define HUMIDITY_CHARACTERISTIC_UUID "00002A6F-0000-1000-8000-00805f9b34fb"

// Custom service for VOC/NOx (placeholder for Phase 3)
#define CUSTOM_SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define VOC_CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef1"
#define NOX_CHARACTERISTIC_UUID "12345678-1234-5678-1234-56789abcdef2"

// Sensor object
SensirionI2CScd4x scd4x;

// BLE objects
BLEServer *pServer = NULL;
BLECharacteristic *pCO2Characteristic = NULL;
BLECharacteristic *pTemperatureCharacteristic = NULL;
BLECharacteristic *pHumidityCharacteristic = NULL;
BLECharacteristic *pVOCCharacteristic = NULL;
BLECharacteristic *pNOxCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

// Timing
unsigned long lastReading = 0;
const unsigned long READING_INTERVAL = 5000; // 5 seconds

// BLE Server Callbacks
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("📱 Device connected!");
  }

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("📱 Device disconnected");
  }
};

void printUint16Hex(uint16_t value) {
  Serial.print(value < 4096 ? "0" : "");
  Serial.print(value < 256 ? "0" : "");
  Serial.print(value < 16 ? "0" : "");
  Serial.print(value, HEX);
}

void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
  Serial.print("Serial: 0x");
  printUint16Hex(serial0);
  printUint16Hex(serial1);
  printUint16Hex(serial2);
  Serial.println();
}

void initializeBLE() {
  Serial.println("\n📡 Initializing BLE...");

  // Create the BLE Device
  BLEDevice::init(DEVICE_NAME);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create Environmental Sensing Service
  BLEService *pEnvironmentalService = pServer->createService(ENVIRONMENTAL_SERVICE_UUID);

  // CO2 Characteristic (uint16, ppm)
  pCO2Characteristic = pEnvironmentalService->createCharacteristic(
      CO2_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pCO2Characteristic->addDescriptor(new BLE2902());

  // Temperature Characteristic (int16, 0.01°C units)
  pTemperatureCharacteristic = pEnvironmentalService->createCharacteristic(
      TEMPERATURE_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pTemperatureCharacteristic->addDescriptor(new BLE2902());

  // Humidity Characteristic (uint16, 0.01% units)
  pHumidityCharacteristic = pEnvironmentalService->createCharacteristic(
      HUMIDITY_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pHumidityCharacteristic->addDescriptor(new BLE2902());

  // Create Custom Service for VOC/NOx
  BLEService *pCustomService = pServer->createService(CUSTOM_SERVICE_UUID);

  // VOC Characteristic (uint16, index)
  pVOCCharacteristic = pCustomService->createCharacteristic(
      VOC_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pVOCCharacteristic->addDescriptor(new BLE2902());

  // NOx Characteristic (uint16, index)
  pNOxCharacteristic = pCustomService->createCharacteristic(
      NOX_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  pNOxCharacteristic->addDescriptor(new BLE2902());

  // Start services
  pEnvironmentalService->start();
  pCustomService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(ENVIRONMENTAL_SERVICE_UUID);
  pAdvertising->addServiceUUID(CUSTOM_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("✅ BLE initialized and advertising");
  Serial.print("📡 Device name: ");
  Serial.println(DEVICE_NAME);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  Serial.println("\n=================================");
  Serial.println("AirSense - Phase 2: SCD41 + BLE");
  Serial.println("=================================\n");

  // Initialize BLE
  initializeBLE();

  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize SCD41
  scd4x.begin(Wire);

  // Stop potentially previously started measurement
  uint16_t error;
  char errorMessage[256];

  error = scd4x.stopPeriodicMeasurement();
  if (error) {
    Serial.print("Error trying to stop measurement: ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  // Read sensor serial number
  uint16_t serial0;
  uint16_t serial1;
  uint16_t serial2;
  error = scd4x.getSerialNumber(serial0, serial1, serial2);
  if (error) {
    Serial.print("Error getting serial number: ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    printSerialNumber(serial0, serial1, serial2);
  }

  // Start periodic measurement
  error = scd4x.startPeriodicMeasurement();
  if (error) {
    Serial.print("Error starting measurement: ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
    Serial.println("\nFailed to initialize SCD41!");
    Serial.println("Check wiring:");
    Serial.println("  SDA -> GPIO6");
    Serial.println("  SCL -> GPIO7");
    Serial.println("  3.3V -> 3.3V");
    Serial.println("  GND -> GND");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("✅ SCD41 initialized successfully!");
  Serial.println("⏳ Waiting for first measurement (5 seconds)...\n");

  delay(5000); // Wait for first measurement
}

void loop() {
  unsigned long currentMillis = millis();

  // Handle BLE reconnection
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Give the bluetooth stack time to get ready
    pServer->startAdvertising();
    Serial.println("📡 Restarting advertising...");
    oldDeviceConnected = deviceConnected;
  }

  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  // Read and broadcast sensor data
  if (currentMillis - lastReading >= READING_INTERVAL) {
    lastReading = currentMillis;

    uint16_t error;
    char errorMessage[256];

    // Read measurement
    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;
    bool isDataReady = false;

    error = scd4x.getDataReadyFlag(isDataReady);
    if (error) {
      Serial.print("Error checking data ready: ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      return;
    }

    if (!isDataReady) {
      Serial.println("⏳ Data not ready yet...");
      return;
    }

    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
      Serial.print("Error reading measurement: ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
    } else if (co2 == 0) {
      Serial.println("⚠️  Invalid sample detected (CO2 = 0), skipping...");
    } else {
      // Print readings
      Serial.println("=================================");
      Serial.print("CO2: ");
      Serial.print(co2);
      Serial.println(" ppm");

      Serial.print("Temperature: ");
      Serial.print(temperature, 1);
      Serial.println(" °C");

      Serial.print("Humidity: ");
      Serial.print(humidity, 1);
      Serial.println(" %RH");

      // Air quality assessment
      if (co2 < 600) {
        Serial.println("Air Quality: EXCELLENT ✨");
      } else if (co2 < 800) {
        Serial.println("Air Quality: GOOD ✓");
      } else if (co2 < 1000) {
        Serial.println("Air Quality: MODERATE ⚠️  - Consider ventilation");
      } else if (co2 < 1500) {
        Serial.println("Air Quality: POOR ⚠️  - Open windows!");
      } else {
        Serial.println("Air Quality: BAD ❌ - Ventilate immediately!");
      }

      // BLE connection status
      if (deviceConnected) {
        Serial.println("📱 Broadcasting to connected device");
      } else {
        Serial.println("📡 Ready for BLE connection");
      }
      Serial.println("=================================\n");

      // Update BLE characteristics if device is connected
      if (deviceConnected) {
        // CO2 (uint16, little endian)
        uint8_t co2Data[2];
        co2Data[0] = co2 & 0xFF;
        co2Data[1] = (co2 >> 8) & 0xFF;
        pCO2Characteristic->setValue(co2Data, 2);
        pCO2Characteristic->notify();

        // Temperature (int16, 0.01°C units, little endian)
        int16_t tempValue = (int16_t)(temperature * 100);
        uint8_t tempData[2];
        tempData[0] = tempValue & 0xFF;
        tempData[1] = (tempValue >> 8) & 0xFF;
        pTemperatureCharacteristic->setValue(tempData, 2);
        pTemperatureCharacteristic->notify();

        // Humidity (uint16, 0.01% units, little endian)
        uint16_t humValue = (uint16_t)(humidity * 100);
        uint8_t humData[2];
        humData[0] = humValue & 0xFF;
        humData[1] = (humValue >> 8) & 0xFF;
        pHumidityCharacteristic->setValue(humData, 2);
        pHumidityCharacteristic->notify();

        // VOC and NOx placeholders (Phase 3 will add SGP41 sensor)
        // For now, send zero values
        uint8_t zeroData[2] = {0, 0};
        pVOCCharacteristic->setValue(zeroData, 2);
        pVOCCharacteristic->notify();
        pNOxCharacteristic->setValue(zeroData, 2);
        pNOxCharacteristic->notify();
      }
    }
  }
}
