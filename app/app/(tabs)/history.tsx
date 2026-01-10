import React, { useState } from 'react';
import { View, Text, StyleSheet, ScrollView, TouchableOpacity } from 'react-native';
import { useSensorStore } from '../../store/useSensorStore';
import { LineChart } from 'react-native-chart-kit';
import { Dimensions } from 'react-native';
import { format } from 'date-fns';

type TimeRange = '1H' | '6H' | '24H' | '7D' | '30D';

export default function HistoryScreen() {
  const [selectedRange, setSelectedRange] = useState<TimeRange>('24H');
  const historicalData = useSensorStore((state) => state.historicalData);

  const screenWidth = Dimensions.get('window').width - 32;

  const getFilteredData = () => {
    const now = Date.now();
    const ranges: Record<TimeRange, number> = {
      '1H': 3600000,
      '6H': 21600000,
      '24H': 86400000,
      '7D': 604800000,
      '30D': 2592000000,
    };

    const cutoff = now - ranges[selectedRange];
    return historicalData.filter((reading) => reading.timestamp >= cutoff);
  };

  const filteredData = getFilteredData();

  const calculateStats = (values: number[]) => {
    if (values.length === 0) return { avg: 0, peak: 0, min: 0 };
    return {
      avg: Math.round(values.reduce((a, b) => a + b, 0) / values.length),
      peak: Math.max(...values),
      min: Math.min(...values),
    };
  };

  const renderChart = (
    title: string,
    metric: 'co2' | 'voc' | 'nox' | 'temperature',
    color: string
  ) => {
    if (filteredData.length < 2) {
      return (
        <View style={styles.chartContainer}>
          <Text style={styles.chartTitle}>{title}</Text>
          <View style={styles.noData}>
            <Text style={styles.noDataText}>Not enough data to display</Text>
          </View>
        </View>
      );
    }

    const values = filteredData.map((r) => r[metric]);
    const stats = calculateStats(values);
    const labels = filteredData.map((r) =>
      selectedRange === '7D' || selectedRange === '30D'
        ? format(r.timestamp, 'MM/dd')
        : format(r.timestamp, 'HH:mm')
    );

    const chartData = {
      labels: labels.filter((_, i) => i % Math.floor(filteredData.length / 5) === 0),
      datasets: [
        {
          data: values,
          color: (opacity = 1) => color,
          strokeWidth: 2,
        },
      ],
    };

    return (
      <View style={styles.chartContainer}>
        <Text style={styles.chartTitle}>{title}</Text>
        <LineChart
          data={chartData}
          width={screenWidth}
          height={220}
          chartConfig={{
            backgroundColor: '#1E1E1E',
            backgroundGradientFrom: '#1E1E1E',
            backgroundGradientTo: '#1E1E1E',
            decimalPlaces: 0,
            color: (opacity = 1) => color,
            labelColor: (opacity = 1) => `rgba(176, 176, 176, ${opacity})`,
            propsForDots: {
              r: '3',
              strokeWidth: '2',
            },
          }}
          bezier
          style={styles.chart}
          withInnerLines={true}
          withOuterLines={true}
          withVerticalLines={false}
        />
        <View style={styles.stats}>
          <View style={styles.stat}>
            <Text style={styles.statLabel}>Average</Text>
            <Text style={styles.statValue}>{stats.avg}</Text>
          </View>
          <View style={styles.stat}>
            <Text style={styles.statLabel}>Peak</Text>
            <Text style={styles.statValue}>{stats.peak}</Text>
          </View>
          <View style={styles.stat}>
            <Text style={styles.statLabel}>Min</Text>
            <Text style={styles.statValue}>{stats.min}</Text>
          </View>
        </View>
      </View>
    );
  };

  return (
    <ScrollView style={styles.container}>
      <View style={styles.rangeSelector}>
        {(['1H', '6H', '24H', '7D', '30D'] as TimeRange[]).map((range) => (
          <TouchableOpacity
            key={range}
            style={[
              styles.rangeButton,
              selectedRange === range && styles.rangeButtonActive,
            ]}
            onPress={() => setSelectedRange(range)}
          >
            <Text
              style={[
                styles.rangeButtonText,
                selectedRange === range && styles.rangeButtonTextActive,
              ]}
            >
              {range}
            </Text>
          </TouchableOpacity>
        ))}
      </View>

      {renderChart('CO₂ Levels', 'co2', 'rgba(244, 67, 54, 1)')}
      {renderChart('VOC Index', 'voc', 'rgba(255, 152, 0, 1)')}
      {renderChart('NOx Index', 'nox', 'rgba(255, 193, 7, 1)')}
      {renderChart('Temperature (°C)', 'temperature', 'rgba(33, 150, 243, 1)')}

      <TouchableOpacity style={styles.exportButton}>
        <Text style={styles.exportButtonText}>EXPORT DATA</Text>
      </TouchableOpacity>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#121212',
    padding: 16,
  },
  rangeSelector: {
    flexDirection: 'row',
    marginBottom: 24,
    backgroundColor: '#1E1E1E',
    borderRadius: 8,
    padding: 4,
  },
  rangeButton: {
    flex: 1,
    paddingVertical: 8,
    alignItems: 'center',
    borderRadius: 6,
  },
  rangeButtonActive: {
    backgroundColor: '#2196F3',
  },
  rangeButtonText: {
    color: '#B0B0B0',
    fontSize: 14,
    fontWeight: '600',
  },
  rangeButtonTextActive: {
    color: '#FFFFFF',
  },
  chartContainer: {
    backgroundColor: '#1E1E1E',
    borderRadius: 16,
    padding: 16,
    marginBottom: 16,
  },
  chartTitle: {
    color: '#FFFFFF',
    fontSize: 16,
    fontWeight: '600',
    marginBottom: 12,
  },
  chart: {
    marginVertical: 8,
    borderRadius: 16,
  },
  stats: {
    flexDirection: 'row',
    justifyContent: 'space-around',
    marginTop: 16,
  },
  stat: {
    alignItems: 'center',
  },
  statLabel: {
    color: '#B0B0B0',
    fontSize: 12,
    marginBottom: 4,
  },
  statValue: {
    color: '#FFFFFF',
    fontSize: 18,
    fontWeight: 'bold',
  },
  noData: {
    height: 220,
    justifyContent: 'center',
    alignItems: 'center',
  },
  noDataText: {
    color: '#666666',
    fontSize: 14,
  },
  exportButton: {
    backgroundColor: '#2196F3',
    padding: 16,
    borderRadius: 8,
    alignItems: 'center',
    marginTop: 8,
    marginBottom: 32,
  },
  exportButtonText: {
    color: '#FFFFFF',
    fontWeight: '600',
    fontSize: 16,
  },
});
