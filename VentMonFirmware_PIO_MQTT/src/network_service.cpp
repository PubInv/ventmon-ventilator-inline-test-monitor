#include "network_service.h"

#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <LittleFS.h>

namespace {
Preferences prefs;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
WebServer server(80);
VentMonNetworkConfig cfg;
VentMonStatusCallback statusCb = nullptr;
unsigned long lastMqttAttemptMs = 0;
unsigned long lastStatusMs = 0;
String lastMeasurement = "{}";
String lastAlarm = "";
String lastNetworkLine1 = "Network idle";
String lastNetworkLine2 = "";

void reportStatus(const String &line1, const String &line2) {
  lastNetworkLine1 = line1;
  lastNetworkLine2 = line2;
  Serial.print(F("[NET] "));
  Serial.print(line1);
  if (line2.length()) {
    Serial.print(F(" | "));
    Serial.print(line2);
  }
  Serial.println();
  if (statusCb) statusCb(line1.c_str(), line2.c_str());
}

const char *kindToString(VentMonTopicKind kind) {
  switch (kind) {
    case TOPIC_MEASUREMENT: return "measurement";
    case TOPIC_ALARM: return "alarm";
    default: return "alarm";
  }
}

VentMonTopicKind stringToKind(const String &kind) {
  if (kind == "measurement") return TOPIC_MEASUREMENT;
  if (kind == "alarm") return TOPIC_ALARM;
  // Safety rule: never create status/custom publishing routes. Unknown kinds become alarms.
  return TOPIC_ALARM;
}

bool isVentMonMeasurementJson(const char *payload) {
  if (!payload) return false;
  JsonDocument doc;
  if (deserializeJson(doc, payload) != DeserializationError::Ok) return false;
  const char *event = doc["event"] | "";
  return strcmp(event, "M") == 0;
}


const char *contentType(const String &path) {
  if (path.endsWith(".html") || path.endsWith(".htm")) return "text/html";
  if (path.endsWith(".css")) return "text/css";
  if (path.endsWith(".js")) return "application/javascript";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".png")) return "image/png";
  if (path.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}

void addCacheHeaders(const String &path) {
  if (path.endsWith(".html") || path.endsWith(".htm")) {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  } else {
    server.sendHeader("Cache-Control", "public, max-age=86400, immutable");
  }
}

bool serveLittleFSFile(String path) {
  if (path.length() == 0) path = "/index.html";
  if (path.endsWith("/")) path += "index.html";

  bool useGzip = false;
  String filePath = path;
  if (LittleFS.exists(path + ".gz")) {
    filePath = path + ".gz";
    useGzip = true;
  } else if (!LittleFS.exists(path)) {
    return false;
  }

  File file = LittleFS.open(filePath, "r");
  if (!file) return false;

  addCacheHeaders(path);
  if (useGzip) server.sendHeader("Content-Encoding", "gzip");
  server.streamFile(file, contentType(path));
  file.close();
  Serial.println("[WEB] Served " + filePath);
  return true;
}

void setDefaultTopics() {
  cfg.topicCount = 2;
  cfg.topics[0] = {"VentMon updates", "ventmon/updates", TOPIC_MEASUREMENT, true};
  cfg.topics[1] = {"Custom alarms", "ventmon/alarms", TOPIC_ALARM, true};
}

void loadConfig() {
  prefs.begin("ventmon-net", false);
  cfg.mqttHost = prefs.getString("host", "public.cloud.shiftr.io");
  cfg.mqttPort = prefs.getUShort("port", 1883);
  cfg.mqttUser = prefs.getString("user", "public");
  cfg.mqttPassword = prefs.getString("pass", "public");
  cfg.deviceName = prefs.getString("dev", "VentMon");
  cfg.mqttEnabled = prefs.getBool("men", true);
  setDefaultTopics();
  String topicJson = prefs.getString("topics", "");
  if (topicJson.length()) {
    JsonDocument doc;
    if (deserializeJson(doc, topicJson) == DeserializationError::Ok && doc.is<JsonArray>()) {
      cfg.topicCount = 0;
      for (JsonObject obj : doc.as<JsonArray>()) {
        if (cfg.topicCount >= VENTMON_MAX_MQTT_TOPICS) break;
        String topic = obj["topic"] | "";
        if (!topic.length()) continue;
        cfg.topics[cfg.topicCount].label = String((const char *)(obj["label"] | "Topic"));
        cfg.topics[cfg.topicCount].topic = topic;
        cfg.topics[cfg.topicCount].kind = stringToKind(String((const char *)(obj["kind"] | "custom")));
        cfg.topics[cfg.topicCount].enabled = obj["enabled"] | true;
        cfg.topicCount++;
      }
      if (cfg.topicCount == 0) setDefaultTopics();
    }
  }
}

String configJson() {
  JsonDocument doc;
  doc["mqttHost"] = cfg.mqttHost;
  doc["mqttPort"] = cfg.mqttPort;
  doc["mqttUser"] = cfg.mqttUser;
  doc["mqttPassword"] = cfg.mqttPassword;
  doc["deviceName"] = cfg.deviceName;
  doc["mqttEnabled"] = cfg.mqttEnabled;
  JsonArray arr = doc["topics"].to<JsonArray>();
  for (uint8_t i = 0; i < cfg.topicCount; i++) {
    JsonObject o = arr.add<JsonObject>();
    o["label"] = cfg.topics[i].label;
    o["topic"] = cfg.topics[i].topic;
    o["kind"] = kindToString(cfg.topics[i].kind);
    o["enabled"] = cfg.topics[i].enabled;
  }
  String out;
  serializeJson(doc, out);
  return out;
}

void saveConfigFromBody() {
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"bad json\"}");
    return;
  }
  cfg.mqttHost = String((const char *)(doc["mqttHost"] | cfg.mqttHost.c_str()));
  cfg.mqttPort = doc["mqttPort"] | cfg.mqttPort;
  cfg.mqttUser = String((const char *)(doc["mqttUser"] | cfg.mqttUser.c_str()));
  cfg.mqttPassword = String((const char *)(doc["mqttPassword"] | cfg.mqttPassword.c_str()));
  cfg.deviceName = String((const char *)(doc["deviceName"] | cfg.deviceName.c_str()));
  cfg.mqttEnabled = doc["mqttEnabled"] | cfg.mqttEnabled;
  if (doc["topics"].is<JsonArray>()) {
    cfg.topicCount = 0;
    for (JsonObject obj : doc["topics"].as<JsonArray>()) {
      if (cfg.topicCount >= VENTMON_MAX_MQTT_TOPICS) break;
      String topic = String((const char *)(obj["topic"] | ""));
      topic.trim();
      if (!topic.length()) continue;
      cfg.topics[cfg.topicCount].label = String((const char *)(obj["label"] | "Topic"));
      cfg.topics[cfg.topicCount].topic = topic;
      cfg.topics[cfg.topicCount].kind = stringToKind(String((const char *)(obj["kind"] | "custom")));
      cfg.topics[cfg.topicCount].enabled = obj["enabled"] | true;
      cfg.topicCount++;
    }
  }
  prefs.putString("host", cfg.mqttHost);
  prefs.putUShort("port", cfg.mqttPort);
  prefs.putString("user", cfg.mqttUser);
  prefs.putString("pass", cfg.mqttPassword);
  prefs.putString("dev", cfg.deviceName);
  prefs.putBool("men", cfg.mqttEnabled);
  JsonDocument topicsDoc;
  JsonArray arr = topicsDoc.to<JsonArray>();
  for (uint8_t i = 0; i < cfg.topicCount; i++) {
    JsonObject o = arr.add<JsonObject>();
    o["label"] = cfg.topics[i].label;
    o["topic"] = cfg.topics[i].topic;
    o["kind"] = kindToString(cfg.topics[i].kind);
    o["enabled"] = cfg.topics[i].enabled;
  }
  String topicsOut;
  serializeJson(topicsDoc, topicsOut);
  prefs.putString("topics", topicsOut);
  mqtt.setServer(cfg.mqttHost.c_str(), cfg.mqttPort);
  mqtt.disconnect();
  reportStatus("MQTT config saved", cfg.mqttHost + ":" + String(cfg.mqttPort));
  server.send(200, "application/json", "{\"ok\":true}");
}

void publishToKind(VentMonTopicKind kind, const char *payload, bool retained = false) {
  if (!payload || !mqtt.connected()) return;
  for (uint8_t i = 0; i < cfg.topicCount; i++) {
    if (cfg.topics[i].enabled && cfg.topics[i].kind == kind && cfg.topics[i].topic.length()) {
      mqtt.publish(cfg.topics[i].topic.c_str(), payload, retained);
    }
  }
}

void setupWeb() {
  server.on("/", HTTP_GET, [](){
    if (!serveLittleFSFile("/index.html")) server.send(404, "text/plain", "Not found");
  });
  server.on("/mqtt", HTTP_GET, [](){
    if (!serveLittleFSFile("/mqtt.html")) server.send(404, "text/plain", "Not found");
  });
  server.on("/api/config", HTTP_GET, [](){ server.send(200, "application/json", configJson()); });
  server.on("/api/config", HTTP_POST, saveConfigFromBody);
  server.on("/api/status", HTTP_GET, [](){
    JsonDocument doc;
    doc["wifi"] = WiFi.isConnected();
    doc["ip"] = WiFi.localIP().toString();
    doc["mqtt"] = mqtt.connected();
    doc["line1"] = lastNetworkLine1;
    doc["line2"] = lastNetworkLine2;
    JsonDocument m;
    if (deserializeJson(m, lastMeasurement) == DeserializationError::Ok) doc["lastMeasurement"] = m;
    else doc["lastMeasurementRaw"] = lastMeasurement;
    doc["lastAlarm"] = lastAlarm;
    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
  });
  server.on("/reset-wifi", HTTP_POST, [](){
    WiFiManager wm;
    wm.resetSettings();
    reportStatus("WiFi reset saved", "Reboot to configure");
    server.send(200, "application/json", "{\"ok\":true}");
  });

  server.onNotFound([](){
    const String uri = server.uri();
    if (uri.startsWith("/api/")) {
      server.send(404, "application/json", "{\"ok\":false,\"error\":\"not found\"}");
      return;
    }
    if (serveLittleFSFile(uri)) return;
    if (serveLittleFSFile("/index.html")) return;
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
  Serial.println(F("[WEB] Web server started"));
}

void connectMqttIfNeeded() {
  if (!cfg.mqttEnabled || mqtt.connected() || WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastMqttAttemptMs < 5000) return;
  lastMqttAttemptMs = millis();
  Serial.println(F("[MQTT] Connecting..."));
  reportStatus("MQTT connecting", cfg.mqttHost + ":" + String(cfg.mqttPort));
  String clientId = cfg.deviceName + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  // No MQTT Last-Will / online announcement: broker receives only VentMon updates and explicit custom alarms.
  bool ok = mqtt.connect(clientId.c_str(), cfg.mqttUser.c_str(), cfg.mqttPassword.c_str());
  if (ok) {
    Serial.println(F("[MQTT] Connected"));
    reportStatus("MQTT connected", cfg.mqttHost);
  } else {
    Serial.println(F("[MQTT] Connection failed"));
    reportStatus("MQTT failed", "state " + String(mqtt.state()));
  }
}
}

void networkServiceSetStatusCallback(VentMonStatusCallback callback) {
  statusCb = callback;
}

void networkServiceBegin() {
  Serial.println(F("[WIFI] Starting WiFiManager, OTA, web visualizer, and MQTT service"));
  loadConfig();

  if (LittleFS.begin(true)) {
    Serial.println(F("[FS] LittleFS mounted"));
    reportStatus("FS mounted", "LittleFS ready");
  } else {
    Serial.println(F("[FS] LittleFS mount failed"));
    reportStatus("FS mount failed", "LittleFS error");
  }

  WiFi.mode(WIFI_STA);
  WiFiManager wm;
  wm.setConfigPortalTimeout(0); // Block until configured, but keep the user informed on LCD/Serial.
  wm.setAPCallback([](WiFiManager *manager) {
    String ap = manager->getConfigPortalSSID();
    reportStatus("Configure WiFi", "AP: " + ap);
  });
  wm.setSaveConfigCallback([]() {
    reportStatus("WiFi saved", "Connecting...");
  });
  String apName = cfg.deviceName + "-Setup";
  reportStatus("WiFi connecting", "Saved network...");
  if (!wm.autoConnect(apName.c_str())) {
    reportStatus("WiFi not set", "AP: " + apName);
  } else {
    reportStatus("WiFi connected", WiFi.localIP().toString());
  }

  ArduinoOTA.setHostname(cfg.deviceName.c_str());
  ArduinoOTA.onStart([](){ Serial.println(F("[OTA] Start")); reportStatus("OTA update", "Starting..."); });
  ArduinoOTA.onEnd([](){ Serial.println(F("[OTA] Done")); reportStatus("OTA update", "Done"); });
  ArduinoOTA.onError([](ota_error_t error){ Serial.println(F("[OTA] Error")); reportStatus("OTA error", String((int)error)); });
  ArduinoOTA.begin();
  mqtt.setServer(cfg.mqttHost.c_str(), cfg.mqttPort);
  mqtt.setBufferSize(768);
  setupWeb();
}

void networkServiceLoop() {
  ArduinoOTA.handle();
  server.handleClient();
  connectMqttIfNeeded();
  if (mqtt.connected()) mqtt.loop();
  // Intentionally no MQTT heartbeat/status publishing. Keep status local to LCD/web only.
}

void networkServicePublishMeasurement(const char *jsonPayload) {
  if (!jsonPayload) return;
  lastMeasurement = jsonPayload;
  // Safety filter: publish only VentMon update measurements like {"event":"M",...}.
  if (!isVentMonMeasurementJson(jsonPayload)) {
    Serial.println(F("[MQTT] Drop non-measurement payload; only event=M is published."));
    return;
  }
  publishToKind(TOPIC_MEASUREMENT, jsonPayload, false);
}

void networkServicePublishAlarm(const char *alarmPayload) {
  if (!alarmPayload) return;
  lastAlarm = alarmPayload;
  publishToKind(TOPIC_ALARM, alarmPayload, false);
}

void networkServicePublishStatus(const char *statusPayload, bool retained) {
  (void)statusPayload;
  (void)retained;
  // Disabled by design: never publish device status/IP/WiFi/RSSI/heartbeat to MQTT.
}

VentMonNetworkConfig networkServiceGetConfig() { return cfg; }

bool networkServiceWifiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool networkServiceMqttConnected() {
  return mqtt.connected();
}

String networkServiceIpAddress() {
  if (WiFi.status() != WL_CONNECTED) return String("NoIP");
  return WiFi.localIP().toString();
}
