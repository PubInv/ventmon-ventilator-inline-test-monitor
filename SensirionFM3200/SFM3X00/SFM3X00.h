/*
 * =====================================================================================
 *
 *       Filename:  SFM3X000.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/16/2020 16:59:40
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lauria Clarke (lauriacl), lauria.clarke@intel.com
 *   Organization:  NSG
 *
 * =====================================================================================
 */

#ifndef SFM3X00_H
#define SFM3X00_H

#include <Wire.h>
#include <Arduino.h>

// pick which sensor you're using
#define SFM3400
// #define SFM3200

constexpr uint8_t FLOW_SENSOR_ADDRESS {0x40};
constexpr float   FLOW_OFFSET         {32768.0};

#ifdef SFM3200
constexpr float   FLOW_SCALE          {120.0};
#else
constexpr float   FLOW_SCALE          {800.0};
#endif

void sendCommand(uint8_t address, uint16_t command);

uint16_t readData(uint8_t address);

uint32_t requestSerialNumber(uint8_t address);

uint16_t requestScaleFactor(uint8_t address);

uint16_t requestOffset(uint8_t address);

void startContinuousMeasurement(uint8_t address);

float readFlow(uint8_t address);

#endif 
