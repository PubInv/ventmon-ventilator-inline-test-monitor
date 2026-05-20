#pragma once

#include <Arduino.h>

static const uint8_t VENTMON_MAX_MQTT_TOPICS = 8;

enum VentMonTopicKind : uint8_t {
  TOPIC_MEASUREMENT = 0,
  TOPIC_ALARM = 1,
  TOPIC_STATUS = 2,   // Deprecated: publishing status is disabled/no-op.
  TOPIC_CUSTOM = 3    // Deprecated: unknown/custom topic types are treated as alarms.
};

struct VentMonMqttTopic {
  String label;
  String topic;
  VentMonTopicKind kind;
  bool enabled;
};

struct VentMonNetworkConfig {
  String mqttHost;
  uint16_t mqttPort;
  String mqttUser;
  String mqttPassword;
  String deviceName;
  bool mqttEnabled;
  VentMonMqttTopic topics[VENTMON_MAX_MQTT_TOPICS];
  uint8_t topicCount;
};

typedef void (*VentMonStatusCallback)(const char *line1, const char *line2);

void networkServiceSetStatusCallback(VentMonStatusCallback callback);
void networkServiceBegin();
void networkServiceLoop();
void networkServicePublishMeasurement(const char *jsonPayload);
void networkServicePublishAlarm(const char *alarmPayload);
void networkServicePublishStatus(const char *statusPayload, bool retained = true);
VentMonNetworkConfig networkServiceGetConfig();
bool networkServiceWifiConnected();
bool networkServiceMqttConnected();
String networkServiceIpAddress();
