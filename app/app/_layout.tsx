import { Stack } from 'expo-router';
import { useEffect } from 'react';
import { useSensorStore } from '../store/useSensorStore';

export default function RootLayout() {
  const loadSettings = useSensorStore((state) => state.loadSettings);

  useEffect(() => {
    loadSettings();
  }, []);

  return (
    <Stack
      screenOptions={{
        headerStyle: {
          backgroundColor: '#121212',
        },
        headerTintColor: '#FFFFFF',
        headerTitleStyle: {
          fontWeight: 'bold',
        },
      }}
    >
      <Stack.Screen name="(tabs)" options={{ headerShown: false }} />
      <Stack.Screen name="connection" options={{ title: 'Connect Device' }} />
    </Stack>
  );
}
