import React from 'react';
import {
  View,
  Text,
  StyleSheet,
  ScrollView,
  Switch,
  TextInput,
  TouchableOpacity,
} from 'react-native';
import { useSensorStore } from '../../store/useSensorStore';
import { bleService } from '../../utils/bleManager';

export default function SettingsScreen() {
  const device = useSensorStore((state) => state.device);
  const settings = useSensorStore((state) => state.settings);
  const updateSettings = useSensorStore((state) => state.updateSettings);
  const setDevice = useSensorStore((state) => state.setDevice);

  const handleDisconnect = async () => {
    await bleService.disconnect();
    setDevice(null);
  };

  return (
    <ScrollView style={styles.container}>
      {device && (
        <View style={styles.section}>
          <Text style={styles.sectionTitle}>Device</Text>
          <View style={styles.deviceCard}>
            <View style={styles.deviceInfo}>
              <Text style={styles.deviceName}>
                {device.isConnected ? '🔵' : '⚪'} {device.name}
              </Text>
              {device.batteryLevel && (
                <Text style={styles.deviceDetail}>Battery: {device.batteryLevel}%</Text>
              )}
              <Text style={styles.deviceDetail}>Firmware: 1.0.0</Text>
            </View>
            {device.isConnected && (
              <TouchableOpacity style={styles.disconnectButton} onPress={handleDisconnect}>
                <Text style={styles.disconnectButtonText}>DISCONNECT</Text>
              </TouchableOpacity>
            )}
          </View>
        </View>
      )}

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Alert Thresholds</Text>
        <View style={styles.card}>
          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>CO₂ Warning (ppm)</Text>
            <TextInput
              style={styles.input}
              value={settings.thresholds.co2Warning.toString()}
              keyboardType="numeric"
              onChangeText={(value) =>
                updateSettings({
                  thresholds: { ...settings.thresholds, co2Warning: parseInt(value) || 1000 },
                })
              }
            />
          </View>

          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>CO₂ Critical (ppm)</Text>
            <TextInput
              style={styles.input}
              value={settings.thresholds.co2Critical.toString()}
              keyboardType="numeric"
              onChangeText={(value) =>
                updateSettings({
                  thresholds: { ...settings.thresholds, co2Critical: parseInt(value) || 1500 },
                })
              }
            />
          </View>

          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>VOC Warning (Index)</Text>
            <TextInput
              style={styles.input}
              value={settings.thresholds.vocWarning.toString()}
              keyboardType="numeric"
              onChangeText={(value) =>
                updateSettings({
                  thresholds: { ...settings.thresholds, vocWarning: parseInt(value) || 250 },
                })
              }
            />
          </View>

          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>VOC Critical (Index)</Text>
            <TextInput
              style={styles.input}
              value={settings.thresholds.vocCritical.toString()}
              keyboardType="numeric"
              onChangeText={(value) =>
                updateSettings({
                  thresholds: { ...settings.thresholds, vocCritical: parseInt(value) || 400 },
                })
              }
            />
          </View>
        </View>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Notifications</Text>
        <View style={styles.card}>
          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>🔔 Enable Push Notifications</Text>
            <Switch
              value={settings.enableNotifications}
              onValueChange={(value) => updateSettings({ enableNotifications: value })}
              trackColor={{ false: '#666666', true: '#2196F3' }}
            />
          </View>

          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>📳 Vibration</Text>
            <Switch
              value={settings.enableVibration}
              onValueChange={(value) => updateSettings({ enableVibration: value })}
              trackColor={{ false: '#666666', true: '#2196F3' }}
            />
          </View>

          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>🔊 Sound</Text>
            <Switch
              value={settings.enableSound}
              onValueChange={(value) => updateSettings({ enableSound: value })}
              trackColor={{ false: '#666666', true: '#2196F3' }}
            />
          </View>

          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>⏰ Quiet Hours</Text>
            <Switch
              value={settings.quietHoursEnabled}
              onValueChange={(value) => updateSettings({ quietHoursEnabled: value })}
              trackColor={{ false: '#666666', true: '#2196F3' }}
            />
          </View>
        </View>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Display</Text>
        <View style={styles.card}>
          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>🌙 Dark Mode</Text>
            <Switch
              value={settings.darkMode}
              onValueChange={(value) => updateSettings({ darkMode: value })}
              trackColor={{ false: '#666666', true: '#2196F3' }}
            />
          </View>

          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>📊 Show Charts</Text>
            <Switch
              value={settings.showCharts}
              onValueChange={(value) => updateSettings({ showCharts: value })}
              trackColor={{ false: '#666666', true: '#2196F3' }}
            />
          </View>

          <View style={styles.settingRow}>
            <Text style={styles.settingLabel}>🔄 Update Rate (seconds)</Text>
            <TextInput
              style={styles.input}
              value={settings.updateRate.toString()}
              keyboardType="numeric"
              onChangeText={(value) =>
                updateSettings({ updateRate: parseInt(value) || 5 })
              }
            />
          </View>
        </View>
      </View>

      <View style={styles.section}>
        <Text style={styles.sectionTitle}>About</Text>
        <View style={styles.card}>
          <Text style={styles.aboutText}>Version 1.0.0</Text>
          <TouchableOpacity style={styles.linkButton}>
            <Text style={styles.linkButtonText}>Privacy Policy</Text>
          </TouchableOpacity>
          <TouchableOpacity style={styles.linkButton}>
            <Text style={styles.linkButtonText}>Support</Text>
          </TouchableOpacity>
        </View>
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
  section: {
    marginBottom: 24,
  },
  sectionTitle: {
    color: '#B0B0B0',
    fontSize: 14,
    fontWeight: '600',
    marginBottom: 8,
    textTransform: 'uppercase',
  },
  card: {
    backgroundColor: '#1E1E1E',
    borderRadius: 12,
    padding: 16,
  },
  deviceCard: {
    backgroundColor: '#1E1E1E',
    borderRadius: 12,
    padding: 16,
  },
  deviceInfo: {
    marginBottom: 16,
  },
  deviceName: {
    color: '#FFFFFF',
    fontSize: 18,
    fontWeight: '600',
    marginBottom: 8,
  },
  deviceDetail: {
    color: '#B0B0B0',
    fontSize: 14,
    marginBottom: 4,
  },
  disconnectButton: {
    backgroundColor: '#F44336',
    padding: 12,
    borderRadius: 8,
    alignItems: 'center',
  },
  disconnectButtonText: {
    color: '#FFFFFF',
    fontWeight: '600',
  },
  settingRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingVertical: 12,
    borderBottomWidth: 1,
    borderBottomColor: '#2E2E2E',
  },
  settingLabel: {
    color: '#FFFFFF',
    fontSize: 16,
    flex: 1,
  },
  input: {
    backgroundColor: '#2E2E2E',
    color: '#FFFFFF',
    padding: 8,
    borderRadius: 6,
    width: 80,
    textAlign: 'center',
  },
  aboutText: {
    color: '#B0B0B0',
    fontSize: 14,
    marginBottom: 12,
  },
  linkButton: {
    paddingVertical: 8,
  },
  linkButtonText: {
    color: '#2196F3',
    fontSize: 16,
  },
});
