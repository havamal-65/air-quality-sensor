import React from 'react';
import { View, Text, StyleSheet } from 'react-native';
import { calculateOverallScore } from '../constants/thresholds';

interface OverallScoreProps {
  co2: number;
  voc: number;
  nox: number;
}

export const OverallScore: React.FC<OverallScoreProps> = ({ co2, voc, nox }) => {
  const score = calculateOverallScore(co2, voc, nox);

  const getScoreColor = (score: number): string => {
    if (score >= 80) return '#4CAF50';
    if (score >= 60) return '#FFC107';
    if (score >= 40) return '#FF9800';
    return '#F44336';
  };

  const getScoreLabel = (score: number): string => {
    if (score >= 80) return 'EXCELLENT';
    if (score >= 60) return 'GOOD';
    if (score >= 40) return 'MODERATE';
    return 'POOR';
  };

  const color = getScoreColor(score);
  const label = getScoreLabel(score);

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Overall Air Quality</Text>
      <View style={[styles.scoreCircle, { borderColor: color }]}>
        <Text style={[styles.scoreLabel, { color }]}>{label}</Text>
        <Text style={styles.scoreValue}>{score}/100</Text>
      </View>
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    backgroundColor: '#1E1E1E',
    borderRadius: 16,
    padding: 20,
    alignItems: 'center',
    marginBottom: 16,
  },
  title: {
    color: '#B0B0B0',
    fontSize: 16,
    fontWeight: '600',
    marginBottom: 16,
  },
  scoreCircle: {
    width: 120,
    height: 120,
    borderRadius: 60,
    borderWidth: 8,
    justifyContent: 'center',
    alignItems: 'center',
  },
  scoreLabel: {
    fontSize: 18,
    fontWeight: 'bold',
    marginBottom: 4,
  },
  scoreValue: {
    color: '#FFFFFF',
    fontSize: 24,
    fontWeight: 'bold',
  },
});
