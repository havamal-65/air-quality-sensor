import * as Notifications from 'expo-notifications';
import * as Haptics from 'expo-haptics';
import { Alert } from '../types/sensor';

Notifications.setNotificationHandler({
  handleNotification: async () => ({
    shouldShowAlert: true,
    shouldPlaySound: false,
    shouldSetBadge: true,
  }),
});

export async function requestNotificationPermissions(): Promise<boolean> {
  const { status: existingStatus } = await Notifications.getPermissionsAsync();
  let finalStatus = existingStatus;

  if (existingStatus !== 'granted') {
    const { status } = await Notifications.requestPermissionsAsync();
    finalStatus = status;
  }

  return finalStatus === 'granted';
}

export async function sendAlertNotification(alert: Alert): Promise<void> {
  const hasPermission = await requestNotificationPermissions();

  if (!hasPermission) {
    console.warn('Notification permissions not granted');
    return;
  }

  const title = alert.severity === 'critical' ? '🚨 Critical Alert' : '⚠️ Warning';
  const body = alert.message;

  await Notifications.scheduleNotificationAsync({
    content: {
      title,
      body,
      data: { alertId: alert.id, type: alert.type },
      sound: false,
      priority: Notifications.AndroidNotificationPriority.HIGH,
    },
    trigger: null, // Send immediately
  });

  // Haptic feedback for critical alerts
  if (alert.severity === 'critical') {
    Haptics.notificationAsync(Haptics.NotificationFeedbackType.Error);
  } else {
    Haptics.notificationAsync(Haptics.NotificationFeedbackType.Warning);
  }
}

export async function cancelAllNotifications(): Promise<void> {
  await Notifications.cancelAllScheduledNotificationsAsync();
}

export function setupNotificationListener(
  onNotificationReceived: (notification: Notifications.Notification) => void
): void {
  Notifications.addNotificationReceivedListener(onNotificationReceived);
}

export function setupNotificationResponseListener(
  onNotificationResponse: (response: Notifications.NotificationResponse) => void
): void {
  Notifications.addNotificationResponseReceivedListener(onNotificationResponse);
}
