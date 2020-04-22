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

/***************************************************************************
  Networking code by Geoff Mulligan, 2020
  designed to support ethernet using esp wifi chip
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WiFi.h>
#include <EthernetUdp.h>
#include <Dns.h>

#include <SFM3X00.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
 
// OLED FeatherWing buttons map to different pins depending on board:
#if defined(ESP8266)
  #define BUTTON_A  0
  #define BUTTON_B 16
  #define BUTTON_C  2
#elif defined(ESP32)
  #define BUTTON_A 15
  #define BUTTON_B 32
  #define BUTTON_C 14
#elif defined(ARDUINO_STM32_FEATHER)
  #define BUTTON_A PA15
  #define BUTTON_B PC7
  #define BUTTON_C PC5
#elif defined(TEENSYDUINO)
  #define BUTTON_A  4
  #define BUTTON_B  3
  #define BUTTON_C  8
#elif defined(ARDUINO_FEATHER52832)
  #define BUTTON_A 31
  #define BUTTON_B 30
  #define BUTTON_C 27
#else // 32u4, M0, M4, nrf52840 and 328p
  #define BUTTON_A  9
  #define BUTTON_B  6
  #define BUTTON_C  5
#endif


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

bool found_display = false;

#define PIRDS_SENSIRION_SFM3200 0
#define PIRDS_SENSIRION_SFM3400 1
int sensirion_sensor_type = PIRDS_SENSIRION_SFM3400;
// At present we have to install the SENSIRION_SFM3400 backwards
// because of the physical mounting!!!
bool SENSOR_INSTALLED_BACKWARD = true;

// we will ust this as a pressure to display to make the OLED useful...
// Eventually we will put this into running window
signed long display_max_pressure = 0;
signed long display_min_pressure = 0;

#ifndef htonl
#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                   ((x)<< 8 & 0x00FF0000UL) | \
                   ((x)>> 8 & 0x0000FF00UL) | \
                   ((x)>>24 & 0x000000FFUL) )
#endif

byte mac[6];
char macs[18];

#define PARAMHOST "ventmon.coslabs.com"
#define PARAMPORT 6111
#define LOCALPORT 6111

char *Loghost = strdup(PARAMHOST);
uint16_t Logport = PARAMPORT;
IPAddress LoghostAddr;

EthernetUDP udpclient;

//#define CIRCBUFF
#ifdef CIRCBUFF
#define CSIZE 500

struct packet_t {
  char event;
  char mtype;
  char loc;
  uint8_t num;
  uint32_t ms;
  int32_t value;
} packets[CSIZE];

uint8_t cbuf_head = 0;
uint8_t cbuf_tail = 0;

void push_data(char event, char mtype, char loc, uint8_t num, uint32_t ms, int32_t value) {
  packets[cbuf_head].event = event;
  packets[cbuf_head].mtype = mtype;
  packets[cbuf_head].log = loc;
  packets[cbuf_head].num = num;
  packets[cbuf_head].ms = ms;
  packets[cbuf_head].value = value;
  cbuf_head++;
  if (cbuf_head > CSIZE) cbuf_head = 0;
}

uint8_t pop_data() {
  while (cbuf_head != cbuf_tail) {
    send_data('M', packets[cbuf_tail].event, packets[cbuf_tail].mtype, packets[cbuf_tail].loc, packets[cbuf_tail].num,
	      packets[cbuf_tail].ms, packets[cbuf_tail].value);
    cbuf_tail++;
    if (cbuf_tail > CSIZE)
      cbuf_tail = 0;
  }
  return 1;
}
#endif

void setupOLED() {
 // Here we initialize the OLED...
  Serial.println("OLED FeatherWing test");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  int ret_oled = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.println("Return value for OLED");
  Serial.println(ret_oled);
  if (ret_oled != 1) { // Address 0x3C for 128x32
    // hopefully this just means you don't have the board; we should send this as a message in the stream
    Serial.println("returing from OLED setup!");
    return;
  }
  found_display = true;
  Serial.println("OLED begun");
 
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();

  delay(1000);
 
  // Clear the buffer.
  display.clearDisplay();
  display.display();
 
  Serial.println("IO test");
 
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
 
  // text display tests
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("Connecting to SSID\n'adafruit':");
  display.print("connected!");
  display.println("IP: 10.0.1.23");
  display.println("Sending val #0");
  display.setCursor(0,0);
  display.display(); // actually display all of the above 
}

void ethernet_setup() {
  Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet
  
  WiFi.macAddress(mac); // Get MAC address of wifi chip for ethernet address
  snprintf(macs, sizeof macs, "%02X:%02X:%02X:%02X:%02X:%02X",
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(F("Mac: "));
  Serial.print(macs);
  Serial.println();
  
  while (1) {
    // start the Ethernet connection:
    Serial.println(F("Initialize Ethernet with DHCP:"));
    if (Ethernet.begin(mac) == 0) {
      Serial.println(F("Failed to configure Ethernet using DHCP"));
      // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware) {
	Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
	while (1) {
	  delay(1); // do nothing, no point running without Ethernet hardware
	}
      }
      if (Ethernet.linkStatus() == LinkOFF) {
	Serial.println(F("Ethernet cable is not connected."));
	delay(5000);
      }
      delay(5000);
    } else {
      Serial.print(F("  DHCP assigned IP "));
      Serial.println(Ethernet.localIP());
      break;
    }
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);

  udpclient.begin(LOCALPORT);
  
  DNSClient dns;
  dns.begin(Ethernet.dnsServerIP());
  if (dns.getHostByName(Loghost, LoghostAddr) == 1) {
    Serial.print("host is ");
    Serial.println(LoghostAddr);
  }
}

void setup() {

  Serial.begin(115200);
  Wire.begin();

  ethernet_setup();

  setupOLED();
  initSensirionFM3200Measurement();

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

bool send_data(char event, char mtype, char loc, uint8_t num, uint32_t ms, int32_t value) {
  union {
    struct {
      char first;
      char mtype;
      char loc;
      uint8_t num;
      uint32_t ms;
      int32_t value;
      char nl;
    } a;
    byte m[13];
  } message;
  uint8_t *mm = message.m;
  message.a.first = 'M';
  message.a.mtype = mtype;
  message.a.loc = loc;
  message.a.num = 0 + num;
  message.a.ms = htonl(ms);
  message.a.value = htonl(value);
  message.a.nl = '\n';

  Serial.print(F(" UDP send to "));
  Serial.print(LoghostAddr);
  Serial.print(F(" "));
  Serial.println(Logport);

  if (udpclient.beginPacket(LoghostAddr, Logport) != 1)
    return false;
  udpclient.write(message.m, 13);
  if (udpclient.endPacket() != 1)
    return false;

  return true;
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
//    bme[idx].setIIRFilterSize(BME680_FILTER_SIZE_3);
//     bme[idx].setGasHeater(320, 150); // 320*C for 150 ms
    // I believe this feature is not needed or useful for this application
    bme[idx].setGasHeater(0, 0); // 320*C for 150 ms  
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
  Serial.print("\" : ");
  Serial.print(v);
  Serial.print(", ");
}
void outputNumFieldNoSep(char *name,signed long v) {
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : ");
  Serial.print(v);
  Serial.print(" ");
}

void outputByteField(char *name,unsigned short v) {
  Serial.print("\"");
  Serial.print(name);
  Serial.print("\" : ");
  Serial.print((unsigned short int) v);
  Serial.print(", ");
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

  send_data(e, t, loc, n, ms, val);
}

// We could send out only raw data, and let more powerful computers process thing.
// But we have a powerful micro controller here, so we will try to be useful!
// Instead of outputting only the absolute pressure, we will output the differential
// pressure in the Airway. We will compute a differential pressure against the 
// ambient air. We will name this D0. We need a moving window to make sure there
// is not jitter.
int amb_wc = 0;
#define AMB_WINDOW_SIZE 4
#define AMB_SAMPLES_PER_WINDOW_ELEMENT 200 
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


// This routine was gotten from the Arduino forums and is informal.
// I may need to improve it; the problem I am currently having is awful.
float readSensirionFlow(int sensirion_sensor_type) {

  
// Documentation inconsistent
  int offset = 32768; // Offset for the sensor

    // NOTE: THIS IS DEPENDENT ON SENSOR!!
    // We may need to use the buttons on the OLED or some other
    // means for setting this.
  const float FM3200_SCALE = 120.0;
  const float FM3400_33_AW_SCALE = 800.0;

// We have to adjust the Library to add the other scale.
  if (sensirion_sensor_type == PIRDS_SENSIRION_SFM3400) {
    float flow = readFlow(sensor_address);
    return flow;
  }

// We currently cannot implement FM3200
  return -99.0;
  
  float scale = (sensirion_sensor_type == PIRDS_SENSIRION_SFM3200) ? FM3200_SCALE : FM3400_33_AW_SCALE;


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

void initSensirionFM3200Measurement() {
// Trying to explicitly send an instruction byte:
  Wire.beginTransmission(0x40); 
  Wire.write(byte(0x10));
  Wire.write(byte(0x00)); // sends instruction byte    Wire.write(val);             // sends potentiometer value byte  
  Wire.endTransmission();     // stop transmitting
  delay(5);
  {
   Wire.requestFrom(0x40, 3); // read 3 bytes from device with address 0x40
   delay(110);
   Serial.println(Wire.available());
  uint16_t a = Wire.read(); // first received byte stored here. The variable "uint16_t" can hold 2 bytes, this will be relevant later
  uint8_t b = Wire.read();
  uint8_t c = Wire.read();
  Serial.println("a,b,c");
  Serial.println(a,HEX);
  Serial.println(b,HEX);
    Serial.println(c,HEX);
  }
}

void buttonA() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("A");
}
void buttonB() {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("B");
}
void buttonC() {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.print("C");
}

// Trying to make simple, I will define 3 lines..
void displayLine(int n,char* s) {
    display.setCursor(0, 10);
    display.print(s);
}

void displayPressure(bool max_not_min) {     
      display.print(max_not_min ? "Max" : "Min" );   
      display.print(" cm H2O: ");
      char buffer[32];
      long display_pressure = max_not_min ? display_max_pressure : display_min_pressure;
      int pressure_sign = ( display_pressure < 0) ? -1 : 1;
      int abs_pressure = (pressure_sign == -1) ? - display_pressure : display_pressure;
      sprintf(buffer, "%d.1", display_pressure/10);  
      display.println(buffer);
}

void loop() {
// TODO: Need to use buttons to allow Sensirion type to be detected if can't do automatically

  if (found_display) {
// experimental OLED test code
    if(!digitalRead(BUTTON_A)) {
      buttonA();
    } else if(!digitalRead(BUTTON_B)) {
      buttonB();
    } else if(!digitalRead(BUTTON_C)) { 
      buttonC();
    } else {
      display.clearDisplay();

      display.setCursor(0, 0);
      displayPressure(true);
      display.setCursor(0,10);
      displayPressure(false);  

    }
  //  delay(10);
  //  yield();
    display.display();
  
  }
  
  unsigned long m = millis();
  if (m > sample_millis) {
    sample_millis = m;
  } else {
    Serial.println("unticked");
    return;
  }

  unsigned long ms = millis();
  seekUnfoundBME();


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


  if (internal_pressure != -999) {    
    outputMeasurment('M', 'P', 'A', 0, ms, internal_pressure);
    Serial.println();
     // really this should be a running max, for now it is instantaneous
    display_max_pressure = internal_pressure - smooth_ambient;
    outputMeasurment('M', 'D', 'A', 0, ms, internal_pressure - smooth_ambient);  
    Serial.println();
  } else {
    // This is not actually part of the format!!!
    Serial.print("\"NA\"");  
  }



// our units are slm * 1000, or milliliters per minute.
  float flow = -999.0;
  float raw_flow = readSensirionFlow(sensirion_sensor_type);
  flow = (SENSOR_INSTALLED_BACKWARD) ? -raw_flow : raw_flow;

  signed long flow_milliliters_per_minute = (signed long) (flow * 1000);
  
  outputMeasurment('M', 'F', 'A', 0, ms, flow_milliliters_per_minute);

  Serial.println();
  Serial.flush();
}
