# VentMon Firmware - PlatformIO networked build

This PlatformIO project keeps the in-house `Pirds` and `SFM3X00` libraries under `lib/` and adds a separate network service module.

## Added features

- WiFiManager captive portal.
- Blocking WiFi setup now reports on the OLED/LCD through `networkServiceSetStatusCallback()`.
- Arduino OTA enabled after WiFi connects.
- Web visualizer at `http://<device-ip>/`.
- Dedicated MQTT configuration page at `http://<device-ip>/mqtt`.
- MQTT host, port, username, password, device name, and publish topic list are saved in ESP32 Preferences/NVS.
- Up to 8 publish topics can be added, enabled, disabled, selected, and categorized as `measurement`, `alarm`, `status`, or `custom`.
- Measurement/alarm/status publishing goes only to enabled topics of the matching category.

## First boot behavior

If no WiFi is configured, the device opens a setup AP named:

`VentMon-Setup`

The display should show:

`Configure WiFi`

and the AP name, so the user knows WiFi must be configured.

## Build/upload

```bash
pio run
pio run -t upload
pio device monitor
```

## Important file locations

- `src/main.ino` - original firmware entry point, now calls the network service.
- `src/network_service.cpp` - WiFiManager, OTA, web UI, MQTT config, topic selector, and publishing logic.
- `include/network_service.h` - network service API.
- `lib/Pirds` and `lib/SFM3X00` - in-house libraries.

## LCD/OLED temporary network setup summary

During blocking WiFiManager setup, the display shows clear setup instructions so the user knows WiFi must be configured.

After setup finishes, the display shows the network summary for 30 seconds, then returns the full OLED/LCD area to the normal VentMon screen:

`W:1 B:1 192.168.x.x`

- `W:1` means WiFi connected, `W:0` means not connected/config portal.
- `B:1` means MQTT broker connected, `B:0` means broker disconnected.
- The final field is the IP address serving the web visualizer and `/mqtt` page, or `NoIP` if WiFi is not connected.

This is controlled in `src/main.ino` with `NETWORK_SUMMARY_AFTER_SETUP_MS` set to `30000UL`.

## MQTT alarm parameter

The most useful parameter for a needed ventilator alarm is inspiratory airway pressure, published as `inspiratory_pressure_cmH2O` in MQTT pressure measurements and high-pressure alarm payloads. VentMon raises a `HIGH_PRESSURE` alarm when the inspiratory differential/absolute pressure reaches `40.0 cmH2O` and keeps the alarm latched until pressure falls to `35.0 cmH2O`, avoiding repeated alarm messages while pressure remains high.

VentMon also raises `SUDDEN_PRESSURE_DROP` when consecutive readings from the same inspiratory pressure stream drop by at least `10.0 cmH2O`, which can indicate a test lung or patient circuit disconnect. The drop alarm remains latched until the pressure delta stabilizes to `3.0 cmH2O` or less.

Example high-pressure alarm payload:

```json
{
  "event": "A",
  "alarm": "HIGH_PRESSURE",
  "severity": "high",
  "measurement": "differential_pressure",
  "location": "inspiratory",
  "parameter": "inspiratory_pressure_cmH2O",
  "value": 42.5,
  "threshold": 40.0,
  "unit": "cmH2O",
  "timestamp_ms": 123456
}
```

Example sudden pressure drop alarm payload:

```text
a5 Hose disconnected
```

## MQTT publish restriction

This build intentionally publishes only two message classes to the broker:

1. VentMon measurement/update JSON where `event` is exactly `M`, for example:
   `{ "event": "M", "type": "F", "ms": 36570, "loc": "I", "num": 0, "val": -33 }`
2. Explicit custom alarms sent through `networkServicePublishAlarm(...)`.

It does **not** publish WiFi status, IP address, RSSI, heartbeat/alive messages, online/offline messages, OTA status, or web visualizer/device-info messages to MQTT. Those remain local only on Serial/LCD/web.
