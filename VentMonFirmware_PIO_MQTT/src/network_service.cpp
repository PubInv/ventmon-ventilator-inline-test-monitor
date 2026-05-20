#include "network_service.h"

#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiManager.h>

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

String htmlPage() {
  return F(R"rawliteral(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1">
<title>VentMon Visualizer</title>
<style>
body{font-family:Arial,sans-serif;margin:0;background:#101827;color:#eef2ff}header{padding:16px;background:#172036;position:sticky;top:0}main{padding:16px;display:grid;gap:16px;grid-template-columns:repeat(auto-fit,minmax(280px,1fr))}.card{background:#1f2a44;border-radius:16px;padding:16px;box-shadow:0 8px 20px #0005}a{color:#93c5fd}input,select{width:100%;padding:10px;margin:6px 0 12px;border-radius:10px;border:0;box-sizing:border-box}button{padding:10px 14px;border:0;border-radius:10px;cursor:pointer;margin:4px 4px 4px 0}pre{white-space:pre-wrap;word-break:break-word;background:#0b1020;padding:12px;border-radius:12px}.ok{color:#86efac}.bad{color:#fca5a5}.small{opacity:.8;font-size:.9rem}</style>
</head><body><header><h2>VentMon Web Visualizer</h2><div id="net"></div><p><a href="/mqtt">Open MQTT configuration and topic selector</a></p></header><main>
<section class="card"><h3>Live measurement</h3><pre id="meas">loading...</pre><h3>Last alarm</h3><pre id="alarm"></pre></section>
<section class="card"><h3>Network status</h3><pre id="lines"></pre><button onclick="fetch('/reset-wifi',{method:'POST'}).then(()=>alert('Saved. Reboot/reset to open WiFi portal.'))">Reset WiFi</button></section>
</main><script>
async function refresh(){let s=await (await fetch('/api/status')).json();net.innerHTML=`WiFi: <b class="${s.wifi?'ok':'bad'}">${s.ip}</b> MQTT: <b class="${s.mqtt?'ok':'bad'}">${s.mqtt?'connected':'offline'}</b>`;meas.textContent=JSON.stringify(s.lastMeasurement,null,2);alarm.textContent=s.lastAlarm||'';lines.textContent=(s.line1||'')+'\n'+(s.line2||'');}
refresh();setInterval(refresh,1000);
</script></body></html>
)rawliteral");
}

String mqttPage() {
  return F(R"rawliteral(
<!doctype html><html><head><meta name="viewport" content="width=device-width,initial-scale=1"><title>VentMon MQTT</title>
<style>body{font-family:Arial,sans-serif;margin:0;background:#101827;color:#eef2ff}main{padding:16px;max-width:900px;margin:auto}.card{background:#1f2a44;border-radius:16px;padding:16px;margin:12px 0;box-shadow:0 8px 20px #0005}input,select{width:100%;padding:10px;margin:6px 0 12px;border-radius:10px;border:0;box-sizing:border-box}button{padding:10px 14px;border:0;border-radius:10px;cursor:pointer;margin:4px 4px 4px 0}.row{display:grid;grid-template-columns:1fr 2fr 150px 90px 70px;gap:8px;align-items:end}@media(max-width:760px){.row{grid-template-columns:1fr}.hide-mobile{display:none}}a{color:#93c5fd}</style></head><body><main>
<h2>MQTT configuration</h2><p><a href="/">Back to visualizer</a></p>
<div class="card"><label><input type="checkbox" id="mqttEnabled"> MQTT enabled</label><label>MQTT host<input id="mqttHost"></label><label>MQTT port<input id="mqttPort" type="number"></label><label>MQTT user<input id="mqttUser"></label><label>MQTT password<input id="mqttPassword" type="password"></label><label>Device name<input id="deviceName"></label></div>
<div class="card"><h3>Publish topics</h3><p>Safety rule: this device publishes only VentMon measurement updates where <code>event</code> is <code>M</code>, plus explicit custom alarms. No WiFi, IP, heartbeat, status, RSSI, or device-info messages are published to MQTT.</p><div id="topics"></div><button onclick="addTopic()">+ Add topic</button><button onclick="save()">Save MQTT settings</button></div>
</main><script>
const kinds=['measurement','alarm'];
function topicRow(t={label:'',topic:'',kind:'measurement',enabled:true}){let d=document.createElement('div');d.className='row';d.innerHTML=`<label>Label<input class="label" value="${t.label||''}"></label><label>Topic<input class="topic" value="${t.topic||''}"></label><label>Type<select class="kind">${kinds.map(k=>`<option ${k==t.kind?'selected':''}>${k}</option>`).join('')}</select></label><label>Publish?<select class="enabled"><option value="true" ${t.enabled?'selected':''}>yes</option><option value="false" ${!t.enabled?'selected':''}>no</option></select></label><button onclick="this.parentElement.remove()">Remove</button>`;topics.appendChild(d)}
function addTopic(){topicRow()}
async function load(){let c=await (await fetch('/api/config')).json();mqttEnabled.checked=!!c.mqttEnabled;mqttHost.value=c.mqttHost||'';mqttPort.value=c.mqttPort||1883;mqttUser.value=c.mqttUser||'';mqttPassword.value=c.mqttPassword||'';deviceName.value=c.deviceName||'VentMon';topics.innerHTML='';(c.topics||[]).forEach(topicRow);if(!(c.topics||[]).length){addTopic();}}
async function save(){let ts=[...topics.children].map(r=>({label:r.querySelector('.label').value,topic:r.querySelector('.topic').value,kind:r.querySelector('.kind').value,enabled:r.querySelector('.enabled').value==='true'})).filter(t=>t.topic.trim());let body={mqttEnabled:mqttEnabled.checked,mqttHost:mqttHost.value,mqttPort:Number(mqttPort.value||1883),mqttUser:mqttUser.value,mqttPassword:mqttPassword.value,deviceName:deviceName.value,topics:ts};await fetch('/api/config',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)});alert('MQTT settings saved. Device will reconnect using the new settings.');}
load();
</script></body></html>
)rawliteral");
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
  server.on("/", HTTP_GET, [](){ server.send(200, "text/html", htmlPage()); });
  server.on("/mqtt", HTTP_GET, [](){ server.send(200, "text/html", mqttPage()); });
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
  server.begin();
}

void connectMqttIfNeeded() {
  if (!cfg.mqttEnabled || mqtt.connected() || WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastMqttAttemptMs < 5000) return;
  lastMqttAttemptMs = millis();
  reportStatus("MQTT connecting", cfg.mqttHost + ":" + String(cfg.mqttPort));
  String clientId = cfg.deviceName + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  // No MQTT Last-Will / online announcement: broker receives only VentMon updates and explicit custom alarms.
  bool ok = mqtt.connect(clientId.c_str(), cfg.mqttUser.c_str(), cfg.mqttPassword.c_str());
  if (ok) {
    reportStatus("MQTT connected", cfg.mqttHost);
  } else {
    reportStatus("MQTT failed", "state " + String(mqtt.state()));
  }
}
}

void networkServiceSetStatusCallback(VentMonStatusCallback callback) {
  statusCb = callback;
}

void networkServiceBegin() {
  Serial.println(F("[NET] Starting WiFiManager, OTA, web visualizer, and MQTT service"));
  loadConfig();

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
  ArduinoOTA.onStart([](){ reportStatus("OTA update", "Starting..."); });
  ArduinoOTA.onEnd([](){ reportStatus("OTA update", "Done"); });
  ArduinoOTA.onError([](ota_error_t error){ reportStatus("OTA error", String((int)error)); });
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
