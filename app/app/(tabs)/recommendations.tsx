import React from 'react';
import { View, Text, StyleSheet, ScrollView } from 'react-native';
import { useSensorStore } from '../../store/useSensorStore';
import { AlertCard } from '../../components/AlertCard';

export default function RecommendationsScreen() {
  const activeAlerts = useSensorStore((state) => state.activeAlerts);
  const removeAlert = useSensorStore((state) => state.removeAlert);
  const currentReading = useSensorStore((state) => state.currentReading);

  const getOverallStatus = (): { color: string; label: string } => {
    if (activeAlerts.some((a) => a.severity === 'critical')) {
      return { color: '#F44336', label: '🔴 Poor Air Quality' };
    }
    if (activeAlerts.length > 0) {
      return { color: '#FFC107', label: '🟡 Moderate Air Quality' };
    }
    return { color: '#4CAF50', label: '🟢 Good Air Quality' };
  };

  const status = getOverallStatus();

  return (
    <ScrollView style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>Current Conditions</Text>
        <Text style={[styles.status, { color: status.color }]}>{status.label}</Text>
      </View>

      {activeAlerts.length > 0 ? (
        <View style={styles.alertsSection}>
          <Text style={styles.alertsTitle}>⚠️ Active Alerts ({activeAlerts.length})</Text>
          {activeAlerts.map((alert) => (
            <AlertCard key={alert.id} alert={alert} onDismiss={removeAlert} />
          ))}
        </View>
      ) : (
        <View style={styles.noAlerts}>
          <Text style={styles.noAlertsIcon}>✅</Text>
          <Text style={styles.noAlertsText}>No active alerts</Text>
          <Text style={styles.noAlertsSubtext}>Air quality is within safe levels</Text>
        </View>
      )}

      <View style={styles.tipsSection}>
        <Text style={styles.tipsTitle}>📚 General Tips</Text>

        <View style={styles.tipCard}>
          <Text style={styles.tipTitle}>Best Air Quality Times</Text>
          <Text style={styles.tipText}>
            Outdoor air is typically cleanest between 6 AM - 8 AM before traffic increases
          </Text>
        </View>

        <View style={styles.tipCard}>
          <Text style={styles.tipTitle}>Ventilation Schedule</Text>
          <Text style={styles.tipText}>
            Open windows for 5-10 minutes every hour when CO₂ levels exceed 800 ppm
          </Text>
        </View>

        <View style={styles.tipCard}>
          <Text style={styles.tipTitle}>Meeting Rooms</Text>
          <Text style={styles.tipText}>
            CO₂ levels spike quickly in crowded spaces. Ensure proper ventilation during and
            after meetings
          </Text>
        </View>

        <View style={styles.tipCard}>
          <Text style={styles.tipTitle}>VOC Sources</Text>
          <Text style={styles.tipText}>
            Common sources include cleaning products, air fresheners, new furniture, and
            paint. Store chemicals in sealed containers
          </Text>
        </View>

        <View style={styles.tipCard}>
          <Text style={styles.tipTitle}>Temperature & Humidity</Text>
          <Text style={styles.tipText}>
            Ideal indoor conditions: 20-22°C (68-72°F) and 40-60% humidity for comfort and
            health
          </Text>
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
  header: {
    marginBottom: 24,
  },
  title: {
    color: '#B0B0B0',
    fontSize: 16,
    marginBottom: 8,
  },
  status: {
    fontSize: 24,
    fontWeight: 'bold',
  },
  alertsSection: {
    marginBottom: 24,
  },
  alertsTitle: {
    color: '#FFFFFF',
    fontSize: 18,
    fontWeight: 'bold',
    marginBottom: 16,
  },
  noAlerts: {
    backgroundColor: '#1E1E1E',
    borderRadius: 12,
    padding: 32,
    alignItems: 'center',
    marginBottom: 24,
  },
  noAlertsIcon: {
    fontSize: 48,
    marginBottom: 12,
  },
  noAlertsText: {
    color: '#FFFFFF',
    fontSize: 18,
    fontWeight: '600',
    marginBottom: 4,
  },
  noAlertsSubtext: {
    color: '#B0B0B0',
    fontSize: 14,
  },
  tipsSection: {
    marginBottom: 32,
  },
  tipsTitle: {
    color: '#FFFFFF',
    fontSize: 18,
    fontWeight: 'bold',
    marginBottom: 16,
  },
  tipCard: {
    backgroundColor: '#1E1E1E',
    borderRadius: 12,
    padding: 16,
    marginBottom: 12,
  },
  tipTitle: {
    color: '#FFFFFF',
    fontSize: 16,
    fontWeight: '600',
    marginBottom: 8,
  },
  tipText: {
    color: '#B0B0B0',
    fontSize: 14,
    lineHeight: 20,
  },
});
