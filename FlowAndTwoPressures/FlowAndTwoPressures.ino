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

/***************************************************************************
  Hacked by Robert L. Read, 2020

  ... in order to support two breakout boards on the same Arduinol
  Modififications released under BSD to avoid complication (I prefer GPL).

 ***************************************************************************/


#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

// It would be nice to support more than one,
// but the BME680 only has two addresses (0x77, and 0x76 when SD0 tied to GND).
Adafruit_BME680 bme[2]; // I2C

bool found_bme[2] = { false, false}; // an abundance of caution to init


uint8_t addr[2] = {0x76,0x77};

//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

void setup() {
  Wire.begin();
  Serial.begin(115200);

  int a = 0;
  int b = 0;
  int c = 0; 
 
  delay(1000);

  init_ambient();
  while (!Serial);
  //Serial.println(F("BME680 test"));
  seekBME(0);
  seekBME(1);
}

void seekUnfoundBME() {
  if (!found_bme[0]) {
    seekBME(0);
  }
  if (!found_bme[1]) {
    seekBME(1);
  }
}

void seekBME(int idx) {
  found_bme[idx] = bme[idx].begin(addr[idx]);
  if (!found_bme[idx]) {
    Serial.println("Could not find a valid BME680 sensor, check wiring for:");
    Serial.println(addr[idx],HEX);
  } else {
    // Set up oversampling and filter initialization
    if (idx == 0) {
//    bme[idx].setTemperatureOversampling(BME680_OS_8X);
      bme[idx].setTemperatureOversampling(BME680_OS_1X);
      bme[idx].setHumidityOversampling(BME680_OS_1X);
      bme[idx].setPressureOversampling(BME680_OS_1X);
 //   bme[idx].setIIRFilterSize(BME680_FILTER_SIZE_3);
  //  bme[idx].setGasHeater(320, 150); // 320*C for 150 ms
    bme[idx].setGasHeater(0, 0); // 320*C for 150 ms 
    } else if (idx == 1) {
     // bme[idx].setTemperatureOversampling(BME680_OS_8X);
      bme[idx].setTemperatureOversampling(BME680_OS_1X);
      bme[idx].setHumidityOversampling(BME680_OS_1X);
      bme[idx].setPressureOversampling(BME680_OS_1X);
    bme[idx].setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme[idx].setGasHeater(320, 150); // 320*C for 150 ms
//    bme[idx].setGasHeater(0, 0); // 320*C for 150 ms  
    }
  }
}

// only need to sample the ambient air occasinally
// (say once a minute) for PEEP analysis
int ambient_counter = 0;

unsigned long sample_millis = 0;

void outputChrField(char *name,char v) {
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : \"");
  Serial.print(v);
  Serial.print("\", ");
}
void outputNumField(char *name,signed long v) {
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : \"");
  Serial.print(v);
  Serial.print("\", ");
}
void outputNumFieldNoSep(char *name,signed long v) {
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : \"");
  Serial.print(v);
  Serial.print("\" ");
}

void outputByteField(char *name,unsigned short v) {
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : \"");
  Serial.print((unsigned short int) v);
  Serial.print("\", ");
}

void outputMeasurment(char e, char t, char loc, unsigned short int n, unsigned long ms, signed long val) {
  Serial.print("{ ");
  outputChrField("event",e);
  outputChrField("type",t);
  outputNumField("ms",ms);
  outputChrField("loc",loc);
  outputByteField("num",n);
  outputNumFieldNoSep("val",val);
  Serial.print(" }");
}

// We could send out only raw data, and let more powerful computers process thing.
// But we have a powerful micro controller here, so we will try to be useful!
// Instead of outputting only the absolute pressure, we will output the differential
// pressure in the Airway. We will compute a differential pressure against the 
// ambient air. We will name this D0. We need a moving window to make sure there
// is not jitter.
int amb_wc = 0;
#define AMB_WINDOW_SIZE 4
#define AMB_SAMPLES_PER_WINDOW_ELEMENT 25 
// sea level starting pressure.
signed long ambient_window[AMB_WINDOW_SIZE];

void init_ambient() {
  for(int i = 0; i < 4; i++) ambient_window[i] = 10340;
}

void report_full(int idx) 
{
  unsigned long ms = millis(); 
   
  if (! bme[idx].performReading()) {
    Serial.println("Failed to perform reading :( for:");
    Serial.println(addr[idx],HEX);
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

void loop() {
  
  unsigned long m = millis();
  if (m > sample_millis) {
    sample_millis = m;
  } else {
    Serial.println("unticked");
    return;
  }
  seekUnfoundBME();

  unsigned long ms = millis();
  // We need to use a better sentinel...this is a legal value!
  // units for pressure are cm H2O * 100 (integer 10ths of mm)
  signed long ambient_pressure = -999; 
  signed long internal_pressure = -999;  // Inspiratory Pathway pressure
  
  if (found_bme[0]) {
    internal_pressure = readPressureOnly(0);
  }
  if (((ambient_counter % AMB_SAMPLES_PER_WINDOW_ELEMENT) == 0) && found_bme[1]) {
      ambient_pressure = readPressureOnly(1);
      ambient_counter = 1;

      // experimentally we will report everything in the stream from 
      // both sensor; sadly the BM# 680 is to slow to do this every sample.
      report_full(0);
      report_full(1);
      
      if (ambient_pressure != -999) {    
        outputMeasurment('M', 'P', 'B', 1, ms, ambient_pressure);
        ambient_window[amb_wc] = ambient_pressure;
        amb_wc = (amb_wc +1) % AMB_WINDOW_SIZE;
        Serial.println();
      } else {
        Serial.print("\"NA\"");  
      }
  } else {
   ambient_counter++;
  }
  signed long smooth_ambient = 0;
  for(int i = 0; i < AMB_WINDOW_SIZE; i++) {
    smooth_ambient += ambient_window[i];
  }
  smooth_ambient = (signed long) (smooth_ambient / AMB_WINDOW_SIZE);

  float flow = -999.0;
  flow = readFlow();

// Note: Units are kiloPasccas for pressure, and slm for flow

  if (internal_pressure != -999) {    
 //   outputMeasurment('M', 'P', 'A', 0, ms, internal_pressure);
 //   Serial.println();
    outputMeasurment('M', 'P', 'D', 0, ms, internal_pressure - smooth_ambient);  
    Serial.println();
  } else {
    // This is not actually part of the format!!!
    Serial.print("\"NA\"");  
  }



// our units are slm * 1000, or milliliters per minute.
  signed long flow_milliliters_per_minute = (signed long) (flow * 1000);

  outputMeasurment('M', 'F', 'A', 0, ms, flow_milliliters_per_minute);
  Serial.println();
  Serial.flush();
}

// the parameter idx here numbers our pressure sensors.
// The sensors may be better, for as per the PIRDS we are returning integer 10ths of a mm of H2O
signed long readPressureOnly(int idx) 
{
  if (idx < 2) {
    if (! bme[idx].performReading()) {
      Serial.println("Failed to perform reading :( for:");
      Serial.println(addr[idx],HEX);
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

float readFlow() {
// Documentation inconsistent
  int offset = 32768; // Offset for the sensor
  float scale = 120.0; // Scale factor for Air and N2 is 140.0, O2 is 142.8

  Wire.requestFrom(0x40, 3); // read 3 bytes from device with address 0x40
  uint16_t a = Wire.read(); // first received byte stored here. The variable "uint16_t" can hold 2 bytes, this will be relevant later
  uint8_t b = Wire.read(); // second received byte stored here
//  Serial.println("raw");
//  Serial.println(a);
//  Serial.println(b);
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
