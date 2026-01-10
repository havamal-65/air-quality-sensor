import React, { useEffect } from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  RefreshControl,
  TouchableOpacity,
} from 'react-native';
import { useRouter } from 'expo-router';
import { useSensorStore } from '../../store/useSensorStore';
import { SensorCard } from '../../components/SensorCard';
import { OverallScore } from '../../components/OverallScore';
import { MiniChart } from '../../components/MiniChart';
import { getCO2Level, getVOCLevel, getNOxLevel } from '../../constants/thresholds';
import { bleService } from '../../utils/bleManager';

export default function HomeScreen() {
  const router = useRouter();
  const [refreshing, setRefreshing] = React.useState(false);

  const device = useSensorStore((state) => state.device);
  const currentReading = useSensorStore((state) => state.currentReading);
  const historicalData = useSensorStore((state) => state.historicalData);
  const updateReading = useSensorStore((state) => state.updateReading);

  useEffect(() => {
    if (device?.isConnected) {
      bleService.subscribeToSensorData(updateReading);
    }
  }, [device?.isConnected]);

  const onRefresh = React.useCallback(() => {
    setRefreshing(true);
    setTimeout(() => {
      setRefreshing(false);
    }, 1000);
  }, []);

  if (!device?.isConnected) {
    return (
      <View style={styles.emptyState}>
        <Text style={styles.emptyIcon}>🔍</Text>
        <Text style={styles.emptyTitle}>No Device Connected</Text>
        <Text style={styles.emptyText}>
          Connect your AirSense device to start monitoring air quality
        </Text>
        <TouchableOpacity
          style={styles.connectButton}
          onPress={() => router.push('/connection')}
        >
          <Text style={styles.connectButtonText}>SCAN FOR DEVICES</Text>
        </TouchableOpacity>
      </View>
    );
  }

  if (!currentReading) {
    return (
      <View style={styles.loadingState}>
        <Text style={styles.loadingIcon}>⏳</Text>
        <Text style={styles.loadingText}>Reading Sensors...</Text>
      </View>
    );
  }

  const co2Status = getCO2Level(currentReading.co2);
  const vocStatus = getVOCLevel(currentReading.voc);
  const noxStatus = getNOxLevel(currentReading.nox);

  const recentData = historicalData.slice(-12);

  return (
    <ScrollView
      style={styles.container}
      refreshControl={<RefreshControl refreshing={refreshing} onRefresh={onRefresh} />}
    >
      <View style={styles.header}>
        <Text style={styles.deviceName}>{device.name}</Text>
        <View style={styles.status}>
          {device.batteryLevel && (
            <Text style={styles.battery}>🔋 {device.batteryLevel}%</Text>
          )}
          <View style={styles.connectionDot} />
        </View>
      </View>

      <OverallScore co2={currentReading.co2} voc={currentReading.voc} nox={currentReading.nox} />

      <View style={styles.sensorGrid}>
        <SensorCard
          title="CO₂"
          value={currentReading.co2}
          unit="ppm"
          status={co2Status.label}
          statusColor={co2Status.color}
        />
        <SensorCard
          title="VOC"
          value={currentReading.voc}
          unit="Index"
          status={vocStatus.label}
          statusColor={vocStatus.color}
        />
      </View>

      <View style={styles.sensorGrid}>
        <SensorCard
          title="NOx"
          value={currentReading.nox}
          unit="Index"
          status={noxStatus.label}
          statusColor={noxStatus.color}
        />
        <SensorCard
          title="TEMP & RH"
          value={`${currentReading.temperature.toFixed(1)}°C  ${currentReading.humidity.toFixed(0)}%`}
          unit=""
          status="💧 Comfort"
          statusColor="#2196F3"
        />
      </View>

      {recentData.length > 1 && <MiniChart data={recentData} metric="co2" />}

      <View style={styles.bottomButtons}>
        <TouchableOpacity
          style={styles.button}
          onPress={() => router.push('/history')}
        >
          <Text style={styles.buttonText}>HISTORY</Text>
        </TouchableOpacity>
        <TouchableOpacity
          style={styles.button}
          onPress={() => router.push('/recommendations')}
        >
          <Text style={styles.buttonText}>RECOMMENDATIONS</Text>
        </TouchableOpacity>
      </View>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
    padding: 16,
  },
  header: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 16,
  },
  deviceName: {
    color: '#FFFFFF',
    fontSize: 18,
    fontWeight: '600',
  },
  status: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  battery: {
    color: '#B0B0B0',
    fontSize: 14,
    marginRight: 8,
  },
  connectionDot: {
    width: 10,
    height: 10,
    borderRadius: 5,
    backgroundColor: '#2196F3',
  },
  sensorGrid: {
    flexDirection: 'row',
    marginBottom: 8,
  },
  bottomButtons: {
    flexDirection: 'row',
    marginTop: 16,
    marginBottom: 32,
  },
  button: {
    flex: 1,
    backgroundColor: '#2196F3',
    padding: 16,
    borderRadius: 8,
    alignItems: 'center',
    marginHorizontal: 4,
  },
  buttonText: {
    color: '#FFFFFF',
    fontWeight: '600',
    fontSize: 14,
  },
  emptyState: {
    flex: 1,
    backgroundColor: '#121212',
    justifyContent: 'center',
    alignItems: 'center',
    padding: 32,
  },
  emptyIcon: {
    fontSize: 64,
    marginBottom: 16,
  },
  emptyTitle: {
    color: '#FFFFFF',
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 8,
  },
  emptyText: {
    color: '#B0B0B0',
    fontSize: 16,
    textAlign: 'center',
    marginBottom: 24,
  },
  connectButton: {
    backgroundColor: '#2196F3',
    paddingHorizontal: 32,
    paddingVertical: 16,
    borderRadius: 8,
  },
  connectButtonText: {
    color: '#FFFFFF',
    fontWeight: '600',
    fontSize: 16,
  },
  loadingState: {
    flex: 1,
    backgroundColor: '#121212',
    justifyContent: 'center',
    alignItems: 'center',
  },
  loadingIcon: {
    fontSize: 48,
    marginBottom: 16,
  },
  loadingText: {
    color: '#B0B0B0',
    fontSize: 18,
  },
});
