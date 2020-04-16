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

#define SFM3400 1
#define SFM3200 0

uint32_t requestSerialNumber();
void startContinuousMeasurement();
float readFlow(bool sensorType);

#endif 
