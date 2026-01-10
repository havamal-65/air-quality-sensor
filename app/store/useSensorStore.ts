import { create } from 'zustand';
import AsyncStorage from '@react-native-async-storage/async-storage';
import { SensorReading, DeviceInfo, AppSettings, Alert } from '../types/sensor';

interface SensorStore {
  // Device state
  device: DeviceInfo | null;
  isScanning: boolean;
  availableDevices: DeviceInfo[];

  // Sensor data
  currentReading: SensorReading | null;
  historicalData: SensorReading[];

  // Alerts
  activeAlerts: Alert[];

  // Settings
  settings: AppSettings;

  // Actions
  setDevice: (device: DeviceInfo | null) => void;
  setIsScanning: (isScanning: boolean) => void;
  setAvailableDevices: (devices: DeviceInfo[]) => void;
  updateReading: (reading: SensorReading) => void;
  addAlert: (alert: Alert) => void;
  removeAlert: (alertId: string) => void;
  updateSettings: (settings: Partial<AppSettings>) => void;
  clearHistory: () => void;
  loadSettings: () => Promise<void>;
  saveSettings: () => Promise<void>;
}

const DEFAULT_SETTINGS: AppSettings = {
  enableNotifications: true,
  enableVibration: true,
  enableSound: false,
  quietHoursEnabled: false,
  darkMode: true,
  showCharts: true,
  updateRate: 5,
  thresholds: {
    co2Warning: 1000,
    co2Critical: 1500,
    vocWarning: 250,
    vocCritical: 400,
  },
};

export const useSensorStore = create<SensorStore>((set, get) => ({
  device: null,
  isScanning: false,
  availableDevices: [],
  currentReading: null,
  historicalData: [],
  activeAlerts: [],
  settings: DEFAULT_SETTINGS,

  setDevice: (device) => set({ device }),

  setIsScanning: (isScanning) => set({ isScanning }),

  setAvailableDevices: (availableDevices) => set({ availableDevices }),

  updateReading: (reading) => {
    set((state) => ({
      currentReading: reading,
      historicalData: [...state.historicalData, reading].slice(-1000), // Keep last 1000 readings
    }));

    // Check for alerts
    const { settings } = get();
    const newAlerts: Alert[] = [];

    if (reading.co2 >= settings.thresholds.co2Critical) {
      newAlerts.push({
        id: `co2-critical-${Date.now()}`,
        type: 'co2',
        severity: 'critical',
        value: reading.co2,
        message: `Critical CO₂ level: ${reading.co2} ppm`,
        recommendations: [
          'Leave the area immediately',
          'Open all windows and doors',
          'Turn on ventilation systems',
        ],
        timestamp: reading.timestamp,
        duration: 0,
      });
    } else if (reading.co2 >= settings.thresholds.co2Warning) {
      newAlerts.push({
        id: `co2-warning-${Date.now()}`,
        type: 'co2',
        severity: 'warning',
        value: reading.co2,
        message: `Elevated CO₂ level: ${reading.co2} ppm`,
        recommendations: [
          'Open windows or doors',
          'Turn on ventilation fan',
          'Take a break outside',
        ],
        timestamp: reading.timestamp,
        duration: 0,
      });
    }

    if (reading.voc >= settings.thresholds.vocCritical) {
      newAlerts.push({
        id: `voc-critical-${Date.now()}`,
        type: 'voc',
        severity: 'critical',
        value: reading.voc,
        message: `Critical VOC level: ${reading.voc}`,
        recommendations: [
          'Identify and remove chemical source',
          'Leave the area if possible',
          'Open all windows',
        ],
        timestamp: reading.timestamp,
        duration: 0,
      });
    } else if (reading.voc >= settings.thresholds.vocWarning) {
      newAlerts.push({
        id: `voc-warning-${Date.now()}`,
        type: 'voc',
        severity: 'warning',
        value: reading.voc,
        message: `Elevated VOC level: ${reading.voc}`,
        recommendations: [
          'Check for chemical sources',
          'Remove cleaning products',
          'Increase air circulation',
        ],
        timestamp: reading.timestamp,
        duration: 0,
      });
    }

    if (newAlerts.length > 0) {
      set((state) => ({
        activeAlerts: [...state.activeAlerts, ...newAlerts],
      }));
    }
  },

  addAlert: (alert) =>
    set((state) => ({
      activeAlerts: [...state.activeAlerts, alert],
    })),

  removeAlert: (alertId) =>
    set((state) => ({
      activeAlerts: state.activeAlerts.filter((a) => a.id !== alertId),
    })),

  updateSettings: (newSettings) => {
    set((state) => ({
      settings: { ...state.settings, ...newSettings },
    }));
    get().saveSettings();
  },

  clearHistory: () => set({ historicalData: [] }),

  loadSettings: async () => {
    try {
      const stored = await AsyncStorage.getItem('app-settings');
      if (stored) {
        const settings = JSON.parse(stored);
        set({ settings: { ...DEFAULT_SETTINGS, ...settings } });
      }
    } catch (error) {
      console.error('Failed to load settings:', error);
    }
  },

  saveSettings: async () => {
    try {
      const { settings } = get();
      await AsyncStorage.setItem('app-settings', JSON.stringify(settings));
    } catch (error) {
      console.error('Failed to save settings:', error);
    }
  },
}));
