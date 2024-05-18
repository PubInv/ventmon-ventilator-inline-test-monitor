
/***************************************************************************
  Copyright Robert L. Read, 2023
  Ben Coombs, 2023
  Networking code by Geoff Mulligan 2020
  Additional work by Lauria Clarke 2020
  designed to support ethernet using esp wifi chip

  Modififications released under BSD to avoid complication (I prefer GPL).
 ***************************************************************************/

 /*
 Public Invention's Ventmon-Ventilator-Inline-Test-Monitor Project is an attempt
 to build a "test fixture" capable of running a 48-hour test on any ventilator
 design and collecting data on many important parameters. We hope to create a
 "gold standard" test that all DIY teams can work to; but this project will
 proceed in parallel with that. The idea is to make a standalone inline device
 plugged into the airway. It serves a dual purpose as a monitor/alarm when used
 on an actual patient, and a test device for testing prototype ventilators. It
 also allows for burnin. Copyright (C) 2021 Robert L. Read, Lauria Clarke,
 Ben Coombs, Darío Hereñú, and Geoff Mulligan.

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as
 published by the Free Software Foundation, either version 3 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Dns.h>
#include <PIRDS.h>
#include <Wire.h>
#include <limits.h>
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include <EEPROM.h>
#include <SFM3X00.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_BME280.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>


#define V5
//#define V4 // V0.4 VentMon with the new 128x64 OLED Screen
//#define DEBUG_1


#ifdef V5
#include <Adafruit_SH110X.h>
#else
#include <Adafruit_SSD1306.h>
#endif


#define BAUD_RATE 500000

#define LOGO_HEIGHT   64
#define LOGO_WIDTH    128
// 'logo-tight', 92x64px
/*const unsigned char logo_bmp [] PROGMEM = {
  // 'logo-tight, 92x64px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xe1, 0xff, 0xc3, 0xff, 0x7f,
  0xff, 0x00, 0x40, 0x00, 0x00, 0x27, 0xff, 0xf9, 0xff, 0xc3, 0xff, 0x7f, 0xff, 0xc3, 0xc0, 0x00,
  0x00, 0x37, 0xff, 0xff, 0xff, 0xc3, 0xff, 0x7f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x1f, 0xff, 0xff,
  0xff, 0xc3, 0xff, 0x7f, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x01, 0xe0, 0x3f, 0x1c, 0x00, 0x3c, 0x0f,
  0x01, 0xff, 0xc0, 0x00, 0x00, 0x01, 0xe0, 0x0f, 0x9c, 0x00, 0x3c, 0x0f, 0x00, 0x7f, 0xc0, 0x00,
  0x00, 0x01, 0xec, 0x07, 0x9c, 0x80, 0x3c, 0x0f, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x01, 0xfc, 0x07,
  0xdd, 0xc0, 0x3c, 0x0f, 0x00, 0x7f, 0x80, 0x00, 0x00, 0x61, 0xfc, 0x03, 0xdf, 0xc0, 0x3c, 0x0f,
  0x00, 0x7f, 0x80, 0x00, 0x00, 0x71, 0xfc, 0x03, 0xdf, 0xc0, 0x3c, 0x0f, 0x00, 0xff, 0x00, 0x00,
  0x00, 0xf9, 0xfc, 0x03, 0xdf, 0x80, 0x3c, 0x0f, 0xff, 0xff, 0x00, 0x00, 0x00, 0xfd, 0xfc, 0x03,
  0xdf, 0x00, 0x3c, 0x0f, 0xff, 0xfd, 0xf0, 0x00, 0x00, 0xff, 0xfc, 0x07, 0xdc, 0x00, 0x3c, 0x0f,
  0xff, 0xfb, 0xe0, 0x00, 0x00, 0x7f, 0xf8, 0x07, 0x9c, 0x00, 0x3c, 0x0f, 0xff, 0xff, 0xe0, 0x00,
  0x00, 0x7f, 0xf8, 0x0f, 0x9c, 0x00, 0x3c, 0x0f, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x1f, 0xf0, 0x3f,
  0x1c, 0x00, 0x3f, 0xcf, 0x00, 0x1f, 0x80, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0x1c, 0x00, 0x3f, 0xaf,
  0x00, 0x1f, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x1c, 0x00, 0x3c, 0x5f, 0x00, 0x0f, 0x00, 0x00,
  0x00, 0x01, 0xff, 0xf8, 0x1c, 0x00, 0x3c, 0x3f, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0,
  0x1f, 0xff, 0xf8, 0x1f, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x1e, 0x00, 0x78, 0x0f,
  0x00, 0x1f, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x1f, 0x00, 0xf8, 0x0f, 0x00, 0x1e, 0x00, 0x00,
  0x00, 0x01, 0xe0, 0x00, 0x1f, 0x81, 0xf0, 0x0f, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00,
  0x0f, 0xff, 0xf3, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x37, 0xfc, 0x00, 0x07, 0xff, 0xe6, 0xff,
  0xff, 0xfc, 0x00, 0x00, 0x00, 0x37, 0xfc, 0x00, 0x03, 0xff, 0xc6, 0x7f, 0xff, 0xfe, 0x00, 0x00,
  0x00, 0x1f, 0xfc, 0x00, 0x00, 0xff, 0x07, 0xff, 0xff, 0xc7, 0x00, 0x00, 0x00, 0x0c, 0x80, 0x00,
  0x18, 0x38, 0x01, 0x80, 0x70, 0x03, 0x80, 0x00, 0x00, 0x18, 0x83, 0x1c, 0x3c, 0x38, 0x00, 0x00,
  0x70, 0x01, 0xe0, 0x00, 0x00, 0x3f, 0xff, 0xbf, 0xfc, 0x3f, 0xfc, 0x3f, 0xf8, 0xff, 0xf0, 0x00,
  0x00, 0x27, 0xfc, 0xf7, 0xfe, 0x27, 0xfc, 0x3f, 0xc8, 0xff, 0x90, 0x00, 0x00, 0x3f, 0xff, 0xbf,
  0xfe, 0x77, 0xfc, 0x3f, 0xf8, 0xff, 0xb0, 0x00, 0x00, 0x1f, 0xff, 0x9f, 0xff, 0xff, 0xfc, 0x3f,
  0xf0, 0xff, 0xe0, 0x00, 0x00, 0x00, 0xf9, 0x08, 0xff, 0xc1, 0xe4, 0x0f, 0x20, 0x1e, 0x20, 0x00,
  0x00, 0x00, 0xf9, 0x08, 0xff, 0xc1, 0xe4, 0x0f, 0xa0, 0x1e, 0x20, 0x00, 0x00, 0x00, 0xf9, 0x08,
  0xff, 0x81, 0xe4, 0x7f, 0xbf, 0xfe, 0x20, 0x00, 0x00, 0x00, 0xf9, 0x08, 0xff, 0x01, 0xe4, 0x7f,
  0x80, 0x3c, 0x20, 0x00, 0x00, 0x00, 0xf9, 0x08, 0xff, 0x81, 0xe4, 0x3f, 0xc0, 0x3c, 0x20, 0x00,
  0x00, 0x00, 0xf9, 0x08, 0xf7, 0xc1, 0xe4, 0x3f, 0xc0, 0x7c, 0x20, 0x00, 0x00, 0x02, 0xf9, 0x08,
  0xf7, 0xc1, 0xe4, 0x1f, 0xc0, 0x78, 0x20, 0x00, 0x00, 0x07, 0xff, 0xff, 0xf3, 0xe1, 0xe4, 0x0f,
  0xe0, 0x78, 0x20, 0x00, 0x00, 0x07, 0xf0, 0x00, 0xf1, 0xe1, 0xe4, 0x01, 0xe0, 0xf8, 0x20, 0x00,
  0x00, 0x03, 0xf0, 0x00, 0xf1, 0xf1, 0xe4, 0x01, 0xf0, 0xf0, 0x20, 0x00, 0x00, 0x00, 0xf0, 0x00,
  0xf3, 0xf9, 0xe4, 0x00, 0xf0, 0xf0, 0x20, 0x00, 0x00, 0x00, 0xf0, 0x00, 0xf6, 0x79, 0xe4, 0x00,
  0xf1, 0xe0, 0x20, 0x00, 0x00, 0x00, 0xf0, 0x00, 0xf4, 0x7d, 0xe4, 0x00, 0xf9, 0xe0, 0x20, 0x00,
  0x00, 0x00, 0xf0, 0x00, 0xf4, 0x3f, 0xe4, 0x00, 0x7b, 0xe0, 0x20, 0x00, 0x00, 0x00, 0xf0, 0x00,
  0xf4, 0x3f, 0xe4, 0x00, 0x7b, 0xc0, 0x20, 0x00, 0x00, 0x00, 0xff, 0xff, 0xf4, 0x1f, 0xe7, 0xff,
  0xff, 0xc0, 0x20, 0x00, 0x00, 0x00, 0xf0, 0x00, 0xf4, 0x0f, 0xe0, 0x00, 0x3f, 0xc0, 0x20, 0x00,
  0x00, 0x00, 0xf7, 0xfe, 0xf4, 0x0f, 0xe0, 0x00, 0x3f, 0x80, 0x20, 0x00, 0x00, 0x00, 0xf4, 0x02,
  0xf4, 0x07, 0xe0, 0x00, 0x3f, 0x80, 0x20, 0x00, 0x00, 0x03, 0xfe, 0x02, 0xf4, 0x03, 0xff, 0xff,
  0xff, 0x80, 0x20, 0x00, 0x00, 0x07, 0xfe, 0x07, 0xfe, 0x03, 0xe0, 0x00, 0x1f, 0x00, 0x20, 0x00,
  0x00, 0x07, 0xfe, 0x07, 0xfe, 0x01, 0xe0, 0x00, 0x1f, 0xff, 0xe0, 0x00, 0x00, 0x07, 0xfe, 0x07,
  0xfe, 0x01, 0xe0, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x00, 0x60, 0x00,
  0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};*/

const unsigned char logo_bmp [] PROGMEM = {
  // 'ventmon 8bit logo, 128x64px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x04, 0xff, 0xf0, 0x3f, 0xf0, 0xff, 0xdf, 0xff, 0x80, 0x7c, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x04, 0xff, 0xfe, 0x3f, 0xf0, 0xff, 0xdf, 0xff, 0xe0, 0xfc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0x3f, 0xf0, 0xff, 0xdf, 0xff, 0xf8, 0xdc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x1f, 0x87, 0x00, 0x0e, 0x03, 0x80, 0x7d, 0xbc, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x03, 0xc7, 0x00, 0x0e, 0x03, 0x80, 0x1d, 0xb8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x01, 0xc7, 0x00, 0x0e, 0x03, 0x80, 0x1e, 0x78, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0xe7, 0x00, 0x0e, 0x03, 0x80, 0x0e, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x40, 0xe7, 0x10, 0x0e, 0x03, 0x80, 0x0f, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x39, 0xc0, 0xf7, 0x30, 0x0e, 0x03, 0x80, 0x0f, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x08, 0x3b, 0x40, 0xf7, 0x50, 0x0e, 0x03, 0x80, 0x1e, 0x08, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0e, 0x3a, 0xc0, 0xf7, 0xb0, 0x0e, 0x03, 0x80, 0x3c, 0x18, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0b, 0x3e, 0xc0, 0xe7, 0xf0, 0x0e, 0x03, 0xff, 0xfe, 0x78, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0d, 0xbd, 0xc0, 0xe7, 0xe0, 0x0e, 0x03, 0xff, 0xff, 0x50, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x0d, 0xbf, 0x81, 0xe7, 0x80, 0x0e, 0x03, 0xff, 0xff, 0xb0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x06, 0xbf, 0x01, 0xc7, 0x00, 0x0e, 0x03, 0x80, 0x0f, 0xe0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x07, 0xfc, 0x07, 0x87, 0x00, 0x0f, 0xf3, 0x80, 0x07, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0x07, 0x00, 0x0e, 0x0b, 0x80, 0x03, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x07, 0x00, 0x0f, 0xe7, 0x80, 0x03, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x07, 0x00, 0x0e, 0x13, 0x80, 0x03, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x07, 0xff, 0xfe, 0x0b, 0x80, 0x03, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x07, 0x00, 0x0e, 0x07, 0x80, 0x03, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x03, 0x80, 0x1c, 0x03, 0x80, 0x07, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x03, 0xc0, 0x3c, 0x03, 0x80, 0x0f, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0x38, 0x00, 0x01, 0xf0, 0xf8, 0x33, 0x80, 0x3e, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x04, 0xff, 0x00, 0x00, 0xff, 0xf0, 0x4f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x04, 0xff, 0x00, 0x00, 0x7f, 0xe0, 0x4f, 0xff, 0xfb, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 0x0f, 0x00, 0x3f, 0xff, 0xc1, 0x80, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0xc4, 0x00, 0x00, 0x08, 0x00, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x01, 0x44, 0x00, 0x00, 0x08, 0x00, 0x60, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0xff, 0xe3, 0xfe, 0xcf, 0xff, 0x0f, 0xfc, 0x3f, 0xf8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x04, 0xff, 0xe4, 0xfd, 0xd3, 0xff, 0x0f, 0xe4, 0x3f, 0xe4, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x04, 0xff, 0xe4, 0xff, 0xd3, 0xff, 0x0f, 0xe4, 0x3f, 0xe4, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x03, 0x1d, 0x43, 0x1f, 0x8c, 0x39, 0x03, 0xdc, 0x07, 0x98, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1d, 0x42, 0x1f, 0x00, 0x39, 0x01, 0xc8, 0x07, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1d, 0x42, 0x1f, 0x80, 0x39, 0x01, 0xc8, 0x07, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1d, 0x42, 0x1f, 0x80, 0x39, 0x19, 0xef, 0xff, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x1d, 0x42, 0x1f, 0xc0, 0x39, 0x14, 0xe0, 0x0f, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xdd, 0x42, 0x1d, 0xe0, 0x39, 0x1b, 0xe0, 0x0e, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x7d, 0x7e, 0x1c, 0xe0, 0x39, 0x1d, 0xf0, 0x0e, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0xbd, 0x00, 0x1c, 0xf0, 0x39, 0x0f, 0xf0, 0x1c, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xfd, 0xff, 0xfc, 0x70, 0x39, 0x0f, 0xf0, 0x1c, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x1c, 0x78, 0x39, 0x03, 0xf8, 0x1c, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x1c, 0x3c, 0x39, 0x00, 0x78, 0x38, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x1d, 0xfc, 0x39, 0x00, 0x38, 0x38, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x1d, 0x1e, 0x39, 0x00, 0x3c, 0x38, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x1d, 0x0e, 0x39, 0x00, 0x1c, 0x70, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x1d, 0x0f, 0x39, 0x00, 0x1c, 0x70, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x1d, 0x07, 0xb9, 0x00, 0x1e, 0x70, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xfd, 0x03, 0xf9, 0xff, 0xfe, 0xe0, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x1d, 0x03, 0xf8, 0x00, 0x0f, 0xe0, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1d, 0xff, 0xdd, 0x01, 0xf8, 0x00, 0x07, 0xc0, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x5d, 0x00, 0xf8, 0x00, 0x07, 0xc0, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x5d, 0x00, 0xff, 0xff, 0xff, 0xc0, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0xff, 0x80, 0x78, 0x00, 0x07, 0x80, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0xff, 0x80, 0x38, 0x00, 0x03, 0xff, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0xff, 0x80, 0x38, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// You will need to change the ESP32 partition scheme to fit the Bluetooth Libraries!
// Tools > Partition Scheme > Minimal SPIFFS (Large APPS with OTA)
// This gives you more program space - choose No OTA for even more space but this removes Over The Air updates feature.
//#define BLE
#ifdef BLE
  #include <BLEDevice.h>
  #include <BLEServer.h>
  #include <BLEUtils.h>
  #include <BLE2902.h>

  BLECharacteristic *characteristicTX;
  bool deviceConnected = false;
  // See the following for generating UUIDs:
  // https://www.uuidgenerator.net/
  #define SERVICE_UUID           "ab0828b1-198e-4351-b779-901fa0e0371e" // UART service UUID
  #define CHARACTERISTIC_UUID_RX "4ac8a682-9736-4e5d-932b-e9b31405049c"
  #define CHARACTERISTIC_UUID_TX "0972EF8C-7613-4075-AD52-756F33D4DA91"
#endif

// #define V4 // V0.4 VentMon with the new 128x64 OLED Screen
//#define DEBUG_1

/* In theory, some of these components are optional, so we track what we find... */
bool found_display = false;

/*  These are all the supported sensor. Each sensor has a "period".
 *  The Period is the number of milliseconds to wait before the next event for that sensor.
 *  This is complicated somewhat by multi-sensor boards.
 */
bool found_bme280[2] = { false, false};
bool found_bme680[2] = { false, false};
bool found_O2 = false;
bool found_flow = false;
bool found_diff_press = false;

unsigned PERIOD_O2 = 4000;
unsigned PERIOD_FLOW = 0;
unsigned PERIOD_I_DPRES = 0; // differential pressure period
unsigned PERIOD_I_ADPRES = 1000; // differential pressure produced by absolute difference, if avaiable
unsigned PERIOD_I_PRES = 10000; // Absolute pressure in the inspiratory limb
unsigned PERIOD_I_TEMP = 10000; // Temperature in the insipiratory limb
unsigned PERIOD_I_H2O = 10000; // Relative humidity in the inspiratory limb
unsigned PERIOD_I_GAS = 20000; // Gas (Ohms) (only avaialable on BME680)
unsigned PERIOD_B_PRES = 10000; // Absolute pressure in the inspiratory limb
unsigned PERIOD_B_TEMP = 10000; // Temperature in the insipiratory limb
unsigned PERIOD_B_H2O = 10000; // Relative humidity in the inspiratory limb
unsigned PERIOD_B_GAS = 20000; // Gas (Ohms) (only avaialable on BME680)

/* These are the times of the last output */
unsigned PERIOD_O2_ms = 0;
unsigned PERIOD_FLOW_ms = 0;
unsigned PERIOD_I_DPRES_ms = 0;
unsigned PERIOD_I_ADPRES_ms = 0;
unsigned PERIOD_BDPRES_ms = 0;
unsigned PERIOD_I_PRES_ms = 0;
unsigned PERIOD_I_TEMP_ms = 0;
unsigned PERIOD_I_H2O_ms = 0;
unsigned PERIOD_I_GAS_ms = 0;
unsigned PERIOD_B_PRES_ms = 0;
unsigned PERIOD_B_TEMP_ms = 0;
unsigned PERIOD_B_H2O_ms = 0;
unsigned PERIOD_B_GAS_ms = 0;

/* DISPLAY ********************************************/
#ifdef V5
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);
//#define BUTTON_A 15
//#define BUTTON_B 32
#define BUTTON_C 14


//Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire); // SH1107 
#define DISPLAY_I2C 0x3D
#elif V4
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
#define DISPLAY_I2C 0x3D
#else
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
#define DISPLAY_I2C 0x3C
#endif


unsigned long buttonCpress_ms = 0;

#define BUTTON_A 13
#define BUTTON_B 12
//#define BUTTON_C 14

/* HSC *************************************************/
// This is for the HSCDRRT001PD2A3, if you change the Honeywell differential
// pressure sensor, you will have to change this. Thanks to: https://github.com/rodan/honeywell_hsc_ssc_i2c
// for inspiration.
#define HSC_ADDR 0x28
const long unsigned MAX_RANGE = ((1 << 14) -1);
const float SENSOR_MIN = MAX_RANGE * 0.1;
const float SENSOR_MAX = MAX_RANGE * 0.9;
const uint16_t  OUTPUT_MIN_U16 = ((uint16_t) SENSOR_MIN);
const uint16_t OUTPUT_MAX_U16 = ((uint16_t) SENSOR_MAX);
#define PRESSURE_MIN -6894.76  // -1psi  in pa
#define PRESSURE_MAX 6894.76   // 1psi in pa


/* NETWORKING ******************************************/

byte mac[6];
char macs[18];

#define PARAMHOST "ventmon.coslabs.com"
#define PARAMPORT 6111
#define LOCALPORT 6111

// #define DEFAULT_HOST_ADDR "13.58.243.230"
#define DEFAULT_HOST_ADDR "52.23.196.81"

char *Loghost = strdup(PARAMHOST);
uint16_t Logport = PARAMPORT;
IPAddress LoghostAddr;
// IPAddress DefaultLogHostAddr(13,58,243,230);
IPAddress DefaultLogHostAddr(52,23,196,81);

UDP* udpclient;
EthernetUDP eudpclient;
WiFiUDP wudpclient;

// These are configurable through the serial port
int ethernet_enabled;
int wifi_enabled;

bool eudpclient_good = false;
bool wudpclient_good = false;


/* PRESSURE *******************************************/

#define SEALEVELPRESSURE_HPA (1013.25)

// It would be nice to support more than one,
// but the BME680 only has two addresses (0x77, and 0x76 when SD0 tied to GND).

Adafruit_BME280 bme280[2]; // I2C
Adafruit_BME680 bme680[2]; // I2C

// !!! Unless physical hardware changes the ambient sensor should have an address of
// 0x77 while the inspiratory limb sensor should have an address of 0x76 on the I2C bus.
// Do not change this unless directed to do so. !!!
// If you plug an additional ambient sensor in, you must ground one of the pins to make it us 0x77
// sensor addresses
#define AMBIENT_SENSOR_ADDRESS  0x77
#define INSPIRATORY_SENSOR_ADDRESS   0x76

#define INSPIRATORY_PRESSURE_SENSOR  0
#define AMBIENT_PRESSURE_SENSOR 1

uint8_t addr[2] = {INSPIRATORY_SENSOR_ADDRESS, AMBIENT_SENSOR_ADDRESS};

// We will just this as a pressure to display to make the OLED useful...
// Eventually we will put this into running window
signed long display_max_pressure = 0;
signed long display_min_pressure = 0;
signed long display_pressure_val;
signed long display_fiO2_val;
signed long display_flow_val;

// Only need to sample the ambient air occasinally
// (say once a minute) for PEEP analysis
int ambient_counter = 0;

// unsigned long sample_millis = 0;

// We could send out only raw data, and let more powerful computers process things.
// But we have a powerful micro controller here, so we will try to be useful!
// Instead of outputting only the absolute pressure, we will output the differential
// pressure in the inspiratory limb. We will compute a differential pressure against the
// ambient air. We will name this D0. We need a moving window to make sure there
// is not jitter.
int amb_wc = 0;
#define AMB_WINDOW_SIZE 4
#define AMB_SAMPLES_PER_WINDOW_ELEMENT 200
// sea level starting pressure.
signed long ambient_window[AMB_WINDOW_SIZE];

signed long smooth_ambient = 0;

/* FLOW ***********************************************/

SFM3X00 flowSensor(0x40);

// Note: The SFM3300-D is compatible with the SFM3200- you should use that for it!
#define PIRDS_SENSIRION_SFM3200 0
#define PIRDS_SENSIRION_SFM3400 1

// int sensirion_sensor_type = PIRDS_SENSIRION_SFM3200;
int sensirion_sensor_type = PIRDS_SENSIRION_SFM3400;
// At present we have to install the SENSIRION_SFM3400 backwards
// because of the physical mounting!!!
bool SENSOR_INSTALLED_BACKWARD = false;

/* FiO2 ***********************************************/

// Oxygen sensor is connected to channel 3 of ADS115
#define O2CHANNEL        3
#define UNPLUGGED_MAX    400
#define FIO2_SAMPLE_RATE 4000

Adafruit_ADS1115 ads;

int initialO2 = 0;
// bool oxygenSensing = true;
unsigned long lastFiO2Sample = 0;

/* ERROR MESSAGE STRINGS ******************************/

const char* flow_too_high = FLOW_TOO_HIGH;
const char* flow_too_low = FLOW_TOO_LOW;
const char* INSPIRATORY_PRESSURE_SENSOR_ERROR = "Inspiratory pressure sensor error";
const char* AMBIENT_PRESSURE_SENSOR_ERROR = "Ambient pressure sensor error";

/******************************************************/
bool ethernet_setup();
void setupOLED();
signed long readPressureOnly(int idx);
void init_ambient(signed long v);

struct hsc_data {
    uint8_t status;             // 2 bits
    uint16_t pressure_data;       // 14 bits
    uint16_t temperature_data;  // 11 bits
};

uint8_t load_hsc_data(const uint8_t addr, struct hsc_data *data)
{
    uint8_t i;
    uint8_t val[4];
    Wire.requestFrom(addr, (uint8_t) 4);
    for (i = 0; i <= 3; i++) {
 //     delay(4);
        val[i] = Wire.read();
    }
    data->status = (val[0] & 0xc0) >> 6;  // first 2 bits from first byte
    data->pressure_data = ((val[0] & 0x3f) << 8) + val[1];
    data->temperature_data = ((val[2] << 8) + (val[3] & 0xe0)) >> 5;
    if ( data->temperature_data == 65535 ) return 4;
    return data->status;
}

float get_HSC_pressure(const struct hsc_data raw,
                   const uint16_t output_min, const uint16_t output_max,
                   const float pressure_min,
                   const float pressure_max) {
    return  ((raw.pressure_data - output_min) * (pressure_max - pressure_min) / (output_max - output_min)) + pressure_min;
}

float get_HSC_temperature(const struct hsc_data raw) {
    return (raw.temperature_data * 0.0977) - 50;
}

#ifdef DEBUG_1
void report_bme280_not_found(uint16_t id) {
   Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
   Serial.print("SensorID was: 0x");
   Serial.println(id,16);
   Serial.print(F("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n"));
   Serial.print(F("   ID of 0x56-0x58 represents a BMP 280,\n"));
   Serial.print(F("        ID of 0x60 represents a BME 280.\n"));
   Serial.print(F("        ID of 0x61 represents a BME 680.\n"));
}
#endif

long readHSCPressure()
{
  struct hsc_data ps;

  uint8_t status = load_hsc_data(HSC_ADDR, &ps);

  float p;
  if ( status == (uint8_t) -1 ) {
    Serial.println("err sensor missing");
    return LONG_MIN;
  } else {
    if ( status == 3 ) {
       Serial.print(F("err diagnostic fault "));
       Serial.println(ps.status, BIN);
       return LONG_MIN;
     }
     if ( status == 2 ) {
       // if data has already been feched since the last
       // measurement cycle
       Serial.print(F("warn stale data "));
       Serial.println(ps.status, BIN);
     }
     if ( status == 1 ) {
       // chip in command mode
       // no clue how to end up here
       Serial.print("warn command mode ");
       Serial.println(ps.status, BIN);
      }
      p = get_HSC_pressure(ps, OUTPUT_MIN_U16, OUTPUT_MAX_U16, PRESSURE_MIN, PRESSURE_MAX);
      // convert to 10th of a cm H2O
      p = p *0.0101972 * 10;
  }
  return (long) p;
}

// true if we have ANY such sensor...
bool found_bme(int idx) {
  return found_bme280[idx] || found_bme680[idx];
}


/******************************************************/

/* DATA TRANSFER ***************************************/

void output_on_serial_print_PIRDS(char e, char t, char loc, unsigned short int n, unsigned long ms, signed long val) {
  Measurement ma = get_measurement(e, t, loc, n, ms, val);
  // I need a proof that this buffer is larger enough, but I think it is...
  char buff[256];
  fill_JSON_buffer_measurement(&ma, buff, 256);
  Serial.print(buff);
}

void outputMeasurement(char e, char t, char loc, unsigned short int n, unsigned long ms, signed long val) {
  output_on_serial_print_PIRDS(e, t, loc, n, ms, val);
  Serial.println();
  send_data(e, t, loc, n, ms, val);
  display_print_pirds(e, t, loc, n, ms, val);
}

void outputMetaEvent(const char *msg, unsigned long ms) {
  Message m = get_message(ms, (char *) msg);
  char buff[264];
  fill_JSON_buffer_message(&m, buff, 264);
  Serial.print(buff);
  Serial.println();
  send_data_message(m);

}

void perform_read(int idx) {
   if (found_bme680[idx]) {
      if (! bme680[idx].performReading()) {
        Serial.println("Failed to perform reading :( for:");
        Serial.println(addr[idx], HEX);
        found_bme680[idx] = false;
        return;
      }
  } else if (found_bme280[idx]) {
// The 280 does not requre a call to .performReading();
  } else {
    Serial.println(F("INTERNAL ERROR! (report full)"));
  }
}

void debugPrintBuffer(uint8_t* m, int n) {
  for (int i = 0; i < n; i++) {
    Serial.print(m[i], HEX);
    Serial.print(F(' '));
  }
  Serial.println();
}

Measurement get_measurement(char event, char mtype, char loc, uint8_t num, uint32_t ms, int32_t value) {
  Measurement ma;
  ma.event = event;
  ma.type = mtype;
  ma.loc = loc;
  ma.num = 0 + num;
  ma.ms = ms;
  ma.val = value;
  return ma;
}

Message get_message(uint32_t ms, char *msg) {
  Message m;
  m.event = 'E';
  m.type = 'M';
  m.ms = ms;
  int n = strlen(msg);
  if (n > 255) {
    m.b_size = 255;
    memcpy(m.buff, msg, 255);
  } else {
    m.b_size = n;
    strcpy(m.buff, msg);
  }
  return m;
}

bool send_data_measurement(Measurement ma) {
  if (eudpclient_good || wudpclient_good) {
    unsigned char m[14];

    fill_byte_buffer_measurement(&ma, m, 13);

    m[13] = '\n';

    if (udpclient->beginPacket(LoghostAddr, Logport) != 1) {
      Serial.println("wudpclient");
      Serial.println("send_data_measurement begin failed at begin!");
      return false;
    } else {
//      Serial.println("Packet begun");
    }
    if (udpclient->write(m, 14) != 14) {
      #ifdef DEBUG_1
      Serial.println("Packet Write failed");
      #endif
    } else {
 //     Serial.println("Packet Write succeeded");
    }
    if (udpclient->endPacket() != 1) {
      #ifdef DEBUG_1
      Serial.println(F("wudpclient"));
      Serial.println(F("send_data_measurement end failed!"));
      #endif
      return false;
    } else {
 //     Serial.println("Packet done");
    }
  }
  return true;
}

bool send_data(char event, char mtype, char loc, uint8_t num, uint32_t ms, int32_t value) {
  Measurement ma = get_measurement(event, mtype, loc, num, ms, value);
  return send_data_measurement(ma);
}

bool send_data_message(Message ma) {
  if (eudpclient_good || wudpclient_good) {
    unsigned char m[264];

    fill_byte_buffer_message(&ma, m, 264);
    // I don't know how to compute this byte.
    m[263] = '\n';

    if (udpclient->beginPacket(LoghostAddr, Logport) != 1) {
//    Serial.println("send_data_message begin failed!");
      return false;
    }
    if (udpclient->write(m, 264) != 264) {
      #ifdef DEBUG_1
      Serial.println(F("Packet Write failed"));
      #endif
    } else {
 //     Serial.println("Packet Write succeeded");
    }
    if (udpclient->endPacket() != 1) {
      #ifdef DEBUG_1
      Serial.println(F("send_data_message end failed!"));
      #endif
      return false;
    }
  }
  return true;
}

/******************************************************/

/* FiO2 ***********************************************/

void initializeOxygenSensor()
{
  // setup ADC for oxygen sensing
  ads.setGain(GAIN_SIXTEEN);
  ads.begin();
  initialO2 = avgADC(O2CHANNEL);
//  Serial.print("intialO2: ");
//  Serial.println(initialO2);
 // found_O2 = (initialO2 <= UNPLUGGED_MAX && initialO2 >= 0);
 found_O2 = (initialO2 <= UNPLUGGED_MAX);
}

const int NUM_TO_AVERAGE = 5;
double avgADC(int adcNumber)
{
  double adc = 0;
  double average;

  for (int i = 0; i < NUM_TO_AVERAGE; i++)
  {
    int16_t adsread = ads.readADC_SingleEnded(adcNumber);
//    Serial.print("adsread :");
//    Serial.println(adsread);
    adc = adc + adsread;
    delay(10);
  }
  average = adc / NUM_TO_AVERAGE;
  return average;
}

/******************************************************/

/* FLOW ***********************************************/


/******************************************************/

/* PRESSURE *******************************************/

void seekBME680(int idx) {
//  uint8_t loc_addr = addr[idx];
  // I don't understand why this API does not work, it seemed to work in the previous version....
//  found_bme[idx] = bme[idx].begin(loc_addr, true);
   found_bme680[idx] = bme680[idx].begin(addr[idx]);
  if (!found_bme680[idx]) {
//    Serial.println("Could not find a valid BME680 sensor, check wiring for:");
//    Serial.println(loc_addr, HEX);
  } else {
    // Set up oversampling and filter initialization
    if (idx == 0) {
      //    bme[idx].setTemperatureOversampling(BME680_OS_8X);
      bme680[idx].setTemperatureOversampling(BME680_OS_1X);
      bme680[idx].setHumidityOversampling(BME680_OS_1X);
      bme680[idx].setPressureOversampling(BME680_OS_1X);
      //   bme[idx].setIIRFilterSize(BME680_FILTER_SIZE_3);
      //  bme[idx].setGasHeater(320, 150); // 320*C for 150 ms
      bme680[idx].setGasHeater(0, 0); // 320*C for 150 ms
    } else if (idx == 1) {
      // bme[idx].setTemperatureOversampling(BME680_OS_8X);
      bme680[idx].setTemperatureOversampling(BME680_OS_1X);
      bme680[idx].setHumidityOversampling(BME680_OS_1X);
      bme680[idx].setPressureOversampling(BME680_OS_1X);
      //    bme[idx].setIIRFilterSize(BME680_FILTER_SIZE_3);
      //     bme[idx].setGasHeater(320, 150); // 320*C for 150 ms
      // I believe this feature is not needed or useful for this application
      bme680[idx].setGasHeater(0, 0); // 320*C for 150 ms
    }
  }
}

void init_ambient(signed long v) {
  for (int i = 0; i < AMB_WINDOW_SIZE; i++) ambient_window[i] = v;
}

signed long readPressureOnly(int idx)
{
  if (idx < 2) {
    if (found_bme280[idx]) {
      return readPressureOnly280(idx);
    } else if (found_bme680[idx]) {
      return readPressureOnly680(idx);
    } else {
      #ifdef DEBUG_1
      // Can't really do give an anwer, this is an internal error!
      Serial.println(F("NO SENSOR ERROR"));
      Serial.print(F("IDX: "));
      Serial.println(idx);
      #endif
      return LONG_MIN;
    }
  }
  else {
    // internal error! Ideally would publish and internal error event
   Serial.println(F("INTERNAL ERROR! BAD IDX"));
    return LONG_MIN;
  }

}

// the parameter idx here numbers our pressure sensors.
// The sensors may be better, for as per the PIRDS we are returning integer 10ths of a mm of H2O
signed long readPressureOnly280(int idx)
{
  if (idx < 2) {
    return (signed long) (0.5 + (bme280[idx].readPressure() / (98.0665 / 10)));
  }
  else {
    // internal error! Ideally would publish and internal error event
    #ifdef DEBUG_1
    Serial.println(F("INTERNAL ERROR! (readPressureOnly280)"));
    #endif
    return LONG_MIN;
  }
}

signed long readPressureOnly680(int idx)
{
  if (idx < 2) {
    if (! bme680[idx].performReading()) {
      Serial.println(F("Failed to perform reading :( for:"));
      Serial.println(addr[idx], HEX);
      found_bme680[idx] = false;
      return LONG_MIN;
    } else {
      // returning resorts in kPa
      // the sensor apparently gives us Pascals...

      return (signed long) (0.5 + (bme680[idx].pressure / (98.0665 / 10)));
    }
  } else {
    // internal error! Ideally would publish and internal error event
    #ifdef DEBUG_1
    Serial.println("INTERNAL ERROR! (readPressureOnly680)");
    #endif
    return LONG_MIN;
  }
}



/******************************************************/

/* DISPLAY ********************************************/

#define ROW_HEIGHT 10
#define ROW_LENGTH 128
#ifdef V4
  #define ROW_NUM 6
#else
  #define ROW_NUM 3
#endif
#define SCROLL_DELAY 500
String display_rows[ROW_NUM];
int row_index = 0;
void displayPrintScroll(String s){
  //Serial.print(s);

  display.clearDisplay();

  int a = row_index+1;
  if (a > ROW_NUM) {
    row_index = ROW_NUM;
    for (int i = 0; i < ROW_NUM-1; i++){
      display_rows[i] = display_rows[i+1];
    }
    display_rows[ROW_NUM-1] = s;

  } else {
    display_rows[row_index] = s;
  }

  row_index++;

  for (int i = 0; i < ROW_NUM; i++){
    display.setCursor(0, i*ROW_HEIGHT);
    display.print(display_rows[i]);
  }

  display.display();
  delay(SCROLL_DELAY);
}

void setupOLED() {

    // Here we initialize the OLED...
  Serial.println("OLED FeatherWing test");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
//  int ret_oled = display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C);
  //Serial.println("Return value for OLED");
  //Serial.println(ret_oled);
//  if (ret_oled != 1) { // Address 0x3C for 128x32
//    // hopefully this just means you don't have the board; we should send this as a message in the stream
//    // Serial.println("returning from OLED setup!");
//    return;
//  }
  delay(250); // wait for the OLED to power up
  found_display = display.begin(0x3C, true); // Address 0x3C default

  if (!found_display) {
    Serial.println("OLED not found.");
    return;
  }
//  return;
//  found_display = true;
  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();

  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  //Serial.println("IO test");
  display.setRotation(1);

#ifdef V4
  pinMode(BUTTON_A, INPUT);
  pinMode(BUTTON_B, INPUT);
#endif

#ifdef V5
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
#endif
  //pinMode(BUTTON_C, INPUT_PULLUP);

  // text display tests
  display.setTextSize(1);
//  display.setTextColor(SSD1306_WHITE);
  //display.setCursor(0, 0);
  //display.print("Starting up...");
  //display.setCursor(0, 0);
  //display.display(); // actually display all of the above

  // VentMon splash screen
  testdrawbitmap();
  delay(3000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(25, 20);
  display.println(F("VentMon"));
  display.setCursor(54, 40);
  display.setTextSize(1);

#ifdef V4
  display.println(F("v0.4"));
#endif
#ifdef V5
  display.println(F("v0.5"));
#endif

  display.display();

  delay(2000);
  display.clearDisplay();

  delay(1000);

  displayPrintScroll("Starting VentMon...");

}



void testdrawbitmap(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  //delay(1000);
}

/*void buttonA() {
  Serial.println("A BUTTON");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("A");
}
*/
void buttonB() {
  Serial.println("B BUTTON");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("B");
}

// This is an experimental use of this event!!!
// Save Log to file
void buttonC() {
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("C");

  unsigned long ms = millis();
  int n = strlen(SAVE_LOG_TO_FILE);
  char buffer[255];
  strcpy(buffer, SAVE_LOG_TO_FILE);
  strcpy(buffer + n, "test_file_name");
  outputMetaEvent(buffer, ms);
}

/* DISPLAY FOR OUR OLED (128 x 32) */
// Trying to make simple, I will define 3 lines..
void displayLine(int n, char* s) {
  display.setCursor(0, 10);
  display.print(s);
}

#define DISPLAY_STAT 0
#define DISPLAY_GRAPH 1
#define DISPLAY_ROTATION_MS = 5000;
int current_display_mode = 0;
long long_display_ms = 0;

// This is implementing a rotating graph
#define GRAPH_X_PIXELS 128
#define LEFT_PREFIX_AREA_X 28
#ifdef V4
#define GRAPH_Y_PIXELS 64
#else
#define GRAPH_Y_PIXELS 32
#endif
#define SAMPLE_PIXELS (GRAPH_X_PIXELS-LEFT_PREFIX_AREA_X)
uint8_t graph_samples[SAMPLE_PIXELS];

#define MAX_PRESSURE_SCALE 45.0
// push a sample on the end and shift all the others...
void push_sample(uint8_t s) {
  for(int i = 0; i < SAMPLE_PIXELS-1; i++) {
    graph_samples[i] = graph_samples[i+1];
  }
  graph_samples[SAMPLE_PIXELS-1] = s;
}


// Alternate between 2 screens every 5 seconds
void displayFromMS(long ms) {

  #ifndef V4
  current_display_mode = (ms / 5000) % 2;
  displayFromMode(current_display_mode);
  #else
  displayFromMode(DISPLAY_STAT);
  displayFromMode(DISPLAY_GRAPH);
  #endif
}

void displayFromMode(int mode) {
  switch (mode) {
    case DISPLAY_STAT:
      //displayPressure(true);
      display_stat();
      break;
    case DISPLAY_GRAPH:
      displayGraph();
      break;
  }
}


void display_stat(){
  display_pressure(display_pressure_val);
  display_flow(display_flow_val, true);
  display_oxygen(display_fiO2_val);
}

void display_pressure(long display_airway_pressure) {
  // 1 Pa =  0.010197442889221 cmH20
  display.print("cmH20: ");
  display.println(display_airway_pressure * 0.01);
}

void display_oxygen(signed long fiO2) {
  if (fiO2 > 0 && fiO2 <= 100){
    char buffer[18];
    sprintf (buffer, "FiO2: %2ld", fiO2);
    display.println(buffer);
  } else {
    display.println("error");
  }
}

void display_flow(signed long flow, bool isLPM){
  char buffer[18];
  if (isLPM){
    sprintf(buffer, "LPM: %2ld", flow/1000);
  } else {
    sprintf(buffer, "mm/min: %2ld", flow);
  }

  display.println(buffer);
}

// #define WHITE 0xFFFF
void displayGraph() {
  #ifndef V4
  display.clearDisplay();
  #endif
  char buffer[32];
  sprintf(buffer, "%.1f", MAX_PRESSURE_SCALE);
  display.print(buffer);

  display.println("cm ");
  display.println("H2O:");
  display.println("0.0");

  for(int i = 0; i < SAMPLE_PIXELS; i++) {
    display.drawPixel(i+LEFT_PREFIX_AREA_X,(GRAPH_Y_PIXELS -1) - graph_samples[i],1);
  }
  display.display();
}

void displayPressure(bool max_not_min) {
  // display.print(max_not_min ? "Max" : "Min" );
  display.print(" cm H2O: ");
  char buffer[32];
  long display_pressure = max_not_min ? display_max_pressure : display_min_pressure;
//  int pressure_sign = ( display_pressure < 0) ? -1 : 1;
//  int abs_pressure = (pressure_sign == -1) ? - display_pressure : display_pressure;
  sprintf(buffer, "%ld.1", display_pressure / 10);
  display.println(buffer);
}

void display_print_pirds(char e, char t, char loc, unsigned short int n, unsigned long ms, signed long val) {
  switch (t) {
    case 'D': //pressure
      display_pressure_val = val;
      break;
    case 'O': //oxygen
      display_fiO2_val = val;
    case 'F': //flow
      display_flow_val = val;
      break;
  }
}


/******************************************************/

/* NETWORKING ******************************************/

bool wifi_setup() {
  char ssid[33];
  char password[121];
  getSSIDFromEEPROM(ssid);
  getPasswordFromEEPROM(password);

  // Connect to the WiFi network (see function below loop)
  return connectToWiFi(ssid, password);
}

bool connectToWiFi(char * ssid, char * pwd)
{
 // int ledState = 0;
  wudpclient_good = false;
  printLine();
  Serial.println("Connecting to WiFi network: " + String(ssid));
  displayPrintScroll("Connecting...");
  //displayPrintScroll("Hold button to skip");
  WiFi.begin(ssid, pwd);

  int NUM_RETRIES = 50;
  int n = 0;
  while (n < NUM_RETRIES && WiFi.status() != WL_CONNECTED)
  {
    /*if (digitalRead(BUTTON_A) || digitalRead(BUTTON_B)) {
      break;
    }*/
    delay(500);
    Serial.print(".");
    n++;
  }
  if (WiFi.status() != WL_CONNECTED) {
     Serial.println("WiFi connection failed!");
     displayPrintScroll("WiFi failed!");
  } else {
    //Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    displayPrintScroll("WiFi connected!");
    displayPrintScroll((String)WiFi.localIP());

    wudpclient_good = (wudpclient.begin(LOCALPORT) == 1);

    Serial.print("WIFI UDP Connection:");
    Serial.println(wudpclient_good ? "GOOD" : "BAD");
    LoghostAddr = DefaultLogHostAddr;
  }
  return wudpclient_good;
}

void requestURL(const char * host, uint8_t port)
{
  printLine();
  Serial.println("Connecting to domain: " + String(host));

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port))
  {
    Serial.println("connection failed");
    return;
  }
  Serial.println("Connected!");
  printLine();

  // This will send the request to the server
  client.print((String)"GET / HTTP/1.1\r\n" +
               "Host: " + String(host) + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available())
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  //Serial.println();
  Serial.println("closing connection");
  client.stop();
}

void printLine()
{
  Serial.println();
  for (int i=0; i<30; i++)
    Serial.print("-");
  Serial.println();
}

bool ethernet_setup() {
  Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  WiFi.macAddress(mac); // Get MAC address of wifi chip for ethernet address
  snprintf(macs, sizeof macs, "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  #ifdef DEBUG_1
  Serial.print(F("Mac: "));
  Serial.print(macs);
  Serial.println();
  #endif

  displayPrintScroll(macs);

  eudpclient_good = false;
  int n = 0;
  const int ETHERNET_TRIES = 1;
  while (n < ETHERNET_TRIES) {
    // start the Ethernet connection:
    displayPrintScroll("Ethernet initializing");
    Serial.println(F("Initialize Ethernet with DHCP:"));

    // Check for Ethernet hardware present
    /*if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
      displayPrintScroll("Ethernet shield not found!");
      return false;
    }*/

    if (Ethernet.linkStatus() == LinkOFF) {
        #ifdef DEBUG_1
        Serial.println(F("Ethernet cable is not connected."));
        #endif
        displayPrintScroll("Cable not connected!");
        delay(1000);
        return false;
    }

    if (Ethernet.begin(mac) == 0) {
        #ifdef DEBUG_1
        Serial.println(F("Failed to configure Ethernet using DHCP"));
        #endif
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
          #ifdef DEBUG_1
          Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
          #endif
          displayPrintScroll("No ethernet shield!");
        }
        /*if (Ethernet.linkStatus() == LinkOFF) {
          Serial.println(F("Ethernet cable is not connected."));
          displayPrintScroll("Cable not connected!");
          delay(3000);
        }*/
    } else {
      #ifdef DEBUG_1
      Serial.print(F("  DHCP assigned IP "));
      Serial.println(Ethernet.localIP());
      #endif
      displayPrintScroll((String)Ethernet.localIP());
      delay(5000); // does this delay need to be this long?
      break;
    }

    Serial.print("FAILED TO FIND, TRY #");
    Serial.println(n);
    n++;
  }
  if (n==ETHERNET_TRIES) {
    #ifdef DEBUG_1
    Serial.println("FAILED TO FIND ETHERNET, GIVING UP!");
    #endif
    displayPrintScroll("Connection FAILED!");
  } else {

    // give the Ethernet shield a second to initialize:
    delay(1000);

    eudpclient_good = (eudpclient.begin(LOCALPORT) == 1);
    if (eudpclient_good) {
      DNSClient dns;
      dns.begin(Ethernet.dnsServerIP());
      if (dns.getHostByName(Loghost, LoghostAddr) == 1) {
        #ifdef DEBUG_1
        Serial.print("host is ");
        Serial.println(LoghostAddr);
        #endif
        displayPrintScroll("Ethernet host:");
        displayPrintScroll((String)LoghostAddr);
      }
    }
  }
  return eudpclient_good;
}


/******************************************************/

/* BLUETOOTH ******************************************/
#ifdef BLE
//callback
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

//callback
class CharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
      //retorna ponteiro para o registrador contendo o valor atual da caracteristica
      std::string rxValue = characteristic->getValue();
      //verifica se existe dados (tamanho maior que zero)
      if (rxValue.length() > 0) {
        Serial.print("BLE Value: ");
        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();

        // Do stuff based on the command received
        if (rxValue.find("L1") != -1) {
          Serial.print("Turning LED ON!");
          //digitalWrite(LED, HIGH);
        }
        else if (rxValue.find("L0") != -1) {
          Serial.print("Turning LED OFF!");
          //digitalWrite(LED, LOW);
        }
      }
    }
};
#endif
/******************************************************/

void output_flow() {
      // our units are slm * 1000, or milliliters per minute.
    float raw_flow = -999.0;
    raw_flow = flowSensor.readFlow();

    unsigned long ms = millis();

    float flow = (SENSOR_INSTALLED_BACKWARD) ? -raw_flow : raw_flow;
    if (flowSensor.checkRange(raw_flow)) {
      outputMetaEvent( (char *) ((flow < 0) ? flow_too_low : flow_too_high), ms);
    } else {
      signed long flow_milliliters_per_minute = (signed long) (flow * 1000);
      outputMeasurement('M', 'F', 'I', 0, ms, flow_milliliters_per_minute);
    }
}

void output_O2() {
//   unsigned long fiO2Timer = millis();
   float fiO2 = (avgADC(O2CHANNEL) / initialO2) * 20.9;
   unsigned long ms = millis();
   outputMeasurement('M', 'O', 'I', 0, ms, fiO2);
}

void output_I_DPRES() {
    unsigned long ms = millis();
          // really this should be a running max, for now it is instantaneous
    display_max_pressure = readHSCPressure();
    outputMeasurement('M', 'D', 'I', 0, ms, display_max_pressure);
    // This arbitrarily make 45 cm H20 the limit;
    uint8_t s = (GRAPH_Y_PIXELS * display_max_pressure) / MAX_PRESSURE_SCALE * 10;
    push_sample(s);
}

void output_I_ADPRES() {
    unsigned long ms = millis();
    signed long internal_pressure = readPressureOnly(INSPIRATORY_PRESSURE_SENSOR);
    if (internal_pressure != LONG_MIN) {
      // really this should be a running max, for now it is instantaneous
      long diff_pressure = internal_pressure - smooth_ambient;
      // NOTE!!! We are representing this as location "1" to distinguish from the differential pressure sensor!
      outputMeasurement('M', 'D', 'I', 1, ms, diff_pressure);
    } else {
     Serial.print(INSPIRATORY_PRESSURE_SENSOR_ERROR);
     outputMetaEvent(INSPIRATORY_PRESSURE_SENSOR_ERROR, ms);
    }
}

void output_I_PRES() {
    output_PRES(0);
}

void output_B_PRES() {
  output_PRES(1);
}

void output_PRES(int idx) {
    char loc = (idx == 0) ? 'I' : 'B';
    int sensor = (idx == 0) ? INSPIRATORY_PRESSURE_SENSOR : AMBIENT_PRESSURE_SENSOR;
    unsigned long ms = millis();
    signed long pressure = readPressureOnly(sensor);
    if (pressure != LONG_MIN) {
      outputMeasurement('M', 'P', loc, 0, ms, pressure);
    } else {
      if (idx == 0) {
        Serial.print(INSPIRATORY_PRESSURE_SENSOR_ERROR);
        outputMetaEvent(INSPIRATORY_PRESSURE_SENSOR_ERROR, ms);
      } else {
        Serial.print(AMBIENT_PRESSURE_SENSOR_ERROR);
        outputMetaEvent(AMBIENT_PRESSURE_SENSOR_ERROR, ms);
      }
    }
}

void output_temp(int idx) {
    char loc = (idx == 0) ? 'I' : 'B';
    unsigned long ms = millis();
//    found_bme280[idx];
//    found_bme680[idx];
    outputMeasurement('M', 'T', loc, 0, ms, (signed long) (0.5 +
      (found_bme280[idx] ? bme280[idx].readTemperature() : bme680[idx].readTemperature()) * 100));
}

void output_h2o(int idx) {
    char loc = (idx == 0) ? 'I' : 'B';
    unsigned long ms = millis();
    outputMeasurement('M', 'H', loc, 0, ms, (signed long) (0.5 +
      (found_bme280[idx] ? bme280[idx].readHumidity() : bme680[idx].readHumidity()) * 100));
}

void output_alt(int idx) {
    char loc = (idx == 0) ? 'I' : 'B';
    unsigned long ms = millis();
    outputMeasurement('M', 'A', loc, 0, ms, (signed long) (0.5 +
      found_bme280[idx] ?
        bme280[idx].readAltitude(SEALEVELPRESSURE_HPA) :
        bme680[idx].readAltitude(SEALEVELPRESSURE_HPA)
      ));
}

void output_gas(int idx) {
    char loc = (idx == 0) ? 'I' : 'B';
    unsigned long ms = millis();
    outputMeasurement('M', 'G', loc, 0, ms, (signed long) (0.5 + bme680[idx].gas_resistance));
}

void setSSIDIntoEEPROM(char *ssid) {
  int i = 0;
  while(i < 33 && ssid[i] != '\0') {
    EEPROM.write(i, ssid[i]);
    i++;
  }
  EEPROM.write(i, '\0');
}

void setPasswordIntoEEPROM(char *password) {
   int i = 0;
   while(i < 120 && password[i] != '\0') {
     EEPROM.write(32+i, password[i]);
     i++;
   }
   EEPROM.write(32+i, '\0');
}

void getSSIDFromEEPROM(char *buffer) {
  for(int i = 0; i < 32; i++) {
     buffer[i] = EEPROM.read(i);
  }
  // A terminator may occur before this, but this handles the extreme
  buffer[32] = '\0';
}

void getPasswordFromEEPROM(char *buffer) {
   for(int i = 0; i < 120; i++) {
     buffer[i] = EEPROM.read(32 + i);
  }
  // A terminator may occur before this, but this handles the extreme
  buffer[120] = '\0';
}

int getEthernetEnabledFromEEPROM() {
  char e = EEPROM.read(32 + 120 + 0);
  return (int) e;
}
void setEthernetEnabledFromEEPROM(char enabled) {
  EEPROM.write(32 + 120 + 0, enabled);
  EEPROM.commit();
}

int getWiFiEnabledFromEEPROM() {
  char e = EEPROM.read(32 + 120 + 1);
  return (int) e;
}
void setWiFiEnabledFromEEPROM(char enabled) {
  EEPROM.write(32 + 120 + 1, enabled);
  EEPROM.commit();
}

void printCurrentCredentials() {
  char ssid[33];
  char password[121];
  getSSIDFromEEPROM(ssid);
  getPasswordFromEEPROM(password);
  Serial.println("Current wifi credentials:");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
}

bool need_to_configure = true;
#define WAIT_TIME_S 10
void configure() {
  displayPrintScroll("Config via serial...");

  Serial.println("Enter 'c' to re-enter this configuration while running.");
  int wifiEnabled = getWiFiEnabledFromEEPROM();
  Serial.println(wifiEnabled ? "WiFi ENABLED" : "WiFi DISABLED");

  Serial.println("Enter 'w' to enable WiFi, 'x' to disable WiFi");
  int ethernetEnabled = getEthernetEnabledFromEEPROM();
  Serial.println(ethernetEnabled ? "Ethernet ENABLED" : "Ethernet DISABLED");
  Serial.println("Enter 'e' to enable Ethernet, 'f' to disable Ethernet.");
  Serial.println("Type the character 'r' to reset wifi ssid and password.");
  Serial.print("Type the character 'c' to continue or wait ");
  Serial.print(WAIT_TIME_S);
  Serial.println(" seconds to begin/resume operation:");
//  if (Serial.available() > 0) {
  Serial.setTimeout(WAIT_TIME_S*1000);
    // read the incoming byte:

  String str = Serial.readStringUntil('\n');
  char c = str.charAt(0);

  switch (c) {
    case 'r':
    {
 //     char discard = Serial.read();
      // discard the end of line..
      Serial.println("Enter SSID:");
      char ssid[33];

      String str = Serial.readStringUntil('\n');
      Serial.println("Read a string!");

      str.toCharArray(ssid, 33);
      Serial.println("ssid");
      Serial.println(ssid);

      Serial.println("Enter password:");
      char password[121];
      str = Serial.readStringUntil('\n');
      str.toCharArray(password,121);
      Serial.println("Read password");
      Serial.println(password);
      setSSIDIntoEEPROM(ssid);
      setPasswordIntoEEPROM(password);
      EEPROM.commit();
      printCurrentCredentials();
      wifi_setup();
    }
    break;
    case 'w':
      wifi_enabled = true;
      setWiFiEnabledFromEEPROM(1);
      Serial.println("WiFi Enabled");
      wifi_setup();
    break;
    case 'x':
      wifi_enabled = false;
      setWiFiEnabledFromEEPROM(0);
      Serial.println("WiFi Disabled");
      wudpclient_good = false;
    break;
    case 'e':
      setEthernetEnabledFromEEPROM(1);
      ethernet_setup();
      ethernet_enabled = true;
      Serial.println("Ethernet Enabled");
    break;
    case 'f':
      setEthernetEnabledFromEEPROM(0);
      ethernet_enabled = false;
      Serial.println("Ethernet Disabled");
      eudpclient_good = false;
    break;
      // just continue;
    case 'c':
    break;
  }
  udpclient = (UDP *) (eudpclient_good ? (UDP *) &eudpclient : (wudpclient_good ? (UDP *) &wudpclient : NULL));
  Serial.setTimeout(1000);
  Serial.println("Enter 'c' to re-enter this configuration while running.");
  need_to_configure = false;
}

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

void loop() {
  if (need_to_configure) {
    configure();
  }
  if (Serial.available() > 0) {
      // read the incoming byte:
      String str = Serial.readStringUntil('\n');
      if (str.startsWith("c")) {
        Serial.println("Configuring!");
        configure();
      }
  }

  unsigned long ms = millis();

    if (found_display) {
    if (!digitalRead(BUTTON_A)) {
      Serial.println("A BUTTON");
    }
  
    if (!digitalRead(BUTTON_B)) {
      Serial.println("B BUTTON");
    }
  
    if (!digitalRead(BUTTON_C)) {
      Serial.println("C BUTTON");
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    displayFromMS(ms);

    display.display();
    }
  /*


  if (digitalRead(BUTTON_A)) {
      Serial.println("A BUTTON");
      buttonC();
  }

  if (digitalRead(BUTTON_B)) {
    Serial.println("B BUTTON");
    buttonB();
    //Serial.println("B BUTTON PROCESS DONE");
  }

  if (found_display) {
    // experimental OLED test code
    if (digitalRead(BUTTON_A)) {
      Serial.println("A BUTTON");
      //buttonA();
    }

    if (digitalRead(BUTTON_B)) {
      Serial.println("B BUTTON");
      //buttonB();
      //Serial.println("B BUTTON PROCESS DONE");
    }
    else if (!digitalRead(BUTTON_C)) {
      unsigned long m = millis();
      if (m > (buttonCpress_ms + 1000)) {
        buttonCpress_ms = m;
        //Serial.println("B BUTTON");
        buttonC();
      }
    } else {
      display.clearDisplay();
      display.setCursor(0, 0);
      displayFromMS(ms);
    //}

    display.display();
  }
  */


  for (int i = 0; i < AMB_WINDOW_SIZE; i++) {
    smooth_ambient += ambient_window[i];
  }
  smooth_ambient = (signed long) (smooth_ambient / AMB_WINDOW_SIZE);


  // Differential, from absolute pressures
  ms = millis();
  if (found_bme(AMBIENT_PRESSURE_SENSOR) && found_bme(INSPIRATORY_PRESSURE_SENSOR)
    && ((PERIOD_I_ADPRES_ms + PERIOD_I_ADPRES)  < ms )) {
    output_I_ADPRES();
    PERIOD_I_ADPRES_ms = ms;
  }

  // Ambient pressures
  ms = millis();
  if (found_bme(AMBIENT_PRESSURE_SENSOR) && ((PERIOD_B_PRES_ms + PERIOD_B_PRES)  < ms )) {
    output_B_PRES();
    PERIOD_B_PRES_ms = ms;
  }

  // Inspiration pressures
  ms = millis();
  if (found_bme(INSPIRATORY_PRESSURE_SENSOR) && ((PERIOD_I_PRES_ms + PERIOD_I_PRES)  < ms )) {
    output_I_PRES();
    PERIOD_I_PRES_ms = ms;
  }

  // Differential Pressure
  ms = millis();
  if (found_diff_press && ((PERIOD_I_DPRES_ms + PERIOD_I_DPRES)  < ms )) {
    output_I_DPRES();
    PERIOD_I_DPRES_ms = ms;
  }

  // FiO2
  ms = millis();
  if (found_O2 && ((PERIOD_O2_ms + PERIOD_O2)  < ms )) {
    output_O2();
    PERIOD_O2_ms = ms;
  }

  // FLOW
  ms = millis();
  if (found_flow && ((PERIOD_FLOW_ms + PERIOD_FLOW)  < ms )) {
    output_flow();
    PERIOD_FLOW_ms = ms;
  }

  // Measures other than flow and pressure...
  // Some of these require a preparatory call to "peform_read", complicating the logic a bit..
  // after we perform that, we can output the ones that are demanded by the period...
  ms = millis();
  if (found_bme(INSPIRATORY_PRESSURE_SENSOR)) {
    if (((PERIOD_I_TEMP_ms + PERIOD_I_TEMP)  < ms ) ||
        ((PERIOD_I_H2O_ms + PERIOD_I_H2O)  < ms ) ||
        ((PERIOD_I_GAS_ms + PERIOD_I_GAS)  < ms ))
       {
     perform_read(INSPIRATORY_PRESSURE_SENSOR);
    }
    if ((PERIOD_I_TEMP_ms + PERIOD_I_TEMP)  < ms ) {
        output_temp(INSPIRATORY_PRESSURE_SENSOR);
        PERIOD_I_TEMP_ms = ms;
    }
    if ((PERIOD_I_H2O_ms + PERIOD_I_H2O)  < ms ) {
        output_h2o(INSPIRATORY_PRESSURE_SENSOR);
        PERIOD_I_H2O_ms = ms;
    }
    if ((PERIOD_I_GAS_ms + PERIOD_I_GAS)  < ms ) {
        output_gas(INSPIRATORY_PRESSURE_SENSOR);
        PERIOD_I_GAS_ms = ms;
    }
  }
  if (found_bme(AMBIENT_PRESSURE_SENSOR)) {
    if (((PERIOD_B_TEMP_ms + PERIOD_B_TEMP)  < ms ) ||
        ((PERIOD_B_H2O_ms + PERIOD_B_H2O)  < ms ) ||
        ((PERIOD_B_GAS_ms + PERIOD_B_GAS)  < ms ))
       {
     perform_read(AMBIENT_PRESSURE_SENSOR);
       }
    if ((PERIOD_B_TEMP_ms + PERIOD_B_TEMP)  < ms ) {
        output_temp(AMBIENT_PRESSURE_SENSOR);
        PERIOD_B_TEMP_ms = ms;
    }
    if ((PERIOD_B_H2O_ms + PERIOD_B_H2O)  < ms ) {
        output_h2o(AMBIENT_PRESSURE_SENSOR);
        PERIOD_B_H2O_ms = ms;
    }
    if ((PERIOD_B_GAS_ms + PERIOD_B_GAS)  < ms ) {
        output_gas(AMBIENT_PRESSURE_SENSOR);
        PERIOD_B_GAS_ms = ms;
    }
  }

  /* BLUETOOTH */
  #ifdef BLE
  if (deviceConnected) {
      characteristicTX->setValue("Hello!\n");
      characteristicTX->notify();
  }
  #endif
}


void setup() {
  // 32 for the ssid, 120 for the pasword, 1 for ethernet, 1 for wifi
  EEPROM.begin(32+120+1+1);

  Serial.begin(BAUD_RATE);
  Wire.begin();

    /* BLUETOOTH */
  #ifdef BLE
  // Create the BLE Device
  BLEDevice::init("VentMon");
  // Create the BLE Server
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());
  // Create the BLE Service
  BLEService *service = server->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  characteristicTX = service->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  characteristicTX->addDescriptor(new BLE2902());

  // Create a BLE Characteristic
  BLECharacteristic *characteristic = service->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  characteristic->setCallbacks(new CharacteristicCallbacks());
  // Start the service
  service->start();
  // Start advertising
  server->getAdvertising()->start();

  //Serial.println("Waiting a client connection to notify...");
  #endif

  Serial.println("----------------- VENTMON STARTING -----------------");

  flowSensor.begin();

  setupOLED();

  while (!Serial);

  displayPrintScroll("Checking sensors...");

  seekBME680(INSPIRATORY_PRESSURE_SENSOR);
  seekBME680(AMBIENT_PRESSURE_SENSOR);

    if (!bme280[0].begin(addr[0], &Wire)) {
      #ifdef DEBUG_1
      report_bme280_not_found(bme280[0].sensorID());
      #endif
    } else {
      found_bme280[0] = true;
    }

    if (!bme280[1].begin(addr[1], &Wire)) {
      #ifdef DEBUG_1
      report_bme280_not_found(bme280[1].sensorID());
      #endif
    } else {
      found_bme280[1] = true;
    }

  signed long v = readPressureOnly(AMBIENT_PRESSURE_SENSOR);
  init_ambient(v);

  initializeOxygenSensor();

  Serial.print("Ambient Pressure Sensor     : ");
  if (found_bme280[AMBIENT_PRESSURE_SENSOR]) {
    Serial.println("BME280");
  } else if (found_bme680[AMBIENT_PRESSURE_SENSOR]) {
    Serial.println("BME680");
  } else {
    Serial.println("NO");
  }
  Serial.print("Inspiratory Pressure Sensor : ");
  if (found_bme280[INSPIRATORY_PRESSURE_SENSOR]) {
    Serial.println("BME280");
  } else if (found_bme680[INSPIRATORY_PRESSURE_SENSOR]) {
    Serial.println("BME680");
  } else {
    Serial.println("NO");
  }

  Serial.print("Found O2 Sensor: ");
  Serial.println(found_O2 ? "YES" : "NO");

  float raw_flow = flowSensor.readFlow();
  // Let's wait and re-read to see if it is present...
  delay(50);
  raw_flow = flowSensor.readFlow();
  Serial.print("Found Flow Sensor: ");
  found_flow = !flowSensor.checkRange(raw_flow);
  Serial.println(found_flow ? "YES" : "NO");

  // Differential Pressure Sensor
  Serial.print("Found Differential Pressure Sensor: ");
  // Differential Pressure Sensor: err diagnostic fault 11
  found_diff_press = (readHSCPressure() != LONG_MIN);
  Serial.println(found_diff_press ? "YES" : "NO");
  Serial.println("----------------- SENSOR TEST COMPLETE -----------------");

  displayPrintScroll("Sensor test complete!");

  // TODO: Comment out for graphics testing...
  int ethernetEnabled = getEthernetEnabledFromEEPROM();
  if (ethernetEnabled) {
    Serial.println("Ethernet currently: ENABLED");
    displayPrintScroll("Ethernet ENABLED");
    ethernet_setup();
  } else {
    Serial.println("Ethernet currently: DISABLED");
    displayPrintScroll("Ethernet ENABLED");
  }
  int wifiEnabled = getWiFiEnabledFromEEPROM();
  if (wifiEnabled) {
    Serial.println("WiFi currently: ENABLED");
    displayPrintScroll("WiFi ENABLED");
    wifi_setup();
  } else {
    Serial.println("WiFi currently DISABLED");
    displayPrintScroll("WiFi DISABLED");
  }
  udpclient = (UDP *) (eudpclient_good ? (UDP *) &eudpclient : (wudpclient_good ? (UDP *) &wudpclient : NULL));

  printCurrentCredentials();
  need_to_configure = true;


}
