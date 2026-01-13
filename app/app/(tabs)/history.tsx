import React, { useState } from 'react';
import { View, Text, StyleSheet, ScrollView, TouchableOpacity } from 'react-native';
import { useSensorStore } from '../../store/useSensorStore';
import { LineChart } from 'react-native-chart-kit';
import { Dimensions } from 'react-native';
import { format } from 'date-fns';

type TimeRange = '5S' | '30S' | '1M' | '5M' | '15M' | '30M' | '1H' | '6H' | '12H' | '24H' | '3D' | '7D' | '14D' | '30D' | '90D' | '6Mo' | '1Y' | '5Y';

export default function HistoryScreen() {
  const [selectedRange, setSelectedRange] = useState<TimeRange>('24H');
  const historicalData = useSensorStore((state) => state.historicalData);

  const screenWidth = Dimensions.get('window').width - 32;

  const getFilteredData = () => {
    const now = Date.now();
    const ranges: Record<TimeRange, number> = {
      '5S': 5000,
      '30S': 30000,
      '1M': 60000,
      '5M': 300000,
      '15M': 900000,
      '30M': 1800000,
      '1H': 3600000,
      '6H': 21600000,
      '12H': 43200000,
      '24H': 86400000,
      '3D': 259200000,
      '7D': 604800000,
      '14D': 1209600000,
      '30D': 2592000000,
      '90D': 7776000000,
      '6Mo': 15552000000,
      '1Y': 31536000000,
      '5Y': 157680000000,
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

    // Determine label format based on time range
    const getLabelFormat = () => {
      const shortRanges = ['5S', '30S', '1M', '5M', '15M', '30M'];
      const hourRanges = ['1H', '6H', '12H', '24H'];
      const dayRanges = ['3D', '7D', '14D', '30D'];
      const longRanges = ['90D', '6Mo', '1Y', '5Y'];

      if (shortRanges.includes(selectedRange)) {
        return 'HH:mm:ss';
      } else if (hourRanges.includes(selectedRange)) {
        return 'HH:mm';
      } else if (dayRanges.includes(selectedRange)) {
        return 'MM/dd HH:mm';
      } else {
        return 'MM/dd/yy';
      }
    };

    const labels = filteredData.map((r) => format(r.timestamp, getLabelFormat()));

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
      labels: labels.filter((_, i) => i % Math.floor(filteredData.length / 5) === 0),
      datasets: [
        {
          data: [yAxisMin, ...values, yAxisMax],
          color: (opacity = 1) => color,
          strokeWidth: 3,
          withDots: false,
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
            color: (opacity = 1) => color,
            labelColor: (opacity = 1) => `rgba(176, 176, 176, ${opacity})`,
            propsForDots: {
              r: '4',
              strokeWidth: '2',
              fill: color,
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

  const timeRanges: TimeRange[] = [
    '5S', '30S', '1M', '5M', '15M', '30M',
    '1H', '6H', '12H', '24H',
    '3D', '7D', '14D', '30D',
    '90D', '6Mo', '1Y', '5Y'
  ];

  return (
    <ScrollView style={styles.container}>
      <ScrollView
        horizontal
        showsHorizontalScrollIndicator={false}
        style={styles.rangeSelector}
        contentContainerStyle={styles.rangeSelectorContent}
      >
        {timeRanges.map((range) => (
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
      </ScrollView>

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
    marginBottom: 24,
    maxHeight: 48,
  },
  rangeSelectorContent: {
    backgroundColor: '#1E1E1E',
    borderRadius: 8,
    padding: 4,
    flexDirection: 'row',
    gap: 4,
  },
  rangeButton: {
    paddingVertical: 8,
    paddingHorizontal: 12,
    alignItems: 'center',
    justifyContent: 'center',
    borderRadius: 6,
    minWidth: 50,
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
