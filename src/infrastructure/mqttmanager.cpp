#include "mqttmanager.h"
#include "config.h"
#include "trace.h"

MqttManager::MqttManager()
    : _mqttServer(nullptr), _mqttPort(1883), _mqttUser(nullptr),
      _mqttPassword(nullptr), _clientId(nullptr), _connected(false),
      _lastAttemptMs(0), _subscribeCount(0)
{
  _pubSubClient.setClient(_wifiClient);
}

void MqttManager::setup(const char *mqttServer, int mqttPort,
                        const char *mqttUser, const char *mqttPassword,
                        const char *clientId)
{
  _mqttServer = mqttServer;
  _mqttPort = mqttPort;
  _mqttUser = mqttUser;
  _mqttPassword = mqttPassword;
  _clientId = clientId;
  _pubSubClient.setServer(_mqttServer, _mqttPort);
  Trace::log(TraceLevel::INFO, "MqttManager setup complete");
}

bool MqttManager::loop(bool wifiConnected)
{
  if (!wifiConnected)
  {
    disconnectIfNeeded();
    return false;
  }

  if (_pubSubClient.connected())
  {
    _pubSubClient.loop();
    return false;
  }

  // One state transition per call: report the loss now and reconnect on the
  // next call, so callers see a disconnected state between two connections.
  if (_connected)
  {
    Trace::logf(TraceLevel::WARNING, "MQTT connection lost, rc=%d", _pubSubClient.state());
    _connected = false;
    return false;
  }

  if (millis() - _lastAttemptMs >= MQTT_RETRY_INTERVAL_MS)
  {
    _lastAttemptMs = millis();
    return connect();
  }

  return false;
}

bool MqttManager::connect()
{
  Trace::log(TraceLevel::INFO, "MQTT connecting...");

  if (!_pubSubClient.connect(_clientId, _mqttUser, _mqttPassword,
                             getLwtTopic(), 1, true, "offline"))
  {
    Trace::logf(TraceLevel::ERROR, "MQTT connect failed, rc=%d", _pubSubClient.state());
    return false;
  }

  Trace::log(TraceLevel::INFO, "MQTT connected");
  _connected = true;
  _pubSubClient.publish(getLwtTopic(), "online", true);

  for (int i = 0; i < _subscribeCount; i++)
  {
    _pubSubClient.subscribe(_subscribeTopics[i]);
  }
  return true;
}

void MqttManager::disconnectIfNeeded()
{
  // Runs on every loop while WiFi is down; PubSubClient::disconnect() would
  // write to the socket and overwrite the last rc each time.
  if (_pubSubClient.connected())
  {
    _pubSubClient.disconnect();
  }
  _connected = false;
}

void MqttManager::publish(const char *topic, const char *payload)
{
  if (_connected)
  {
    _pubSubClient.publish(topic, payload);
  }
}

void MqttManager::publishRetained(const char *topic, const char *payload)
{
  if (_connected)
  {
    _pubSubClient.publish(topic, payload, true);
  }
}

bool MqttManager::isConnected() const
{
  return _connected;
}

void MqttManager::subscribe(const char *topic)
{
  if (_subscribeCount < MAX_SUBSCRIPTIONS)
  {
    _subscribeTopics[_subscribeCount++] = topic;
  }
}

void MqttManager::setCallback(std::function<void(char *, uint8_t *, unsigned int)> callback)
{
  _pubSubClient.setCallback(callback);
}

const char *MqttManager::getLwtTopic() const
{
  snprintf(_lwtTopic, sizeof(_lwtTopic), "%s/system/status", _clientId);
  return _lwtTopic;
}
