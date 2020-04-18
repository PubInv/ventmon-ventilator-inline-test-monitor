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

// TODO: This should be re-re-factored based on the Sensirion GitHub.
// https://github.com/Sensirion/embedded-sfm
// This is by no means the corret implementation of sensor functionality, howveer it works for now. 

void sendCommand(uint8_t address, uint16_t command)
{
  //Serial.println();
  //Serial.println(command, HEX);
  uint8_t b1 = (command & 0xFF00) >> 8;
  //Serial.println(b1, HEX);
  uint8_t b0 = command & 0x00FF;
  //Serial.println(b0, HEX);

  Wire.beginTransmission(byte(address)); 
  Wire.write(byte(b1));      
  Wire.write(byte(b0));     
  Wire.endTransmission();
}

uint16_t readData(uint8_t address)
{
  uint8_t b[2];

  Wire.requestFrom(0x40, 2); 

  b[1] = Wire.read();
  b[0] = Wire.read();

  uint16_t c {0};
  c = (b[1] << 8) | b[0];

  //Serial.println(c, HEX);
  //Serial.println(c, DEC);

  return c;
}


uint32_t requestSerialNumber(uint8_t address)
{
  sendCommand(address, 0x31AE);

  uint16_t upperBytes = readData(address);

  sendCommand(address, 0x31AF);

  uint16_t lowerBytes = readData(address);

  uint32_t serialNumber {0};
  
  serialNumber = ((uint32_t)upperBytes << 16) | lowerBytes;
  
 return serialNumber;
}



uint16_t requestScaleFactor(uint8_t address)
{
  sendCommand(address, 0x30DE);

  int16_t scaleFactor = readData(address);
  
  return scaleFactor;
}


uint16_t requestOffset(uint8_t address)
{
  sendCommand(address, 0x30DF);

  uint16_t offset = readData(address);
  
  return offset;
}


void startContinuousMeasurement(uint8_t address)
{
  sendCommand(address, 0x1000);
}


float readFlow(uint8_t address)
{
  uint16_t rawFlow = readData(address);
   
  float flow = ((float)rawFlow - flow_offset) / flow_scale;
  
  return flow;
}

