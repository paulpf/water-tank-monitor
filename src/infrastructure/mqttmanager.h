#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <functional>

class MqttManager
{
public:
  MqttManager();

  void setup(const char *mqttServer, int mqttPort, const char *mqttUser,
             const char *mqttPassword, const char *clientId);

  // Connects while WiFi is up, retries every MQTT_RETRY_INTERVAL_MS and
  // disconnects when WiFi is down. Returns true only in the call that
  // established a new connection.
  bool loop(bool wifiConnected);

  void publish(const char *topic, const char *payload);
  void publishRetained(const char *topic, const char *payload);
  bool isConnected() const;

  // Re-subscribed automatically on every (re)connect
  void subscribe(const char *topic);
  void setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback);

private:
  bool connect();
  void disconnectIfNeeded();
  const char *getLwtTopic() const;

  const char *_mqttServer;
  int _mqttPort;
  const char *_mqttUser;
  const char *_mqttPassword;
  const char *_clientId;
  bool _connected;
  unsigned long _lastAttemptMs;

  static const int MAX_SUBSCRIPTIONS = 4;
  const char *_subscribeTopics[MAX_SUBSCRIPTIONS];
  int _subscribeCount;

  WiFiClient _wifiClient;
  PubSubClient _pubSubClient;

  static const int LWT_TOPIC_MAX_LEN = 64;
  mutable char _lwtTopic[LWT_TOPIC_MAX_LEN];
};

#endif // MQTTMANAGER_H
