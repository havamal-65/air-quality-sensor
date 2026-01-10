// Air quality thresholds and status levels

export enum AirQualityLevel {
  EXCELLENT = 'excellent',
  GOOD = 'good',
  MODERATE = 'moderate',
  POOR = 'poor',
  BAD = 'bad',
}

export const CO2_THRESHOLDS = {
  [AirQualityLevel.EXCELLENT]: { max: 600, color: '#4CAF50', label: 'Excellent' },
  [AirQualityLevel.GOOD]: { max: 800, color: '#4CAF50', label: 'Good' },
  [AirQualityLevel.MODERATE]: { max: 1000, color: '#FFC107', label: 'Moderate' },
  [AirQualityLevel.POOR]: { max: 1500, color: '#FF9800', label: 'Poor' },
  [AirQualityLevel.BAD]: { max: Infinity, color: '#F44336', label: 'Bad' },
};

export const VOC_THRESHOLDS = {
  [AirQualityLevel.GOOD]: { max: 150, color: '#4CAF50', label: 'Good' },
  [AirQualityLevel.MODERATE]: { max: 250, color: '#FFC107', label: 'Moderate' },
  [AirQualityLevel.POOR]: { max: 400, color: '#FF9800', label: 'Poor' },
  [AirQualityLevel.BAD]: { max: Infinity, color: '#F44336', label: 'Bad' },
};

export const NOX_THRESHOLDS = {
  [AirQualityLevel.GOOD]: { max: 150, color: '#4CAF50', label: 'Good' },
  [AirQualityLevel.MODERATE]: { max: 250, color: '#FFC107', label: 'Moderate' },
  [AirQualityLevel.POOR]: { max: 400, color: '#FF9800', label: 'Poor' },
  [AirQualityLevel.BAD]: { max: Infinity, color: '#F44336', label: 'Bad' },
};

export function getCO2Level(ppm: number): { level: AirQualityLevel; color: string; label: string } {
  if (ppm < 600) return { level: AirQualityLevel.EXCELLENT, ...CO2_THRESHOLDS.excellent };
  if (ppm < 800) return { level: AirQualityLevel.GOOD, ...CO2_THRESHOLDS.good };
  if (ppm < 1000) return { level: AirQualityLevel.MODERATE, ...CO2_THRESHOLDS.moderate };
  if (ppm < 1500) return { level: AirQualityLevel.POOR, ...CO2_THRESHOLDS.poor };
  return { level: AirQualityLevel.BAD, ...CO2_THRESHOLDS.bad };
}

export function getVOCLevel(index: number): { level: AirQualityLevel; color: string; label: string } {
  if (index < 150) return { level: AirQualityLevel.GOOD, ...VOC_THRESHOLDS.good };
  if (index < 250) return { level: AirQualityLevel.MODERATE, ...VOC_THRESHOLDS.moderate };
  if (index < 400) return { level: AirQualityLevel.POOR, ...VOC_THRESHOLDS.poor };
  return { level: AirQualityLevel.BAD, ...VOC_THRESHOLDS.bad };
}

export function getNOxLevel(index: number): { level: AirQualityLevel; color: string; label: string } {
  if (index < 150) return { level: AirQualityLevel.GOOD, ...NOX_THRESHOLDS.good };
  if (index < 250) return { level: AirQualityLevel.MODERATE, ...NOX_THRESHOLDS.moderate };
  if (index < 400) return { level: AirQualityLevel.POOR, ...NOX_THRESHOLDS.poor };
  return { level: AirQualityLevel.BAD, ...NOX_THRESHOLDS.bad };
}

// Calculate overall air quality score (0-100)
export function calculateOverallScore(co2: number, voc: number, nox: number): number {
  const co2Score = Math.max(0, 100 - (co2 - 400) / 20);
  const vocScore = Math.max(0, 100 - voc / 5);
  const noxScore = Math.max(0, 100 - nox / 5);

  return Math.round((co2Score + vocScore + noxScore) / 3);
}
