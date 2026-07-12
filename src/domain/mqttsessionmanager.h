#ifndef MQTTSESSIONMANAGER_H
#define MQTTSESSIONMANAGER_H

class MqttSessionManager
{
public:
  enum MqttState
  {
    MQTT_DISCONNECTED_STATE,
    MQTT_CONNECTING_STATE,
    MQTT_CONNECTED_STATE
  };

  MqttSessionManager(unsigned long retryIntervalMs = 5000,
                     int maxReconnectAttempts = 5);

  bool shouldAttemptConnect(unsigned long now);
  void onConnectAttemptStarted();
  void onConnectSuccess();
  void onConnectFailure();
  void onConnectionLost();
  void forceDisconnect();

  bool isConnected() const;
  MqttState state() const;
  int reconnectAttempts() const;

private:
  MqttState _state;
  unsigned long _lastMqttAttemptMillis;
  const unsigned long _mqttRetryInterval;
  int _mqttReconnectAttempts;
  const int _maxMqttReconnectAttempts;
};

#endif // MQTTSESSIONMANAGER_H
