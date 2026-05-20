# Codex prompt for next iteration

You are editing an ESP32 PlatformIO Arduino project for VentMon.

Goals:
1. Keep custom in-house libraries under `lib/Pirds` and `lib/SFM3X00`. Do not replace them with PlatformIO registry libraries.
2. Preserve the existing sensor logic in `src/main.ino` unless a change is required for integration.
3. The network layer is intentionally isolated in `src/network_service.cpp` and `include/network_service.h`.
4. WiFiManager must visibly report blocking WiFi configuration state on the display. The current project uses `networkServiceSetStatusCallback(showNetworkStatusOnDisplay)` from `main.ino`.
5. MQTT settings must be configurable from a dedicated page `/mqtt`, not mixed only into the visualizer.
6. MQTT configuration must include host, port, username, password, device name, MQTT enable/disable, and a list of publish topics.
7. Users must be able to add, remove, enable, disable, select, and save topics. Supported topic types: `measurement`, `alarm`, `status`, and `custom`.
8. Save all MQTT settings and topic selections in ESP32 Preferences/NVS so they survive reboot.
9. Publishing behavior:
   - `networkServicePublishMeasurement()` publishes only to enabled `measurement` topics.
   - `networkServicePublishAlarm()` publishes only to enabled `alarm` topics.
   - `networkServicePublishStatus()` publishes only to enabled `status` topics.
10. Avoid long blocking delays inside the normal loop. WiFiManager can block during first configuration, but it must show the user instructions on the display.
11. Run `pio run` after changes and fix compile errors.

Suggested checks:
- Confirm ArduinoJson is listed in `platformio.ini`.
- Confirm `/mqtt` loads on desktop and phone width.
- Confirm saved topics remain after reboot.
- Confirm MQTT reconnects after saving new broker settings.
- Confirm no duplicate legacy MQTT publishing remains in `main.ino` unless intentionally kept.

## Latest requested patch
During WiFiManager blocking configuration, keep showing clear setup instructions on the OLED/LCD. After WiFi/network setup completes, show WiFi status, MQTT broker status, and the served IP address for exactly 30 seconds, then stop reserving LCD/OLED space and return the full screen to the normal VentMon display. Current summary format is `W:1 B:1 192.168.x.x`, where W is WiFi and B is broker/MQTT. This is controlled by `NETWORK_SUMMARY_AFTER_SETUP_MS = 30000UL` in `src/main.ino`.


## Important MQTT safety rule

Do not add any MQTT publishing for status, IP address, WiFi state, RSSI, OTA events, uptime, alive/heartbeat, or device info. The broker must receive only:

- VentMon update JSON messages where `event == "M"`.
- Explicit custom alarm messages through `networkServicePublishAlarm(...)`.

Keep LCD and web status local only.
