/*
 * =====================================================================================
 *
 *       Filename:  PIRDSEvent.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/09/2020 14:25:55
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

#ifndef PIRDSEVENT_H
#define PIRDSEVENT_H

#include <Stream.h>
#include <Arduino.h>

class PIRDSEvent
{
  public:
    char    eventType;
    int     eventLength;

    PIRDSEvent(char type = 'M', int length = 12) :
      eventType{type}, eventLength{length}
    {
    }

    PIRDSEvent receiveEvent(Stream* eventChannel);
    virtual void sendEvent(Stream* eventChannel);
};


class Measurement: public PIRDSEvent
{
  public:
    char          measurementType;
    char          deviceType;
    unsigned char deviceLocation;
    unsigned long measurementTime;
    long          measurementValue;

    Measurement() = default;
    Measurement(char measurement, char device, char location, long time, long value) :
      measurementType {measurement},
      deviceType      {device},
      deviceLocation  {location},
      measurementTime {time},
      measurementValue{value}
      {
        eventType = 'M';
        eventLength = 12;
      }

  void setValues(char measurement, char device, char location, long time, long value);
  void readMeasurement(Stream* eventChannel);
  virtual void sendEvent(Stream* eventChannel);
};


class Assertion: public Measurement
{

  public:
    Assertion()
    {
      eventType = 'A';
      eventLength = 12;
    }

  virtual void sendEvent(Stream* eventChannel);
};


class Meta: public PIRDSEvent
{
  char metaType;
  String message;

  public:
    Meta()
    {
      eventType = 'E';
    }

    // something is wrong with this constructor
    Meta(int length) : PIRDSEvent{length}
    {
      eventType = 'E';
    }

    virtual void sendEvent(Stream* eventChannel);
    void setValues(char meta, String msg);
};
#endif
