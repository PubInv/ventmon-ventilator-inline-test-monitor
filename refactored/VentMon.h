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

#ifndef VENTMON_H
#define VENTMON_H

#include "PIRDSEvent.h"

// maximum number of efents that can be stored at once
constexpr int maximum_events {10};

// sampling intervals
constexpr int pressure_sample_interval {5000};
constexpr int flow_sample_interval {5500};

// value for each pressure sensor
constexpr bool ambient_pressure {0};
constexpr bool internal_pressure {1};


bool setupFlowSensor(Meta* meta);
bool setupPressureSensors(Meta* meta);

void samplePressure(Measurement* measurement, bool sensor);
void sampleFlow(Measurement* measurement);

bool sendPIRDSEvents(Stream* eventChannel, PIRDSEvent** events, int eventCount);


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


