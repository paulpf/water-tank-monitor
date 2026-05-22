#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include "imessagepublisher.h"
#include "imqttconnectioncontrol.h"
#include "mqttsessionmanager.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

class MqttManager : public IMessagePublisher, public IMqttConnectionControl
{
public:
  MqttManager();

  void setup(const char *mqttServer, int mqttPort, const char *mqttUser,
             const char *mqttPassword, const char *clientId);
  void loop();

  void publish(const char *topic, const char *payload) override;
  void publishRetained(const char *topic, const char *payload) override;
  bool isConnected() override;

  void requestConnect() override;
  void forceDisconnect() override;

private:
  void reconnect();
  const char *getLwtTopic() const;

  const char *_mqttServer;
  int _mqttPort;
  const char *_mqttUser;
  const char *_mqttPassword;
  const char *_clientId;
  bool _connectRequested;

  WiFiClient _wifiClient;
  PubSubClient _pubSubClient;
  MqttSessionManager _sessionManager;

  static const int LWT_TOPIC_MAX_LEN = 64;
  mutable char _lwtTopic[LWT_TOPIC_MAX_LEN];
};

#endif // MQTTMANAGER_H
