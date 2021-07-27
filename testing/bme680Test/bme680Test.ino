/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

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

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme0; // I2C
Adafruit_BME680 bme1; // I2C

void setup()
{
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 test"));

  // SENSOR 0 - 0X77
  if (!bme0.begin(0x77, true))     ///< The default I2C address
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme0.setTemperatureOversampling(BME680_OS_8X);
  bme0.setHumidityOversampling(BME680_OS_2X);
  bme0.setPressureOversampling(BME680_OS_4X);
  bme0.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme0.setGasHeater(320, 150); // 320*C for 150 ms


  // SENSOR 1 - 0X76
  if (!bme1.begin(0x76, true))     ///< The default I2C address
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme1.setTemperatureOversampling(BME680_OS_8X);
  bme1.setHumidityOversampling(BME680_OS_2X);
  bme1.setPressureOversampling(BME680_OS_4X);
  bme1.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme1.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop()
{
  if (! bme0.performReading())
  {
    Serial.println("Failed to perform reading :(");
    return;
  }

  Serial.print("pressure 0: ");
  Serial.print(bme0.pressure / 100.0);
  Serial.println(" hPa");
  Serial.print(0.5 + (bme0.pressure / (98.0665 / 10)));
  Serial.println();
  Serial.print("temperature 0:");
  Serial.print(bme0.temperature);
  Serial.println(" *C");
  Serial.print("humidity 0: ");
  Serial.print(bme0.humidity);
  Serial.println(" %");
  Serial.print("approx. altitude 0:");
  Serial.print(bme0.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");
  Serial.print("voc 0: ");
  Serial.print(bme0.gas_resistance / 1000.0);
  Serial.println(" KOhms");
  Serial.println();

  delay(2000);


  if (! bme1.performReading())
  {
    Serial.println("Failed to perform reading :(");
    return;
  }

  Serial.print("pressure 1: ");
  Serial.print(bme1.pressure / 100.0);
  Serial.println(" hPa");
  Serial.print(0.5 + (bme1.pressure / (98.0665 / 10)));
  Serial.println();
  Serial.print("temperature 1:");
  Serial.print(bme1.temperature);
  Serial.println(" *C");
  Serial.print("humidity 1: ");
  Serial.print(bme1.humidity);
  Serial.println(" %");
  Serial.print("approx. altitude 1:");
  Serial.print(bme1.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");
  Serial.print("voc 1: ");
  Serial.print(bme1.gas_resistance / 1000.0);
  Serial.println(" KOhms");
  Serial.println();

  Serial.println();


  delay(2000);
}
