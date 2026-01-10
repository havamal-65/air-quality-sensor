// Type definitions for sensor data and app state

export interface SensorReading {
  co2: number; // ppm
  temperature: number; // Celsius
  humidity: number; // percentage
  voc: number; // Index 0-500
  nox: number; // Index 0-500
  timestamp: number; // Unix timestamp
}

export interface DeviceInfo {
  id: string;
  name: string;
  rssi?: number;
  isConnected: boolean;
  batteryLevel?: number;
}

export interface HistoricalData {
  readings: SensorReading[];
  timeRange: '1H' | '6H' | '24H' | '7D' | '30D';
}

export interface AlertThresholds {
  co2Warning: number;
  co2Critical: number;
  vocWarning: number;
  vocCritical: number;
}

export interface AppSettings {
  enableNotifications: boolean;
  enableVibration: boolean;
  enableSound: boolean;
  quietHoursEnabled: boolean;
  quietHoursStart?: string;
  quietHoursEnd?: string;
  darkMode: boolean;
  showCharts: boolean;
  updateRate: number; // seconds
  thresholds: AlertThresholds;
}

export interface Alert {
  id: string;
  type: 'co2' | 'voc' | 'nox';
  severity: 'warning' | 'critical';
  value: number;
  message: string;
  recommendations: string[];
  timestamp: number;
  duration: number; // milliseconds since alert started
}
