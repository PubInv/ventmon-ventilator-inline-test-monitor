/*
 * =====================================================================================
 *
 *       Filename:  SFM3X000.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/16/2020 17:00:07
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lauria Clarke (lauriacl), lauria.clarke@intel.com
 *   Organization:  NSG
 *
 * =====================================================================================
 */

#include "SFM3X00.h"

uint32_t requestSerialNumber()
{
  // transmit to device #064 (0x40)
  Wire.beginTransmission(byte(0x40)); 
  // 0x31AE is the command to read the serial number
  Wire.write(byte(0x31));    
  Wire.write(byte(0xAE));   
  Wire.endTransmission();
  // read 4 bytes from device with address 0x40
  Wire.requestFrom(0x40, 4); 

  // print serial number
  uint8_t b[4];
  uint32_t serialNumber {0};
  Wire.readBytes(b, 4);
  for (int i = 0; i < 4; i++)
  {
    serialNumber = serialNumber | ((long)b[i] << (8UL * i));
  }
  
 return serialNumber;
}


void startContinuousMeasurement()
{
  Wire.beginTransmission(byte(0x40)); 
  Wire.write(byte(0x10));      
  Wire.write(byte(0x00));     
  Wire.endTransmission();
}


float readFlow(bool sensorType)
{
  const float FM3200_SCALE = 120.0;
  const float FM3400_33_AW_SCALE = 800.0;
  float scale = (sensorType == SFM3200) ? FM3200_SCALE : FM3400_33_AW_SCALE;
  
  // Offset for the sensor
  int offset = 32768;

  // read 3 bytes from device with address 0x40
  Wire.requestFrom(0x40, 2); 
  // first received byte stored here.
  uint8_t a = Wire.read(); 
  // second received byte stored here
  uint8_t b = Wire.read(); 

  // combine the two received bytes to a 16bit integer value
  uint16_t c = (a << 8) | b;
  // remove the two least significant bits
  c >>= 2;

  float Flow = ((float)c - 8192) / scale;
  // print the calculated flow to the serial interface
  return Flow;
}

