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
  long serialNo = requestSerialNumber(FLOW_SENSOR_ADDRESS);
 
  uint16_t scale = requestScaleFactor(FLOW_SENSOR_ADDRESS);

  uint16_t offset = requestOffset(FLOW_SENSOR_ADDRESS);


  if(offset != FLOW_OFFSET)
  {
    meta->setValues('M', "OFFSET mismatch!");
    // DEBUG_PRINT("OFFSET mismatch!");
    return 1;
  }
  else if(scale != FLOW_SCALE)
  {
    meta->setValues('M', "SCALE mismatch!");
    // DEBUG_PRINT("SCALE mismatch!");
    return 1;
  }
  else
  {
    meta->setValues('M', "flow sensor initialized");
    // DEBUG_PRINT("flow sensor initialized!");
    startContinuousMeasurement(FLOW_SENSOR_ADDRESS);
  }

  return 0;
}

bool setupPressureSensors(Meta* meta, Adafruit_BME680* airwayPressure, Adafruit_BME680* ambientPressure)
{
  
  // AIRWAY SENSOR - 0X77
  if (!airwayPressure->begin(BME680_AIRWAY_ADDRESS, true))     
  {
    meta->setValues('M', "could not find AIRWAY pressure sensor");
    return 1;
  }

  // Set up oversampling and filter initialization
  airwayPressure->setTemperatureOversampling(BME680_OS_8X);
  airwayPressure->setHumidityOversampling(BME680_OS_2X);
  airwayPressure->setPressureOversampling(BME680_OS_4X);
  airwayPressure->setIIRFilterSize(BME680_FILTER_SIZE_3);
  airwayPressure->setGasHeater(320, 150); // 320*C for 150 ms


  // AMBIENT SENSOR - 0X76
  if (!ambientPressure->begin(BME680_AMBIENT_ADDRESS, true))     
  {
    meta->setValues('M', "could not find AMBIENT pressure sensor");
    return 1;
  }

  // Set up oversampling and filter initialization
  ambientPressure->setTemperatureOversampling(BME680_OS_8X);
  ambientPressure->setHumidityOversampling(BME680_OS_2X);
  ambientPressure->setPressureOversampling(BME680_OS_4X);
  ambientPressure->setIIRFilterSize(BME680_FILTER_SIZE_3);
  ambientPressure->setGasHeater(320, 150); // 320*C for 150 ms
  
  meta->setValues('M', "pressure sensors initialized");

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
  uint16_t flow = readFlow(FLOW_SENSOR_ADDRESS);
  
  measurement->setValues('F', 'A', 4, millis(), flow);
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


void setupEthernet(char* logHost, IPAddress* logHostAddr, byte* macAddress, char* macAddressString, EthernetUDP* udpClient) 
{
  byte ip[] = { 124, 103, 1, 1 }; 
  // ESP32 with Adafruit Featherwing Ethernet
  Ethernet.init(33);
  // Get MAC address of wifi chip for ethernet address
  WiFi.macAddress(macAddress); 
  snprintf(macAddressString, sizeof macAddressString, "%02X:%02X:%02X:%02X:%02X:%02X",
           macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
  Serial.print(F("Mac: "));
  Serial.print(macAddressString);
  Serial.println();

  while (1) 
  {
    // start the Ethernet connection:
    Serial.println(F("Initialize Ethernet with DHCP:"));
    if (Ethernet.begin(macAddress) == 0) 
    {
      Serial.println(F("Failed to configure Ethernet using DHCP"));
      // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware) 
      {
        // should this throw more of an error
        Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
        while (1) 
        {
          // do nothing, no point running without Ethernet hardware
          delay(1); 
        }
      }
      if (Ethernet.linkStatus() == LinkOFF) 
      {
        Serial.println(F("Ethernet cable is not connected."));
        delay(5000);
      }
      delay(5000);
    } 
    else 
    {
      Serial.print(F("  DHCP assigned IP "));
      Serial.println(Ethernet.localIP());
      break;
    }
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);

#ifdef UDP
  udpClient->begin(PARAMPORT);
#endif

  DNSClient dns;
  dns.begin(Ethernet.dnsServerIP());
  
  if (dns.getHostByName(logHost, *logHostAddr) == 1) 
  {
    Serial.print("host is ");
    Serial.println(*logHostAddr);
  }

}


bool sendDataUDP(EthernetUDP* udpClient, Measurement* measurement, IPAddress* logHostAddr) 
{
  union 
  {
    struct 
    {
      char first;
      char mtype;
      char stype;
      uint8_t loc;
      uint32_t ms;
      int32_t value;
      char nl;
    } a;
    byte m[13];
  } message;

  uint8_t *mm = message.m;
  message.a.first = measurement->eventType;
  message.a.mtype = measurement->measurementType;
  message.a.stype = measurement->deviceType;
  message.a.loc   = 0 + measurement->deviceLocation;
  message.a.ms    = htonl(measurement->measurementTime);
  message.a.value = htonl(measurement->measurementValue);
  message.a.nl = '\n';

  Serial.print("Value: ");
  Serial.println(message.a.value;
  
  //Serial.print(F(" UDP send to "));
  //Serial.println(*logHostAddr);

  if (udpClient->beginPacket(*logHostAddr, PARAMPORT) != 1)
    return false;

  udpClient->write(message.m, 13);

  if (udpClient->endPacket() != 1)
    return false;

  return true;
}



void pushData(packet_t* packets, uint8_t* cbuf_head, Measurement* measurement) 
{
  //Serial.print("pushData - head: ");
  //Serial.println(*cbuf_head);

  packets[*cbuf_head].mtype = measurement->measurementType;
  packets[*cbuf_head].stype = measurement->deviceType;            
  packets[*cbuf_head].loc   = measurement->deviceLocation;
  packets[*cbuf_head].ms    = measurement->measurementTime;
  packets[*cbuf_head].value = measurement->measurementValue;
  (*cbuf_head)++;

  if (*cbuf_head > CSIZE) *cbuf_head = 0;
}

uint8_t popData(uint8_t* cbuf_tail, uint8_t* cbuf_head, EthernetUDP* udpClient, Measurement* measurement, IPAddress* logHostAddr) 
{
  //Serial.print("popData - head: ");
  //Serial.print(*cbuf_head);
  //Serial.print(" tail: ");
  //Serial.println(*cbuf_tail);

  while (*cbuf_head != *cbuf_tail) 
  {
    sendDataUDP(udpClient, measurement, logHostAddr);

    (*cbuf_tail)++;
    if ((*cbuf_tail) > CSIZE)
      (*cbuf_tail) = 0;
  }
  return 1;
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


