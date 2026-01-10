import React from 'react';
import { View, Text, StyleSheet, TouchableOpacity } from 'react-native';
import { Alert } from '../types/sensor';

interface AlertCardProps {
  alert: Alert;
  onDismiss: (alertId: string) => void;
}

export const AlertCard: React.FC<AlertCardProps> = ({ alert, onDismiss }) => {
  const getSeverityColor = (severity: 'warning' | 'critical'): string => {
    return severity === 'critical' ? '#F44336' : '#FF9800';
  };

  const getSeverityIcon = (severity: 'warning' | 'critical'): string => {
    return severity === 'critical' ? '🔴' : '🟠';
  };

  const formatDuration = (ms: number): string => {
    const minutes = Math.floor(ms / 60000);
    if (minutes < 1) return 'Just now';
    if (minutes === 1) return '1 minute';
    return `${minutes} minutes`;
  };

  return (
    <View style={[styles.container, { borderLeftColor: getSeverityColor(alert.severity) }]}>
      <View style={styles.header}>
        <Text style={styles.icon}>{getSeverityIcon(alert.severity)}</Text>
        <View style={styles.headerText}>
          <Text style={styles.message}>{alert.message}</Text>
          <Text style={styles.duration}>Duration: {formatDuration(alert.duration)}</Text>
        </View>
        <TouchableOpacity onPress={() => onDismiss(alert.id)} style={styles.dismissButton}>
          <Text style={styles.dismissText}>✕</Text>
        </TouchableOpacity>
      </View>

      <View style={styles.recommendations}>
        <Text style={styles.recommendationsTitle}>💡 Recommended Actions:</Text>
        {alert.recommendations.map((rec, index) => (
          <Text key={index} style={styles.recommendation}>
            • {rec}
          </Text>
        ))}
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    backgroundColor: '#1E1E1E',
    borderRadius: 12,
    padding: 16,
    marginBottom: 12,
    borderLeftWidth: 4,
  },
  header: {
    flexDirection: 'row',
    alignItems: 'flex-start',
    marginBottom: 12,
  },
  icon: {
    fontSize: 24,
    marginRight: 12,
  },
  headerText: {
    flex: 1,
  },
  message: {
    color: '#FFFFFF',
    fontSize: 16,
    fontWeight: '600',
    marginBottom: 4,
  },
  duration: {
    color: '#B0B0B0',
    fontSize: 12,
  },
  dismissButton: {
    padding: 4,
  },
  dismissText: {
    color: '#B0B0B0',
    fontSize: 20,
  },
  recommendations: {
    marginTop: 8,
  },
  recommendationsTitle: {
    color: '#FFFFFF',
    fontSize: 14,
    fontWeight: '600',
    marginBottom: 8,
  },
  recommendation: {
    color: '#B0B0B0',
    fontSize: 14,
    marginLeft: 8,
    marginBottom: 4,
  },
});
