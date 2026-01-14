import { BLE_CONFIG } from '../constants/ble';
import { SensorReading } from '../types/sensor';
import { WebBluetoothService } from './webBluetooth';

// Platform check
const IS_WEB = typeof navigator !== 'undefined' && navigator.product === 'ReactNative' ? false : true;

// Dynamic import for BLE (only on native)
let BleManager: any;
let Device: any;
let Characteristic: any;

if (!IS_WEB) {
  try {
    const ble = require('@sfourdrinier/react-native-ble-plx');
    BleManager = ble.BleManager;
    Device = ble.Device;
    Characteristic = ble.Characteristic;
  } catch (e) {
    console.warn('BLE not available on this platform');
  }
}

class BLEService {
  private manager: any;
  private device: any = null;
  private isConnecting = false;
  private demoMode = false;
  private demoInterval: any = null;
  private webBluetooth: WebBluetoothService | null = null;
  private useWebBluetooth = false;

  constructor() {
    if (IS_WEB) {
      // Try to use Web Bluetooth on web platforms
      this.webBluetooth = new WebBluetoothService();
      this.useWebBluetooth = this.webBluetooth.isSupported();

      if (!this.useWebBluetooth) {
        console.warn('Web Bluetooth not supported, using demo mode');
        this.demoMode = true;
      } else {
        console.log('Web Bluetooth is supported!');
      }
    } else if (BleManager) {
      try {
        this.manager = new BleManager();
      } catch (e) {
        console.warn('BLE native module not available (Expo Go), using demo mode');
        this.demoMode = true;
      }
    } else {
      console.warn('No BLE support available, using demo mode');
      this.demoMode = true;
    }
  }

  async initializeBluetooth(): Promise<boolean> {
    if (this.demoMode) {
      return true; // Demo mode always ready
    }

    if (this.useWebBluetooth) {
      return await this.webBluetooth!.initializeBluetooth();
    }

    const state = await this.manager.state();
    if (state !== 'PoweredOn') {
      return false;
    }
    return true;
  }

  async scanForDevices(
    onDeviceFound: (device: any) => void,
    timeoutMs: number = BLE_CONFIG.SCAN_TIMEOUT_MS
  ): Promise<void> {
    if (this.demoMode) {
      // Demo mode - simulate finding a device
      setTimeout(() => {
        onDeviceFound({
          id: 'demo-device-001',
          name: 'AirSense Demo',
          rssi: -45,
        });
      }, 1000);
      return Promise.resolve();
    }

    if (this.useWebBluetooth) {
      // Web Bluetooth doesn't have a scan mode
      // User must click a button to trigger device picker
      // This is handled in connectToDevice instead
      return Promise.resolve();
    }

    return new Promise((resolve, reject) => {
      const timeout = setTimeout(() => {
        this.manager.stopDeviceScan();
        resolve();
      }, timeoutMs);

      this.manager.startDeviceScan(
        null,
        { allowDuplicates: false },
        (error: any, device: any) => {
          if (error) {
            clearTimeout(timeout);
            this.manager.stopDeviceScan();
            reject(error);
            return;
          }

          if (device?.name?.includes(BLE_CONFIG.DEVICE_NAME)) {
            onDeviceFound(device);
          }
        }
      );
    });
  }

  stopScan(): void {
    if (!this.demoMode && !this.useWebBluetooth && this.manager) {
      this.manager.stopDeviceScan();
    }
  }

  async connectToDevice(deviceId: string): Promise<any> {
    if (this.isConnecting) {
      throw new Error('Connection already in progress');
    }

    this.isConnecting = true;

    if (this.demoMode) {
      // Demo mode - simulate connection
      await new Promise(resolve => setTimeout(resolve, 1000));
      this.device = {
        id: deviceId,
        name: 'AirSense Demo',
      };
      this.isConnecting = false;
      return this.device;
    }

    if (this.useWebBluetooth) {
      try {
        // For Web Bluetooth, request device (shows browser picker)
        const device = await this.webBluetooth!.requestDevice();
        await this.webBluetooth!.connectToDevice();
        this.device = device;
        this.isConnecting = false;
        return device;
      } catch (error) {
        this.isConnecting = false;
        throw error;
      }
    }

    try {
      const device = await this.manager.connectToDevice(deviceId);
      await device.discoverAllServicesAndCharacteristics();
      this.device = device;
      this.isConnecting = false;
      return device;
    } catch (error) {
      this.isConnecting = false;
      throw error;
    }
  }

  async disconnect(): Promise<void> {
    if (this.demoMode) {
      if (this.demoInterval) {
        clearInterval(this.demoInterval);
        this.demoInterval = null;
      }
      this.device = null;
      return;
    }

    if (this.useWebBluetooth) {
      await this.webBluetooth!.disconnect();
      this.device = null;
      return;
    }

    if (this.device) {
      await this.device.cancelConnection();
      this.device = null;
    }
  }

  async subscribeToSensorData(
    onDataReceived: (reading: SensorReading) => void
  ): Promise<void> {
    if (!this.device) {
      throw new Error('No device connected');
    }

    if (this.demoMode) {
      // Demo mode - generate realistic sensor data
      this.startDemoDataGenerator(onDataReceived);
      return;
    }

    if (this.useWebBluetooth) {
      await this.webBluetooth!.subscribeToSensorData(onDataReceived);
      return;
    }

    const reading: Partial<SensorReading> = {
      timestamp: Date.now(),
    };

    // Subscribe to CO2
    await this.device.monitorCharacteristicForService(
      BLE_CONFIG.ENVIRONMENTAL_SERVICE_UUID,
      BLE_CONFIG.CO2_CHARACTERISTIC_UUID,
      (error: any, characteristic: any) => {
        if (error || !characteristic?.value) return;
        reading.co2 = this.parseCO2(characteristic);
        this.emitIfComplete(reading, onDataReceived);
      }
    );

    // Subscribe to Temperature
    await this.device.monitorCharacteristicForService(
      BLE_CONFIG.ENVIRONMENTAL_SERVICE_UUID,
      BLE_CONFIG.TEMPERATURE_CHARACTERISTIC_UUID,
      (error: any, characteristic: any) => {
        if (error || !characteristic?.value) return;
        reading.temperature = this.parseTemperature(characteristic);
        this.emitIfComplete(reading, onDataReceived);
      }
    );

    // Subscribe to Humidity
    await this.device.monitorCharacteristicForService(
      BLE_CONFIG.ENVIRONMENTAL_SERVICE_UUID,
      BLE_CONFIG.HUMIDITY_CHARACTERISTIC_UUID,
      (error: any, characteristic: any) => {
        if (error || !characteristic?.value) return;
        reading.humidity = this.parseHumidity(characteristic);
        this.emitIfComplete(reading, onDataReceived);
      }
    );

    // Subscribe to VOC
    await this.device.monitorCharacteristicForService(
      BLE_CONFIG.CUSTOM_SERVICE_UUID,
      BLE_CONFIG.VOC_CHARACTERISTIC_UUID,
      (error: any, characteristic: any) => {
        if (error || !characteristic?.value) return;
        reading.voc = this.parseVOC(characteristic);
        this.emitIfComplete(reading, onDataReceived);
      }
    );

    // Subscribe to NOx
    await this.device.monitorCharacteristicForService(
      BLE_CONFIG.CUSTOM_SERVICE_UUID,
      BLE_CONFIG.NOX_CHARACTERISTIC_UUID,
      (error: any, characteristic: any) => {
        if (error || !characteristic?.value) return;
        reading.nox = this.parseNOx(characteristic);
        this.emitIfComplete(reading, onDataReceived);
      }
    );
  }

  private startDemoDataGenerator(onDataReceived: (reading: SensorReading) => void): void {
    let baseCO2 = 650;
    let baseVOC = 120;
    let baseNOx = 85;
    let baseTemp = 22.4;
    let baseHumidity = 48;

    // Send initial reading
    onDataReceived({
      co2: baseCO2,
      voc: baseVOC,
      nox: baseNOx,
      temperature: baseTemp,
      humidity: baseHumidity,
      timestamp: Date.now(),
    });

    // Update every 5 seconds with realistic variations
    this.demoInterval = setInterval(() => {
      // Simulate realistic variations
      baseCO2 += (Math.random() - 0.5) * 100;
      baseCO2 = Math.max(400, Math.min(2000, baseCO2)); // Keep in realistic range

      baseVOC += (Math.random() - 0.5) * 30;
      baseVOC = Math.max(0, Math.min(500, baseVOC));

      baseNOx += (Math.random() - 0.5) * 20;
      baseNOx = Math.max(0, Math.min(500, baseNOx));

      baseTemp += (Math.random() - 0.5) * 0.5;
      baseTemp = Math.max(18, Math.min(28, baseTemp));

      baseHumidity += (Math.random() - 0.5) * 2;
      baseHumidity = Math.max(30, Math.min(70, baseHumidity));

      onDataReceived({
        co2: Math.round(baseCO2),
        voc: Math.round(baseVOC),
        nox: Math.round(baseNOx),
        temperature: Math.round(baseTemp * 10) / 10,
        humidity: Math.round(baseHumidity),
        timestamp: Date.now(),
      });
    }, 5000);
  }

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

  private parseCO2(characteristic: any): number {
    if (!characteristic.value) return 0;
    const buffer = Buffer.from(characteristic.value, 'base64');
    return buffer.readUInt16LE(0);
  }

  private parseTemperature(characteristic: any): number {
    if (!characteristic.value) return 0;
    const buffer = Buffer.from(characteristic.value, 'base64');
    return buffer.readInt16LE(0) / 100;
  }

  private parseHumidity(characteristic: any): number {
    if (!characteristic.value) return 0;
    const buffer = Buffer.from(characteristic.value, 'base64');
    return buffer.readUInt16LE(0) / 100;
  }

  private parseVOC(characteristic: any): number {
    if (!characteristic.value) return 0;
    const buffer = Buffer.from(characteristic.value, 'base64');
    return buffer.readUInt16LE(0);
  }

  private parseNOx(characteristic: any): number {
    if (!characteristic.value) return 0;
    const buffer = Buffer.from(characteristic.value, 'base64');
    return buffer.readUInt16LE(0);
  }

  isDeviceConnected(): boolean {
    if (this.useWebBluetooth) {
      return this.webBluetooth!.isDeviceConnected();
    }
    return this.device !== null;
  }

  getDevice(): any {
    if (this.useWebBluetooth) {
      return this.webBluetooth!.getDevice();
    }
    return this.device;
  }

  isDemoMode(): boolean {
    return this.demoMode;
  }

  isWebBluetoothMode(): boolean {
    return this.useWebBluetooth;
  }

  destroy(): void {
    this.stopScan();
    if (this.demoMode) {
      if (this.demoInterval) {
        clearInterval(this.demoInterval);
      }
      return;
    }
    if (this.useWebBluetooth) {
      this.webBluetooth!.disconnect();
      return;
    }
    if (this.device) {
      this.device.cancelConnection();
    }
    if (this.manager) {
      this.manager.destroy();
    }
  }
}

export const bleService = new BLEService();
