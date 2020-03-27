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

  Wire.beginTransmission(byte(0x40)); // transmit to device #064 (0x40)
  Wire.write(byte(0x10));      //
  Wire.write(byte(0x00));      //
  Wire.endTransmission(); 

  delay(5);

  Wire.requestFrom(0x40, 3); //
  a = Wire.read(); // first received byte stored here
  b = Wire.read(); // second received byte stored here
  c = Wire.read(); // third received byte stored here
  Wire.endTransmission();
  Serial.print(a);
  Serial.print(b);
  Serial.println(c);

  delay(5);
 
  Wire.requestFrom(0x40, 3); //
  a = Wire.read(); // first received byte stored here
  b = Wire.read(); // second received byte stored here
  c = Wire.read(); // third received byte stored here
  Wire.endTransmission();
  Serial.print(a);
  Serial.print(b);
  Serial.println(c);

  delay(5);
  while (!Serial);
  Serial.println(F("BME680 test"));
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
//    bme[idx].setTemperatureOversampling(BME680_OS_8X);
      bme[idx].setTemperatureOversampling(BME680_OS_NONE);
      bme[idx].setHumidityOversampling(BME680_OS_NONE);
      bme[idx].setPressureOversampling(BME680_OS_1X);
 //   bme[idx].setIIRFilterSize(BME680_FILTER_SIZE_3);
  //  bme[idx].setGasHeater(320, 150); // 320*C for 150 ms
    bme[idx].setGasHeater(0, 0); // 320*C for 150 ms 
  }
}

// only need to sample the ambient air occasinally
// (say once a minute) for PEEP analysis
int ambient_counter = 0;
float AMB_kPa = 0.0;  // Ambient pressure
#define AMB_UNDERSAMPLE 100
void loop() {
  seekUnfoundBME();
  float IPA_kPa = 0.0;  // Inspiratory Pathway pressure

  float flow = -999.0;
  if (found_bme[0])
    IPA_kPa = readPressureOnly(0);
  if (((ambient_counter % AMB_UNDERSAMPLE) == 0) && found_bme[1]) {
    AMB_kPa = readPressureOnly(1);
    ambient_counter = 1;
  } else {
    ambient_counter++;
  }
  flow = readFlow();

// I personally prefer to transfer JSON objects, 
// separated by new lines

// Note: Units are kiloPasccas for pressure, and slm for flow
  Serial.print("{ ");
  Serial.print("IPA: ");
  Serial.print(IPA_kPa);
  Serial.print(",");
  Serial.print("AMB: ");
  Serial.print(AMB_kPa);
  Serial.print(",");
  Serial.print("Flow :");
  Serial.print(flow);
  Serial.print("}\n");
//  delay(10);
}

float readPressureOnly(int idx) 
{
  if (! bme[idx].performReading()) {
    Serial.println("Failed to perform reading :( for:");
    Serial.println(addr[idx],HEX);
    found_bme[idx] = false;
    return -999.0;
  }

  // returning resorts in kPa
  return bme[idx].pressure / 1000.0;
}
void readBME(int idx) 
{
  if (! bme[idx].performReading()) {
    Serial.println("Failed to perform reading :( for:");
    Serial.println(addr[idx],HEX);
    found_bme[idx] = false;
    return;
  }
  Serial.print("Data For:");
  Serial.println(addr[idx],HEX);
//  Serial.print("Temperature = ");
//  Serial.print(bme[idx].temperature);
//  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme[idx].pressure / 1000.0);
  Serial.println(" kPa");

 // Serial.print("Humidity = ");
//  Serial.print(bme[idx].humidity);
//  Serial.println(" %");

  // Serial.print("Gas = ");
  // Serial.print(bme[idx].gas_resistance / 1000.0);
  //Serial.println(" KOhms");

  //Serial.print("Approx. Altitude = ");
  //Serial.print(bme[idx].readAltitude(SEALEVELPRESSURE_HPA));
  //Serial.println(" m");

  Serial.println();
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
 // Serial.println("raw");
  //Serial.println(a);
 // Serial.println(b);
  uint8_t crc = Wire.read(); // crc value stored here
  uint8_t mycrc = 0xFF; // initialize crc variable
  mycrc = crc8(a, mycrc); // let first byte through CRC calculation
  mycrc = crc8(b, mycrc); // and the second byte too
  if (mycrc != crc) { // check if the calculated and the received CRC byte matches
  //  Serial.println("Error: wrong CRC");
  }
  a = (a << 8) | b; // combine the two received bytes to a 16bit integer value
  a >>= 2; // remove the two least significant bits

  //Serial.println("combined");
  //Serial.println(a);
  //Serial.println(a - 8192);
  float Flow = ((float)a - 8192) / scale;
  //Serial.println(a); // print the raw data from the sensor to the serial interface
  //Serial.print("Flow in spm: ");
  //Serial.println(Flow); // print the calculated flow to the serial interface
  //Serial.println();
  return Flow;
}
