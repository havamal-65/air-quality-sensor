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

  // Calculate Y-axis scale with minimum range of 100
  const minValue = Math.min(...values);
  const maxValue = Math.max(...values);
  const dataRange = maxValue - minValue;

  // Ensure minimum vertical range of 100 units
  const minRange = 100;
  const range = Math.max(dataRange, minRange);

  // Center the data in the range
  const midPoint = (maxValue + minValue) / 2;
  const yAxisMin = Math.max(0, Math.floor(midPoint - range / 2));
  const yAxisMax = Math.ceil(midPoint + range / 2);

  const chartData = {
    labels: labels.filter((_, i) => i % Math.floor(data.length / 4) === 0),
    datasets: [
      {
        data: [yAxisMin, ...values, yAxisMax],
        color: (opacity = 1) => `rgba(33, 150, 243, ${opacity})`,
        strokeWidth: 3,
        withDots: false,
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
        yAxisSuffix=""
        yAxisInterval={1}
        fromZero={false}
        segments={4}
        yAxisLabel=""
        yLabelsOffset={0}
        formatYLabel={(value) => Math.round(Number(value)).toString()}
        chartConfig={{
          backgroundColor: '#1E1E1E',
          backgroundGradientFrom: '#1E1E1E',
          backgroundGradientTo: '#1E1E1E',
          decimalPlaces: metric === 'temperature' ? 1 : 0,
          color: (opacity = 1) => `rgba(33, 150, 243, ${opacity})`,
          labelColor: (opacity = 1) => `rgba(176, 176, 176, ${opacity})`,
          style: {
            borderRadius: 16,
          },
          propsForDots: {
            r: '4',
            strokeWidth: '2',
            stroke: '#2196F3',
            fill: '#2196F3',
          },
          propsForBackgroundLines: {
            strokeDasharray: '',
            stroke: 'rgba(176, 176, 176, 0.1)',
          },
        }}
        style={styles.chart}
        withInnerLines={true}
        withOuterLines={true}
        withVerticalLines={false}
        withHorizontalLines={true}
        withDots={true}
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
