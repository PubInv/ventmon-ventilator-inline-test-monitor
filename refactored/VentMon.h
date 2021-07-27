/*
 * =====================================================================================
 *
 *       Filename:  VentMon.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/13/2020 12:51:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lauria Clarke (lauriacl), lauria.clarke@intel.com
 *   Organization:  NSG
 *
 * =====================================================================================
 */

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

#ifndef VENTMON_H
#define VENTMON_H

#include <Wire.h>
#include <PIRDSEvent.h>
#include <SFM3X00.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <SPI.h>
#include <WiFi.h>
#include <Ethernet.h>

#define UDP

#ifdef UDP
#include <EthernetUdp.h>
#include <Dns.h>
#endif


#define CSIZE 500


#define PARAMHOST "ventmon.coslabs.com"
#ifdef UDP
#define PARAMPORT 5858
#else
#define PARAMPORT 6110
#endif

// packet struct
struct packet_t
{
  char mtype;
  char stype;
  uint8_t loc;
  uint32_t ms;
  int32_t value;
};


// default sensor address
#define BME680_AIRWAY_ADDRESS       (0x77)
// the address of the sensor with a jumper between SDO and GND will be 0x76
#define BME680_AMBIENT_ADDRESS       (0x76)


// maximum number of efents that can be stored at once
constexpr int maximum_events {10};

// sampling intervals
constexpr int pressure_sample_interval {5000};
constexpr int flow_sample_interval {500};

// value for each pressure sensor
constexpr bool ambient_pressure {0};
constexpr bool internal_pressure {1};


bool setupFlowSensor(Meta* meta);
bool setupPressureSensors(Meta* meta, Adafruit_BME680* airwayPressure, Adafruit_BME680* ambientPressure);

void samplePressure(Measurement* measurement, bool sensor);
void sampleFlow(Measurement* measurement);

bool sendPIRDSEvents(Stream* eventChannel, PIRDSEvent** events, int eventCount);

void setupEthernet(char* logHost, IPAddress* logHostAddr, byte* macAddress, char* macAddressString, EthernetUDP* udpClient);

void pushData(packet_t* packets, uint8_t* cbuf_head, Measurement* measurement);

uint8_t popData(uint8_t* cbuf_tail, uint8_t* cbuf_head, EthernetUDP* udpClient, Measurement* measurement, IPAddress* logHostAddr);

bool sendDataUDP(EthernetUDP* UDPClient, Measurement* measurement, IPAddress* logHostAddr);


// BELOW THIS POINT IS CODE THAT WAS PORTED FROM ROB'S INITIAL SKETCH
// It needs to be refactored before it is integrated back into the library
/*

void seekUnfoundBME();

void seekBME(int idx);

void outputChrField(char *name, char v);

void outputNumField(char *name, signed long v);

void outputNumFieldNoSep(char *name, signed long v);

void outputByteField(char *name, unsigned short v);

void outputMeasurment(char e, char t, char loc, unsigned short int n, unsigned long ms, signed long val);

void init_ambient();

void report_full(int idx);

signed long readPressureOnly(int idx);

uint8_t crc8(const uint8_t data, uint8_t crc);

float readSensirionFlow(int sensirion_sensor_type);

void initSensirionFM3200Measurement();
*/


#endif
