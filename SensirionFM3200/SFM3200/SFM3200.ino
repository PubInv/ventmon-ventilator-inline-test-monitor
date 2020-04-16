
#include <Wire.h>


void setup()
{
  Wire.begin();
  Serial.begin(115200);

  requestSerialNumber();  
  

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

// None of the Sensirion sensors I have seem to actually have a serial number built in!
// https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/5_Mass_Flow_Meters/Application_Notes/Sensirion_Mass_Flo_Meters_SFM3xxx_I2C_Functional_Description.pdf
void requestSerialNumber() {
    Wire.beginTransmission(byte(0x40)); // transmit to device #064 (0x40)
    // 0x31AE is the command to read the serial number
  Wire.write(byte(0x31));      //
  Wire.write(byte(0xAE));      //
  Wire.endTransmission(); 
  Wire.requestFrom(0x40, 4); // read 4 bytes from device with address 0x40
  uint8_t a = Wire.read(); 
  uint8_t b = Wire.read();
  uint8_t c = Wire.read(); 
  uint8_t d = Wire.read(); 
  Serial.println("Sensirion Serial Number:");
  Serial.println(a,HEX);
  Serial.println(b,HEX);
  Serial.println(c,HEX);
  Serial.println(d,HEX);
}

void loop() {
// Documentation inconsistent
  int offset = 32768; // Offset for the sensor
  // use scale of 800 for SFM3400 and 120 for SFM3200
  float scale = 120.0; // Scale factor for Air and N2 is 140.0, O2 is 142.8
  
 
  Wire.requestFrom(0x40, 3); // read 3 bytes from device with address 0x40
  uint16_t a = Wire.read(); // first received byte stored here. The variable "uint16_t" can hold 2 bytes, this will be relevant later
  uint8_t b = Wire.read(); // second received byte stored here
  Serial.println("raw");
  Serial.println(a);
  Serial.println(b);
  uint8_t crc = Wire.read(); // crc value stored here
  uint8_t mycrc = 0xFF; // initialize crc variable
  mycrc = crc8(a, mycrc); // let first byte through CRC calculation
  mycrc = crc8(b, mycrc); // and the second byte too
  if (mycrc != crc) { // check if the calculated and the received CRC byte matches
  //  Serial.println("Error: wrong CRC");
  }
  a = (a << 8) | b; // combine the two received bytes to a 16bit integer value
  a >>= 2; // remove the two least significant bits

  Serial.println("combined");
  Serial.println(a);
  Serial.println(a - 8192);
  float Flow = ((float)a - 8192) / scale;
  //Serial.println(a); // print the raw data from the sensor to the serial interface
  Serial.println(Flow); // print the calculated flow to the serial interface

  delay(550);
}
