import React from 'react';
import { View, Text, StyleSheet, Platform, Linking, TouchableOpacity } from 'react-native';

interface iOSBannerProps {
  visible: boolean;
}

export default function iOSBanner({ visible }: iOSBannerProps) {
  if (!visible) return null;

  const handleDownloadPress = () => {
    // TODO: Update this URL when TestFlight/App Store link is available
    const appStoreUrl = 'https://github.com/Havamal-65/air-quality-sensor#ios-installation';
    Linking.openURL(appStoreUrl);
  };

  return (
    <View style={styles.container}>
      <View style={styles.banner}>
        <Text style={styles.icon}>📱</Text>
        <View style={styles.textContainer}>
          <Text style={styles.title}>iOS Users: Native App Required</Text>
          <Text style={styles.message}>
            Web Bluetooth is not supported on iOS Safari. Download the native app for full BLE functionality.
          </Text>
        </View>
        <TouchableOpacity style={styles.button} onPress={handleDownloadPress}>
          <Text style={styles.buttonText}>Get App</Text>
        </TouchableOpacity>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    width: '100%',
    paddingHorizontal: 16,
    paddingVertical: 12,
  },
  banner: {
    backgroundColor: '#FF9800',
    borderRadius: 12,
    padding: 16,
    flexDirection: 'row',
    alignItems: 'center',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
  },
  icon: {
    fontSize: 32,
    marginRight: 12,
  },
  textContainer: {
    flex: 1,
    marginRight: 12,
  },
  title: {
    fontSize: 16,
    fontWeight: 'bold',
    color: '#fff',
    marginBottom: 4,
  },
  message: {
    fontSize: 13,
    color: '#fff',
    lineHeight: 18,
  },
  button: {
    backgroundColor: '#fff',
    paddingHorizontal: 16,
    paddingVertical: 8,
    borderRadius: 8,
  },
  buttonText: {
    color: '#FF9800',
    fontWeight: 'bold',
    fontSize: 14,
  },
});
