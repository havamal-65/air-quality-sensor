/**
 * Web Bluetooth API wrapper for browser-based BLE communication
 * Works on Chrome/Edge on Windows, macOS, Linux, and Android
 * Does NOT work on iOS (any browser)
 */

import { BLE_CONFIG } from '../constants/ble';
import { SensorReading } from '../types/sensor';

export class WebBluetoothService {
  private device: BluetoothDevice | null = null;
  private server: BluetoothRemoteGATTServer | null = null;
  private characteristics: Map<string, BluetoothRemoteGATTCharacteristic> = new Map();

  /**
   * Check if Web Bluetooth is supported in this browser
   */
  isSupported(): boolean {
    return typeof navigator !== 'undefined' && 'bluetooth' in navigator;
  }

  /**
   * Initialize and check Bluetooth availability
   */
  async initializeBluetooth(): Promise<boolean> {
    if (!this.isSupported()) {
      console.warn('Web Bluetooth not supported in this browser');
      return false;
    }
    return true;
  }

  /**
   * Scan and connect to AirSense device
   * Web Bluetooth doesn't have a "scan" mode - user must click to trigger device picker
   */
  async requestDevice(): Promise<any> {
    if (!this.isSupported()) {
      throw new Error('Web Bluetooth not supported');
    }

    try {
      // Request device with specific services
      this.device = await navigator.bluetooth.requestDevice({
        filters: [
          { name: BLE_CONFIG.DEVICE_NAME },
          { namePrefix: 'AirSense' }
        ],
        optionalServices: [
          BLE_CONFIG.ENVIRONMENTAL_SERVICE_UUID,
          BLE_CONFIG.CUSTOM_SERVICE_UUID
        ]
      });

      console.log('Device selected:', this.device.name);

      return {
        id: this.device.id,
        name: this.device.name,
        rssi: null, // Web Bluetooth doesn't provide RSSI
      };
    } catch (error) {
      console.error('Device selection cancelled or failed:', error);
      throw error;
    }
  }

  /**
   * Connect to the selected device
   */
  async connectToDevice(): Promise<void> {
    if (!this.device) {
      throw new Error('No device selected. Call requestDevice() first.');
    }

    try {
      console.log('Connecting to GATT Server...');
      this.server = await this.device.gatt!.connect();
      console.log('Connected to GATT Server');

      // Get Environmental Sensing Service
      console.log('Discovering services...');
      const envService = await this.server.getPrimaryService(
        BLE_CONFIG.ENVIRONMENTAL_SERVICE_UUID
      );

      // Get all characteristics
      const co2Char = await envService.getCharacteristic(
        BLE_CONFIG.CO2_CHARACTERISTIC_UUID
      );
      const tempChar = await envService.getCharacteristic(
        BLE_CONFIG.TEMPERATURE_CHARACTERISTIC_UUID
      );
      const humChar = await envService.getCharacteristic(
        BLE_CONFIG.HUMIDITY_CHARACTERISTIC_UUID
      );

      this.characteristics.set('co2', co2Char);
      this.characteristics.set('temperature', tempChar);
      this.characteristics.set('humidity', humChar);

      // Get Custom Service
      try {
        const customService = await this.server.getPrimaryService(
          BLE_CONFIG.CUSTOM_SERVICE_UUID
        );

        const vocChar = await customService.getCharacteristic(
          BLE_CONFIG.VOC_CHARACTERISTIC_UUID
        );
        const noxChar = await customService.getCharacteristic(
          BLE_CONFIG.NOX_CHARACTERISTIC_UUID
        );

        this.characteristics.set('voc', vocChar);
        this.characteristics.set('nox', noxChar);
      } catch (error) {
        console.warn('Custom service not available:', error);
      }

      console.log('All characteristics discovered');
    } catch (error) {
      console.error('Connection failed:', error);
      throw error;
    }
  }

  /**
   * Subscribe to sensor data notifications
   */
  async subscribeToSensorData(
    onDataReceived: (reading: SensorReading) => void
  ): Promise<void> {
    if (!this.server || !this.server.connected) {
      throw new Error('Not connected to device');
    }

    const reading: Partial<SensorReading> = {
      timestamp: Date.now(),
    };

    // Subscribe to CO2
    const co2Char = this.characteristics.get('co2');
    if (co2Char) {
      await co2Char.startNotifications();
      co2Char.addEventListener('characteristicvaluechanged', (event: any) => {
        const value = event.target.value as DataView;
        reading.co2 = value.getUint16(0, true); // true = little-endian
        this.emitIfComplete(reading, onDataReceived);
      });
    }

    // Subscribe to Temperature
    const tempChar = this.characteristics.get('temperature');
    if (tempChar) {
      await tempChar.startNotifications();
      tempChar.addEventListener('characteristicvaluechanged', (event: any) => {
        const value = event.target.value as DataView;
        reading.temperature = value.getInt16(0, true) / 100; // Scale back
        this.emitIfComplete(reading, onDataReceived);
      });
    }

    // Subscribe to Humidity
    const humChar = this.characteristics.get('humidity');
    if (humChar) {
      await humChar.startNotifications();
      humChar.addEventListener('characteristicvaluechanged', (event: any) => {
        const value = event.target.value as DataView;
        reading.humidity = value.getUint16(0, true) / 100; // Scale back
        this.emitIfComplete(reading, onDataReceived);
      });
    }

    // Subscribe to VOC
    const vocChar = this.characteristics.get('voc');
    if (vocChar) {
      await vocChar.startNotifications();
      vocChar.addEventListener('characteristicvaluechanged', (event: any) => {
        const value = event.target.value as DataView;
        reading.voc = value.getUint16(0, true);
        this.emitIfComplete(reading, onDataReceived);
      });
    }

    // Subscribe to NOx
    const noxChar = this.characteristics.get('nox');
    if (noxChar) {
      await noxChar.startNotifications();
      noxChar.addEventListener('characteristicvaluechanged', (event: any) => {
        const value = event.target.value as DataView;
        reading.nox = value.getUint16(0, true);
        this.emitIfComplete(reading, onDataReceived);
      });
    }

    console.log('Subscribed to all notifications');
  }

  /**
   * Emit complete reading when all values are received
   */
  private emitIfComplete(
    reading: Partial<SensorReading>,
    callback: (reading: SensorReading) => void
  ): void {
    if (
      reading.co2 !== undefined &&
      reading.temperature !== undefined &&
      reading.humidity !== undefined &&
      reading.voc !== undefined &&
      reading.nox !== undefined
    ) {
      callback({
        ...reading,
        timestamp: Date.now(),
      } as SensorReading);
    }
  }

  /**
   * Disconnect from device
   */
  async disconnect(): Promise<void> {
    if (this.server && this.server.connected) {
      this.server.disconnect();
    }
    this.device = null;
    this.server = null;
    this.characteristics.clear();
  }

  /**
   * Check if device is connected
   */
  isDeviceConnected(): boolean {
    return this.server?.connected || false;
  }

  /**
   * Get current device
   */
  getDevice(): any {
    return this.device
      ? {
          id: this.device.id,
          name: this.device.name,
          isConnected: this.server?.connected || false,
        }
      : null;
  }
}
