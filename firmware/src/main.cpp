/**
 * AirSense Firmware - Phase 1: Basic SCD41 Sensor Reading
 *
 * Hardware:
 * - Seeed XIAO ESP32-C3
 * - Adafruit SCD41 CO2/Temperature/Humidity Sensor
 *
 * I2C Pins:
 * - SDA: GPIO6
 * - SCL: GPIO7
 */

#include <Arduino.h>
#include <Wire.h>
#include <SensirionI2CScd4x.h>

// I2C Configuration
#define I2C_SDA 6
#define I2C_SCL 7

// Sensor object
SensirionI2CScd4x scd4x;

// Timing
unsigned long lastReading = 0;
const unsigned long READING_INTERVAL = 5000; // 5 seconds

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

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  Serial.println("\n=================================");
  Serial.println("AirSense - Phase 1: SCD41 Test");
  Serial.println("=================================\n");

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

  Serial.println("SCD41 initialized successfully!");
  Serial.println("Waiting for first measurement (5 seconds)...\n");

  delay(5000); // Wait for first measurement
}

void loop() {
  unsigned long currentMillis = millis();

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
      Serial.println("Data not ready yet...");
      return;
    }

    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
      Serial.print("Error reading measurement: ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
    } else if (co2 == 0) {
      Serial.println("Invalid sample detected (CO2 = 0), skipping...");
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
      Serial.println("=================================\n");

      // Air quality assessment
      if (co2 < 600) {
        Serial.println("Air Quality: EXCELLENT");
      } else if (co2 < 800) {
        Serial.println("Air Quality: GOOD");
      } else if (co2 < 1000) {
        Serial.println("Air Quality: MODERATE - Consider ventilation");
      } else if (co2 < 1500) {
        Serial.println("Air Quality: POOR - Open windows!");
      } else {
        Serial.println("Air Quality: BAD - Ventilate immediately!");
      }
      Serial.println();
    }
  }
}
