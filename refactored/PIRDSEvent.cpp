/*
 * =====================================================================================
 *
 *       Filename:  PIRDSEvent.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/09/2020 14:16:09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lauria Clarke (lauriacl), lauria.clarke@gmail.com
 *   Organization:  Public Invention
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

#include "PIRDSEvent.h"

/*
PIRDSEvent PIRDSEvent::receiveEvent(Stream* eventChannel)
{
  if (eventChannel->available() > 0)
  {
    char data = eventChannel->read();
    switch (data)
    {
      // measurement event
      case 'M':
        Measurement m;
        m.readMeasurement(eventChannel);
        return m;

      // meta event
      case 'E':
        Meta e;
        e.readMeasurement(eventChannel);
        return e;

      // assertion event
      case 'A':
        Assertion a;
        a.readMeasurement(eventChannel);
        return a;
    }
  }
}
*/



void PIRDSEvent::sendEvent(Stream* eventChannel)
{

  // eventChannel->println(sizeof(this));

  unsigned char* temp = (unsigned char*)this;
  for(int i =0; i < this->eventLength; i++)
  {
    eventChannel->println(temp[i], HEX);
  }

  eventChannel->println("----------");
}


void Measurement::setValues(char measurement, char device, char location, long time, long value)
{
  measurementType = measurement;
  deviceType = device;
  deviceLocation = location;
  measurementTime = time;
  measurementValue = value;
}


void Meta::setValues(char meta, String msg)
{
  metaType = 'M';
  message = msg;
  eventLength = sizeof(msg);
}



// TODO: this function should check that the channel is open and can receive data
void Measurement::sendEvent(Stream* eventChannel)
{
  eventChannel->println("----------");
  eventChannel->println("sending MEASUREMENT event...");

  eventChannel->println(eventType, HEX);
  eventChannel->println(measurementType, HEX);
  eventChannel->println(deviceType, HEX);
  eventChannel->println(deviceLocation, HEX);
  eventChannel->println(measurementTime, HEX);
  eventChannel->println(measurementValue, HEX);

  eventChannel->println("----------");
}

void Meta::sendEvent(Stream* eventChannel)
{
  eventChannel->println("----------");
  eventChannel->println("sending META event...");
  eventChannel->println(eventType, HEX);
  eventChannel->println(message);
  eventChannel->println("----------");
}

void Assertion::sendEvent(Stream* eventChannel)
{
  eventChannel->println("----------");
  eventChannel->println("sending ASSERTION event...");
  eventChannel->println("----------");
}

void Measurement::readMeasurement(Stream* eventChannel)
{
  char measurementType[1];
  eventChannel->readBytes(measurementType, 1);
  this->measurementType = measurementType[0];

  char deviceType[1];
  eventChannel->readBytes(deviceType, 1);
  this->deviceType = deviceType[0];

  byte deviceLocation[1];
  eventChannel->readBytes(deviceLocation, 1);
  this->deviceLocation = deviceLocation[0];

  byte measurementTime[4];
  eventChannel->readBytes(measurementTime, 4);
  // place bytes into struct
  long tempTime;
  for (int i = 0; i < 4; i++)
  {
    tempTime = tempTime | ((long)measurementTime[3 - i] << 8UL * i) & (0xFFUL << 8UL * i);
  }
  this->measurementTime = tempTime;

  byte measurementValue[4];
  eventChannel->readBytes(measurementValue, 4);
  // place bytes into struct
  long tempValue;
  for (int i = 0; i < 4; i++)
  {
    tempValue = tempValue | ((long)measurementValue[3 - i] << 8UL * i) & (0xFFUL << 8UL * i);
  }
  this->measurementValue = tempValue;
}
