import React from 'react';
import { View, Text, StyleSheet, Dimensions } from 'react-native';
import { LineChart } from 'react-native-chart-kit';
import { SensorReading } from '../types/sensor';
import { format } from 'date-fns';

interface MiniChartProps {
  data: SensorReading[];
  metric: 'co2' | 'voc' | 'nox' | 'temperature';
}

export const MiniChart: React.FC<MiniChartProps> = ({ data, metric }) => {
  const screenWidth = Dimensions.get('window').width - 32;

  if (data.length < 2) {
    return (
      <View style={styles.container}>
        <Text style={styles.title}>Last 1 Hour Trend</Text>
        <View style={styles.noData}>
          <Text style={styles.noDataText}>Collecting data...</Text>
        </View>
      </View>
    );
  }

  const values = data.map((reading) => reading[metric] || 0);
  const labels = data.map((reading) => format(reading.timestamp, 'HH:mm'));

  const chartData = {
    labels: labels.filter((_, i) => i % Math.floor(data.length / 4) === 0),
    datasets: [
      {
        data: values,
        color: (opacity = 1) => `rgba(33, 150, 243, ${opacity})`,
        strokeWidth: 2,
      },
    ],
  };

  const metricLabels = {
    co2: 'CO₂ (ppm)',
    voc: 'VOC Index',
    nox: 'NOx Index',
    temperature: 'Temperature (°C)',
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Last 1 Hour Trend - {metricLabels[metric]}</Text>
      <LineChart
        data={chartData}
        width={screenWidth}
        height={180}
        chartConfig={{
          backgroundColor: '#1E1E1E',
          backgroundGradientFrom: '#1E1E1E',
          backgroundGradientTo: '#1E1E1E',
          decimalPlaces: 0,
          color: (opacity = 1) => `rgba(33, 150, 243, ${opacity})`,
          labelColor: (opacity = 1) => `rgba(176, 176, 176, ${opacity})`,
          style: {
            borderRadius: 16,
          },
          propsForDots: {
            r: '2',
            strokeWidth: '2',
            stroke: '#2196F3',
          },
        }}
        bezier
        style={styles.chart}
        withInnerLines={false}
        withOuterLines={true}
        withVerticalLines={false}
      />
    </View>
  );
};

const styles = StyleSheet.create({
  container: {
    backgroundColor: '#1E1E1E',
    borderRadius: 16,
    padding: 16,
    marginTop: 16,
  },
  title: {
    color: '#B0B0B0',
    fontSize: 14,
    fontWeight: '600',
    marginBottom: 12,
  },
  chart: {
    marginVertical: 8,
    borderRadius: 16,
  },
  noData: {
    height: 180,
    justifyContent: 'center',
    alignItems: 'center',
  },
  noDataText: {
    color: '#666666',
    fontSize: 14,
  },
});
