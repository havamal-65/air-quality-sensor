import React, { useState, useEffect } from 'react';
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  FlatList,
  ActivityIndicator,
} from 'react-native';
import { useRouter } from 'expo-router';
import { Device } from 'react-native-ble-plx';
import { useSensorStore } from '../store/useSensorStore';
import { bleService } from '../utils/bleManager';
import { BLE_CONFIG } from '../constants/ble';

export default function ConnectionScreen() {
  const router = useRouter();
  const [isScanning, setIsScanning] = useState(false);
  const [devices, setDevices] = useState<Device[]>([]);
  const [connecting, setConnecting] = useState<string | null>(null);

  const setDevice = useSensorStore((state) => state.setDevice);

  useEffect(() => {
    startScan();
    return () => {
      bleService.stopScan();
    };
  }, []);

  const startScan = async () => {
    setIsScanning(true);
    setDevices([]);

    try {
      const isReady = await bleService.initializeBluetooth();
      if (!isReady) {
        alert('Bluetooth is not enabled. Please enable Bluetooth and try again.');
        setIsScanning(false);
        return;
      }

      await bleService.scanForDevices(
        (device) => {
          setDevices((prev) => {
            const exists = prev.find((d) => d.id === device.id);
            if (exists) return prev;
            return [...prev, device];
          });
        },
        BLE_CONFIG.SCAN_TIMEOUT_MS
      );
    } catch (error) {
      console.error('Scan error:', error);
      alert('Failed to scan for devices');
    } finally {
      setIsScanning(false);
    }
  };

  const handleConnect = async (device: Device) => {
    setConnecting(device.id);

    try {
      await bleService.connectToDevice(device.id);

      setDevice({
        id: device.id,
        name: device.name || 'AirSense',
        rssi: device.rssi || undefined,
        isConnected: true,
        batteryLevel: 95, // TODO: Read from device
      });

      router.replace('/(tabs)');
    } catch (error) {
      console.error('Connection error:', error);
      alert('Failed to connect to device');
      setConnecting(null);
    }
  };

  const getSignalStrength = (rssi?: number | null): string => {
    if (!rssi) return 'Unknown';
    if (rssi > -60) return 'Strong';
    if (rssi > -80) return 'Medium';
    return 'Weak';
  };

  const getSignalIcon = (rssi?: number | null): string => {
    if (!rssi) return '⚪';
    if (rssi > -60) return '🟢';
    if (rssi > -80) return '🟡';
    return '🔴';
  };

  const renderDevice = ({ item }: { item: Device }) => (
    <TouchableOpacity
      style={styles.deviceItem}
      onPress={() => handleConnect(item)}
      disabled={connecting !== null}
    >
      <View style={styles.deviceInfo}>
        <Text style={styles.deviceIcon}>{getSignalIcon(item.rssi)}</Text>
        <View style={styles.deviceText}>
          <Text style={styles.deviceName}>{item.name || 'Unknown Device'}</Text>
          <Text style={styles.deviceDetails}>
            Signal: {getSignalStrength(item.rssi)} • Ready
          </Text>
        </View>
      </View>
      {connecting === item.id ? (
        <ActivityIndicator color="#2196F3" />
      ) : (
        <TouchableOpacity
          style={styles.connectButton}
          onPress={() => handleConnect(item)}
        >
          <Text style={styles.connectButtonText}>CONNECT</Text>
        </TouchableOpacity>
      )}
    </TouchableOpacity>
  );

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.icon}>🔵</Text>
        <Text style={styles.title}>Connect to AirSense Device</Text>
      </View>

      {isScanning && (
        <View style={styles.scanningCard}>
          <ActivityIndicator size="large" color="#2196F3" />
          <Text style={styles.scanningText}>Scanning for devices...</Text>
        </View>
      )}

      <FlatList
        data={devices}
        renderItem={renderDevice}
        keyExtractor={(item) => item.id}
        ListEmptyComponent={
          !isScanning ? (
            <View style={styles.emptyState}>
              <Text style={styles.emptyText}>No devices found</Text>
            </View>
          ) : null
        }
        contentContainerStyle={styles.deviceList}
      />

      <TouchableOpacity
        style={[styles.scanButton, isScanning && styles.scanButtonDisabled]}
        onPress={startScan}
        disabled={isScanning}
      >
        <Text style={styles.scanButtonText}>
          {isScanning ? 'SCANNING...' : 'SCAN AGAIN'}
        </Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
    padding: 16,
  },
  header: {
    alignItems: 'center',
    marginVertical: 24,
  },
  icon: {
    fontSize: 64,
    marginBottom: 16,
  },
  title: {
    color: '#FFFFFF',
    fontSize: 20,
    fontWeight: '600',
  },
  scanningCard: {
    backgroundColor: '#1E1E1E',
    borderRadius: 12,
    padding: 24,
    alignItems: 'center',
    marginBottom: 16,
  },
  scanningText: {
    color: '#B0B0B0',
    fontSize: 16,
    marginTop: 12,
  },
  deviceList: {
    paddingBottom: 16,
  },
  deviceItem: {
    backgroundColor: '#1E1E1E',
    borderRadius: 12,
    padding: 16,
    marginBottom: 12,
    flexDirection: 'row',
    alignItems: 'center',
    justifyContent: 'space-between',
  },
  deviceInfo: {
    flexDirection: 'row',
    alignItems: 'center',
    flex: 1,
  },
  deviceIcon: {
    fontSize: 24,
    marginRight: 12,
  },
  deviceText: {
    flex: 1,
  },
  deviceName: {
    color: '#FFFFFF',
    fontSize: 16,
    fontWeight: '600',
    marginBottom: 4,
  },
  deviceDetails: {
    color: '#B0B0B0',
    fontSize: 14,
  },
  connectButton: {
    backgroundColor: '#2196F3',
    paddingHorizontal: 20,
    paddingVertical: 10,
    borderRadius: 8,
  },
  connectButtonText: {
    color: '#FFFFFF',
    fontWeight: '600',
    fontSize: 14,
  },
  emptyState: {
    alignItems: 'center',
    paddingVertical: 48,
  },
  emptyText: {
    color: '#666666',
    fontSize: 16,
  },
  scanButton: {
    backgroundColor: '#2196F3',
    padding: 16,
    borderRadius: 8,
    alignItems: 'center',
    marginTop: 8,
  },
  scanButtonDisabled: {
    backgroundColor: '#666666',
  },
  scanButtonText: {
    color: '#FFFFFF',
    fontWeight: '600',
    fontSize: 16,
  },
});
