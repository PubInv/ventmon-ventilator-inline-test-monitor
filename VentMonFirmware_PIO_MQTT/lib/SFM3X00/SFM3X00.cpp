/*
 * =====================================================================================
 *
 *       Filename:  SFM3X000.cpp
 *
 *    Description:  Senserion SFM3X00 library
 *
 *        Version:  1.0
 *        Created:  04/16/2020 16:59:40
 *
 *   Organization:  Public Invention
 *
 *        License:  Sensirion BSD 3-Clause License
 *
 * =====================================================================================
 */

#include "SFM3X00.h"


void SFM3X00::sendCommand(uint16_t command)
{
  //Serial.println();
  //Serial.println(command, HEX);
  uint8_t b1 = (command & 0xFF00) >> 8;
  //Serial.println(b1, HEX);
  uint8_t b0 = command & 0x00FF;
  //Serial.println(b0, HEX);

  Wire.beginTransmission(byte(this->sensorAddress));
  Wire.write(byte(b1));
  Wire.write(byte(b0));
  Wire.endTransmission();
}


uint16_t SFM3X00::readData()
{
  uint8_t b[2];

  Wire.requestFrom(this->sensorAddress, 2);

  b[1] = Wire.read();
  b[0] = Wire.read();

  uint16_t c {0};
  c = (b[1] << 8) | b[0];

  //Serial.println(c, HEX);
  //Serial.println(c, DEC);

  return c;
}


uint32_t SFM3X00::requestSerialNumber()
{
  sendCommand(READ_SERIAL_NUMBER_U);

  uint16_t upperBytes = readData();

  sendCommand(READ_SERIAL_NUMBER_L);

  uint16_t lowerBytes = readData();

  uint32_t serialNumber {0};

  serialNumber = ((uint32_t)upperBytes << 16) | lowerBytes;

 return serialNumber;
}


uint32_t SFM3X00::requestArticleNumber()
{
  sendCommand(READ_ARTICLE_NUMBER_U);

  uint16_t upperBytes = readData();

  sendCommand(READ_ARTICLE_NUMBER_L);

  uint16_t lowerBytes = readData();

  uint32_t articleNumber {0};

  articleNumber = ((uint32_t)upperBytes << 16) | lowerBytes;

 return articleNumber;
}



uint16_t SFM3X00::requestScaleFactor()
{
  sendCommand(READ_SCALE_FACTOR);

  int16_t scaleFactor = readData();

  return scaleFactor;
}


uint16_t SFM3X00::requestOffset()
{
  sendCommand(READ_FLOW_OFFSET);

  uint16_t offset = readData();

  return offset;
}


void SFM3X00::setupFlowSensor()
{
  this->serialNumber = requestSerialNumber();
  this->articleNumber = requestArticleNumber();
  this->flowOffset   = requestOffset();
  this->flowScale    = requestScaleFactor();

  if(this-> flowScale == 800.0)
  {
    this->minFlow = SFM3400_MIN;
    this->maxFlow = SFM3400_MAX;
  }
  else if(this-> flowScale == 120.0)
  {
    this->minFlow = SFM3200_MIN;
    this->maxFlow = SFM3200_MAX;
  }
}


void SFM3X00::startContinuousMeasurement()
{
  sendCommand(START_CONTINUOUS_MEASUREMENT);
}


void SFM3X00::begin()
{
  this->setupFlowSensor();
  this->startContinuousMeasurement();
}



float SFM3X00::readFlow()
{
  uint16_t rawFlow = readData();

  float flow = ((float)rawFlow - this->flowOffset) / this->flowScale;

  return flow;
}


bool SFM3X00::checkRange(uint16_t rawFlow)
{
  return ((rawFlow <= this->minFlow) || (rawFlow >= this-> maxFlow));
}

bool SFM3X00::checkRange(float computedFlow)
{
  float min_f = ((float)this->minFlow - this->flowOffset) / this->flowScale;
  float max_f = ((float)this->maxFlow - this->flowOffset) / this->flowScale;

  return ((computedFlow <= min_f) || (computedFlow >= max_f));
}
