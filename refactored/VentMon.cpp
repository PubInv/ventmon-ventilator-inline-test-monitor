/*
 * =====================================================================================
 *
 *       Filename:  VentMon.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/13/2020 12:55:20
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lauria Clarke (lauriacl), lauria.clarke@intel.com
 *   Organization:  NSG
 *
 * =====================================================================================
 */

#include "VentMon.h"

bool setupFlowSensor(Meta* meta)
{
//   eventChannel->println("setting up flow sensor...");
   
  meta->setValues('M', "flow sensor initialized");


  // if setup is sucessful send a message, else return 1
  //if(sucessful)
  //{
    //Meta flowSucsess(25, 'M', "Flow Sensor Initialized");
    //flowSucess.sendEvent(eventChannel);
    //return 0
  //}
  //else
  //{
    //return 1;
  //}
  return 0;
}

bool setupPressureSensors(Meta* meta)
{
//   eventChannel->println("setting up pressure sensors...");

  meta->setValues('M', "pressure sensors initialized");
  
  // if setup is sucessful send a message, else return 1
  //if(sucessful)
  //{
    //Meta flowSucsess(25, 'M', "Flow Sensor Initialized");
    //flowSucess.sendEvent(eventChannel);
    //return 0
  //}
  //else
  //{
    //return 1;
  //}
  return 0;
}


void samplePressure(Measurement* measurement, bool sensor)
{

  if(sensor)
  {
    measurement->setValues('P', 'B', 0, millis(), 1111);
  }
  else
  {
    measurement->setValues('P', 'A', 0, millis(), 2222);
  }
}


void sampleFlow(Measurement* measurement)
{
  measurement->setValues('F', 'A', 4, millis(), 3333);
}

bool sendPIRDSEvents(Stream* eventChannel, PIRDSEvent** events, int eventCount)
{
  for(int i = 0; i < eventCount; i++)
  {
    events[i]->sendEvent(eventChannel);
  }

  

  //if(eventChannel is available to write)
  //{
    //send
  //}
  //else
  //{
    //return 1;
  //}

  return 0;
}
/*

bool receivePIRDSEvents(Stream* eventChannel, PIRDSEvent** events, int eventCount)
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




  //if(eventChannel is available to write)
  //{
    //send
  //}
  //else
  //{
    //return 1;
  //}

  return 0;
}
*/




// BELOW THIS POINT IS CODE THAT WAS PORTED FROM ROB'S INITIAL SKETCH
// It needs to be refactored before it is integrated back into the library
/*
void seekUnfoundBME()
{
  if (!found_bme[0])
  {
    seekBME(0);
  }
  if (!found_bme[1])
  {
    seekBME(1);
  }
}

void seekBME(int idx)
{
  found_bme[idx] = bme[idx].begin(addr[idx]);
  if (!found_bme[idx])
  {
    Serial.println("Could not find a valid BME680 sensor, check wiring for:");
    Serial.println(addr[idx], HEX);
  }
  else
  {
    // Set up oversampling and filter initialization
    if (idx == 0)
    {
      //    bme[idx].setTemperatureOversampling(BME680_OS_8X);
      bme[idx].setTemperatureOversampling(BME680_OS_1X);
      bme[idx].setHumidityOversampling(BME680_OS_1X);
      bme[idx].setPressureOversampling(BME680_OS_1X);
      //   bme[idx].setIIRFilterSize(BME680_FILTER_SIZE_3);
      //  bme[idx].setGasHeater(320, 150); // 320*C for 150 ms
      bme[idx].setGasHeater(0, 0); // 320*C for 150 ms
    }
    else if (idx == 1)
    {
      // bme[idx].setTemperatureOversampling(BME680_OS_8X);
      bme[idx].setTemperatureOversampling(BME680_OS_1X);
      bme[idx].setHumidityOversampling(BME680_OS_1X);
      bme[idx].setPressureOversampling(BME680_OS_1X);
      bme[idx].setIIRFilterSize(BME680_FILTER_SIZE_3);
      // bme[idx].setGasHeater(320, 150); // 320*C for 150 ms
      // I believe this feature is not needed or useful for this application
      bme[idx].setGasHeater(0, 0); // 320*C for 150 ms
    }
  }
}

// only need to sample the ambient air occasinally
// (say once a minute) for PEEP analysis
int ambient_counter = 0;

unsigned long sample_millis = 0;

void outputChrField(char *name, char v)
{
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : \"");
  Serial.print(v);
  Serial.print("\", ");
}
void outputNumField(char *name, signed long v)
{
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : ");
  Serial.print(v);
  Serial.print(", ");
}
void outputNumFieldNoSep(char *name, signed long v)
{
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : ");
  Serial.print(v);
  Serial.print(" ");
}

void outputByteField(char *name, unsigned short v)

{
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : \"");
  Serial.print((unsigned short int) v);
  Serial.print("\", ");
}

void outputMeasurment(char e, char t, char loc, unsigned short int n, unsigned long ms, signed long val) {
  Serial.print("{ ");
  outputChrField("event", e);
  outputChrField("type", t);
  outputNumField("ms", ms);
  outputChrField("loc", loc);
  outputByteField("num", n);
  outputNumFieldNoSep("val", val);
  Serial.print(" }");
}

// We could send out only raw data, and let more powerful computers process thing.
// But we have a powerful micro controller here, so we will try to be useful!
// Instead of outputting only the absolute pressure, we will output the differential
// pressure in the Airway. We will compute a differential pressure against the
// ambient air. We will name this D0. We need a moving window to make sure there
// is not jitter.
int amb_wc = 0;

signed long ambient_window[AMB_WINDOW_SIZE];

void init_ambient() {
  for (int i = 0; i < 4; i++) ambient_window[i] = 10340;
}

void report_full(int idx)
{
  unsigned long ms = millis();

  if (! bme[idx].performReading())
  {
    Serial.println("Failed to perform reading :( for:");
    Serial.println(addr[idx], HEX);
    found_bme[idx] = false;
    return;
  }


  char loc = (idx == 0) ? 'A' : 'B';

  outputMeasurment('M', 'T', loc, 0, ms, (signed long) (0.5 + (bme[idx].temperature * 100)));
  Serial.println();

  // This code is useful for debugging
  //  Serial.print("Data For:");
  //  Serial.println(addr[idx],HEX);
  //  Serial.print("Temperature = ");
  // Serial.print(bme[idx].temperature);
  // Serial.println(" *C");
  //
  //  Serial.print("Pressure = ");
  //  Serial.print(bme[idx].pressure / 1000.0);
  //  Serial.println(" kPa");

  outputMeasurment('M', 'P', loc, 0, ms, (signed long) (0.5 + (bme[idx].pressure / (98.0665 / 10))));
  Serial.println();
  // Serial.print("Humidity = ");
  // Serial.print(bme[idx].humidity);
  // Serial.println(" %");

  outputMeasurment('M', 'H', loc, 0, ms, (signed long) (0.5 + (bme[idx].humidity * 100)));
  Serial.println();
  //  Serial.print("Gas = "); 
  //  Serial.print(bme[idx].gas_resistance / 1000.0);
  //  Serial.println(" KOhms");
  outputMeasurment('M', 'G', loc, 0, ms, (signed long) (0.5 + bme[idx].gas_resistance));
  Serial.println();
  //  Serial.print("Approx. Altitude = ");
  //  Serial.print(bme[idx].readAltitude(SEALEVELPRESSURE_HPA));
  //  Serial.println(" m");
  outputMeasurment('M', 'A', loc, 0, ms, (signed long) (0.5 + bme[idx].readAltitude(SEALEVELPRESSURE_HPA)));

  Serial.println();
}






// the parameter idx here numbers our pressure sensors.
// The sensors may be better, for as per the PIRDS we are returning integer 10ths of a mm of H2O
signed long readPressureOnly(int idx)
{
  if (idx < 2) {
    if (! bme[idx].performReading()) {
      Serial.println("Failed to perform reading :( for:");
      Serial.println(addr[idx], HEX);
      found_bme[idx] = false;
      return -999.0;
    } else {
      // returning resorts in kPa
      // the sensor apparently gives us Pascals...

      return (signed long) (0.5 + (bme[idx].pressure / (98.0665 / 10)));
    }
  } else {
    // internal error! Ideally would publish and internal error event
  }

}


uint8_t crc8(const uint8_t data, uint8_t crc) {
  crc ^= data;

  for ( uint8_t i = 8; i; --i ) {
    crc = ( crc & 0x80 )
          ? (crc << 1) ^ 0x31
          : (crc << 1);
  }
  return crc;
}


// This routine was gotten from the Arduino forums and is informal.
// I may need to improve it; the problem I am currently having is awful.
float readSensirionFlow(int sensirion_sensor_type)
{


  // Documentation inconsistent
  int offset = 32768; // Offset for the sensor

  // NOTE: THIS IS DEPENDENT ON SENSOR!!
  // We may need to use the buttons on the OLED or some other
  // means for setting this.
  const float FM3200_SCALE = 120.0;
  const float FM3400_33_AW_SCALE = 800.0;
  float scale = (sensirion_sensor_type == SENSIRION_FM3200) ? FM3200_SCALE : FM3400_33_AW_SCALE;

  Wire.requestFrom(0x40, 3); // read 3 bytes from device with address 0x40
  uint16_t a = Wire.read(); // first received byte stored here. The variable "uint16_t" can hold 2 bytes, this will be relevant later
  uint8_t b = Wire.read(); // second received byte stored here
  uint8_t crc = Wire.read(); // crc value stored here
  uint8_t mycrc = 0xFF; // initialize crc variable
  mycrc = crc8(a, mycrc); // let first byte through CRC calculation
  mycrc = crc8(b, mycrc); // and the second byte too
  if (mycrc != crc) { // check if the calculated and the received CRC byte matches
    //  Serial.println("Error: wrong CRC");
  }
  a = (a << 8) | b; // combine the two received bytes to a 16bit integer value
  a >>= 2; // remove the two least significant bits

  //  Serial.println("combined");
  //  Serial.println(a);
  //  Serial.println(a - 8192);
  float Flow = ((float)a - 8192) / scale;
  //Serial.println(a); // print the raw data from the sensor to the serial interface
  //Serial.print("Flow in spm: ");
  //Serial.println(Flow); // print the calculated flow to the serial interface
  //Serial.println();
  return Flow;
}

// My lame-o attempt to get the flow sensor working
// https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/5_Mass_Flow_Meters/Application_Notes/Sensirion_Mass_Flo_Meters_SFM3xxx_I2C_Functional_Description.pdf
//

void initSensirionFM3200Measurement()
{
  // Trying to explicitly send an instruction byte:
  Wire.beginTransmission(0x40);
  Wire.write(byte(0x10));
  Wire.write(byte(0x00)); // sends instruction byte   
  Wire.endTransmission();     // stop transmitting

  delay(5);
  {
    Wire.requestFrom(0x40, 3); // read 3 bytes from device with address 0x40
    delay(110);
    Serial.println(Wire.available());
  
    // first received byte stored here. The variable "uint16_t" can hold 2 bytes, this will be relevant later
    uint16_t a = Wire.read();     
    uint8_t b = Wire.read();
    uint8_t c = Wire.read();
    Serial.println("a,b,c");
    Serial.println(a, HEX);
    Serial.println(b, HEX);
    Serial.println(c, HEX);
  }
}

*/


