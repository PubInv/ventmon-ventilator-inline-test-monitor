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


constexpr uint8_t sensor_address {0x40};
constexpr float   flow_offset    {32768.0};
constexpr float   flow_scale     {800.0};


void sendCommand(uint8_t address, uint16_t command);

uint16_t readData(uint8_t address);

uint32_t requestSerialNumber(uint8_t address);

uint16_t requestScaleFactor(uint8_t address);

uint16_t requestOffset(uint8_t address);

void startContinuousMeasurement(uint8_t address);

float readFlow(uint8_t address);

#endif 
