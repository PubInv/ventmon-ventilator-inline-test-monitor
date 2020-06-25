
/***************************************************************************
  Hacked by Robert L. Read, 2020
  Networking code by Geoff Mulligan 2020
  designed to support ethernet using esp wifi chip

  All Adafruit libraries written by Limor Fried for Adafruit Industries.

  Modififications released under BSD to avoid complication (I prefer GPL).
 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WiFi.h>
#include <EthernetUdp.h>
#include <Dns.h>

#include <PIRDS.h>
#include <SFM3X00.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* DISPLAY ********************************************/
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

bool found_display = false;
unsigned long buttonCpress_ms = 0;

#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14

/* NETWORKING ******************************************/

byte mac[6];
char macs[18];

#define PARAMHOST "ventmon.coslabs.com"
#define PARAMPORT 6111
#define LOCALPORT 6111

char *Loghost = strdup(PARAMHOST);
uint16_t Logport = PARAMPORT;
IPAddress LoghostAddr;

EthernetUDP udpclient;

/* PRESSURE *******************************************/

#define SEALEVELPRESSURE_HPA (1013.25)

// It would be nice to support more than one,
// but the BME680 only has two addresses (0x77, and 0x76 when SD0 tied to GND).

Adafruit_BME680 bme[2]; // I2C

bool found_bme[2] = { false, false}; // an abundance of caution to init

// !!! Unless physical hardware changes the ambient sensor should have an address of
// 0x76 while the airway sensor should have an address of 0x77 on the I2C bus.
// Do not change this unless directed to do so. !!!
// sensor addresses
#define AMBIENT_SENSOR_ADDRESS  0x76
#define AIRWAY_SENSOR_ADDRESS   0x77

// This is for Rob Read's broken board!!!!
// #define AMBIENT_SENSOR_ADDRESS  0x77
// #define AIRWAY_SENSOR_ADDRESS   0x76
// these values should match the order the sensors occur in the array addr (below)
#define AIRWAY_PRESSURE_SENSOR  0
#define AMBIENT_PRESSURE_SENSOR 1

uint8_t addr[2] = {AIRWAY_SENSOR_ADDRESS, AMBIENT_SENSOR_ADDRESS};

// we will ust this as a pressure to display to make the OLED useful...
// Eventually we will put this into running window
signed long display_max_pressure = 0;
signed long display_min_pressure = 0;

// only need to sample the ambient air occasinally
// (say once a minute) for PEEP analysis
int ambient_counter = 0;

unsigned long sample_millis = 0;

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

/* FLOW ***********************************************/

char* flow_too_high = FLOW_TOO_HIGH;
char* flow_too_low = FLOW_TOO_LOW;

SFM3X00 flowSensor(0x40);

#define PIRDS_SENSIRION_SFM3200 0
#define PIRDS_SENSIRION_SFM3400 1

int sensirion_sensor_type = PIRDS_SENSIRION_SFM3200;
// At present we have to install the SENSIRION_SFM3400 backwards
// because of the physical mounting!!!
bool SENSOR_INSTALLED_BACKWARD = true;

/* FiO2 ***********************************************/

// oxygen sensor is connecte to channel 3 of ADS115
#define O2CHANNEL 3
#define UNPLUGGED_MAX 400

Adafruit_ADS1115 ads;

int initialO2 = 0;
bool oxygenSensing = true;

/******************************************************/

void setup() {

  Serial.begin(115200);
  Wire.begin();

  ethernet_setup();

  flowSensor.begin();

  setupOLED();

  int a = 0;
  int b = 0;
  int c = 0;

  delay(1000);


  while (!Serial);
  //Serial.println(F("BME680 test"));
  seekBME(AIRWAY_PRESSURE_SENSOR);
  seekBME(AMBIENT_PRESSURE_SENSOR);
  signed long v = readPressureOnly(AMBIENT_PRESSURE_SENSOR);
  init_ambient(v);

  initializeOxygenSensor();
}


void loop() {

  if (found_display) {
    // experimental OLED test code
    if (!digitalRead(BUTTON_A)) {
      buttonA();
    }
    // ButtonB is causing my machine to hang with no explanation
    else if (!digitalRead(BUTTON_B)) {
      Serial.println("B BUTTON");
      buttonB();
      Serial.println("B BUTTON PROCESS DONE");
    }
    else if (!digitalRead(BUTTON_C)) {
      unsigned long m = millis();
      if (m > (buttonCpress_ms + 1000)) {
        buttonCpress_ms = m;
        Serial.println("B BUTTON");
        buttonC();
      }
    } else {
      display.clearDisplay();

      display.setCursor(0, 0);
      displayPressure(true);
      display.setCursor(0, 10);
      displayPressure(false);
    }

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

  if (found_bme[AIRWAY_PRESSURE_SENSOR]) {
    internal_pressure = readPressureOnly(AIRWAY_PRESSURE_SENSOR);
  }
  if (((ambient_counter % AMB_SAMPLES_PER_WINDOW_ELEMENT) == 0) && found_bme[AMBIENT_PRESSURE_SENSOR]) {
    ambient_pressure = readPressureOnly(AMBIENT_PRESSURE_SENSOR);
    ambient_counter = 1;

    // experimentally we will report everything in the stream from
    // both sensor; sadly the BM# 680 is to slow to do this every sample.
    report_full(AIRWAY_PRESSURE_SENSOR);
    report_full(AMBIENT_PRESSURE_SENSOR);

    if (ambient_pressure != -999) {
      outputMeasurement('M', 'P', 'B', 1, ms, ambient_pressure);
      ambient_window[amb_wc] = ambient_pressure;
      amb_wc = (amb_wc + 1) % AMB_WINDOW_SIZE;
      Serial.println();
    } else {
      Serial.print("\"NA\"");
    }
  } else {
    ambient_counter++;
  }
  signed long smooth_ambient = 0;
  for (int i = 0; i < AMB_WINDOW_SIZE; i++) {
    smooth_ambient += ambient_window[i];
  }
  smooth_ambient = (signed long) (smooth_ambient / AMB_WINDOW_SIZE);


  if (internal_pressure != -999) {
    outputMeasurement('M', 'P', 'A', 0, ms, internal_pressure);
    Serial.println();
    // really this should be a running max, for now it is instantaneous
    display_max_pressure = internal_pressure - smooth_ambient;
    outputMeasurement('M', 'D', 'A', 0, ms, internal_pressure - smooth_ambient);
    Serial.println();
  } else {
    // This is not actually part of the format!!!
    Serial.print("\"NA\"");
  }



  // FiO2
  if (oxygenSensing)
  {
    float fiO2 = (avgADC(O2CHANNEL) / initialO2) * 20.9;
    outputMeasurement('M', 'O', 'A', 0, ms, fiO2);
  }

  // FLOW
  // our units are slm * 1000, or milliliters per minute.
  float raw_flow = -999.0;
  raw_flow = flowSensor.readFlow();
  if (flowSensor.checkRange(raw_flow)) {
    if (raw_flow > 0) {
      outputMetaEvent( (char *) ((SENSOR_INSTALLED_BACKWARD) ? flow_too_low : flow_too_high), ms);
    } else {
      outputMetaEvent( (char *) ((SENSOR_INSTALLED_BACKWARD) ? flow_too_high : flow_too_low), ms);
    }
    Serial.println();
  }

  float flow = (SENSOR_INSTALLED_BACKWARD) ? -raw_flow : raw_flow;
  signed long flow_milliliters_per_minute = (signed long) (flow * 1000);
  outputMeasurement('M', 'F', 'A', 0, ms, flow_milliliters_per_minute);

  Serial.println();
  Serial.flush();
}

/******************************************************/

/* DATA TRANSFER ***************************************/

void output_on_serial_print_PIRDS(char e, char t, char loc, unsigned short int n, unsigned long ms, signed long val) {
  Measurement ma = get_measurement(e, t, loc, n, ms, val);
  // I need a proof that this buffer is larger enough, but I think it is...
  char buff[256];
  int rv = fill_JSON_buffer_measurement(&ma, buff, 256);
  Serial.print(buff);
}

void outputMeasurement(char e, char t, char loc, unsigned short int n, unsigned long ms, signed long val) {

  output_on_serial_print_PIRDS(e, t, loc, n, ms, val);

  send_data(e, t, loc, n, ms, val);
}

void outputMetaEvent(char *msg, unsigned long ms) {
  Message m = get_message(ms, msg);
  char buff[264];
  int rv = fill_JSON_buffer_message(&m, buff, 264);
  Serial.print(buff);

  send_data_message(m);
}

void report_full(int idx)
{
  unsigned long ms = millis();

  if (! bme[idx].performReading()) {
    Serial.println("Failed to perform reading :( for:");
    Serial.println(addr[idx], HEX);
    found_bme[idx] = false;
    return;
  }

  char loc = (idx == 0) ? 'A' : 'B';
  outputMeasurement('M', 'T', loc, 0, ms, (signed long) (0.5 + (bme[idx].temperature * 100)));
  Serial.println();
  outputMeasurement('M', 'P', loc, 0, ms, (signed long) (0.5 + (bme[idx].pressure / (98.0665 / 10))));
  Serial.println();
  outputMeasurement('M', 'H', loc, 0, ms, (signed long) (0.5 + (bme[idx].humidity * 100)));
  Serial.println();
  outputMeasurement('M', 'G', loc, 0, ms, (signed long) (0.5 + bme[idx].gas_resistance));
  Serial.println();
  outputMeasurement('M', 'A', loc, 0, ms, (signed long) (0.5 + bme[idx].readAltitude(SEALEVELPRESSURE_HPA)));
  Serial.println();
}

void debugPrintBuffer(uint8_t* m, int n) {
  for (int i = 0; i < n; i++) {
    Serial.print(m[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

Measurement get_measurement(char event, char mtype, char loc, uint8_t num, uint32_t ms, int32_t value) {
  Measurement ma;
  ma.event = event;
  ma.type = mtype;
  ma.loc = loc;
  ma.num = 0 + num;
  ma.ms = ms;
  ma.val = value;
  return ma;
}

Message get_message(uint32_t ms, char *msg) {
  Message m;
  m.event = 'E';
  m.type = 'M';
  m.ms = ms;
  int n = strlen(msg);
  if (n > 255) {
    m.b_size = 255;
    memcpy(m.buff, msg, 255);
  } else {
    m.b_size = n;
    strcpy(m.buff, msg);
  }
  return m;
}

bool send_data_measurement(Measurement ma) {

  unsigned char m[14];

  fill_byte_buffer_measurement(&ma, m, 13);

  m[13] = '\n';

  //  Serial.print(F(" UDP send to "));
  //  Serial.print(LoghostAddr);
  //  Serial.print(F(" "));
  //  Serial.println(Logport);

  if (udpclient.beginPacket(LoghostAddr, Logport) != 1) {
    Serial.println("send_data_measurement begin failed!");
    return false;
  }
  udpclient.write(m, 14);
  if (udpclient.endPacket() != 1) {
    Serial.println("send_data_measurement end failed!");
    return false;
  }

  return true;
}

bool send_data(char event, char mtype, char loc, uint8_t num, uint32_t ms, int32_t value) {
  Measurement ma = get_measurement(event, mtype, loc, num, ms, value);
  return send_data_measurement(ma);
}

bool send_data_message(Message ma) {

  unsigned char m[264];

  fill_byte_buffer_message(&ma, m, 264);
  // I don't know how to compute this byte.
  m[263] = '\n';


  if (udpclient.beginPacket(LoghostAddr, Logport) != 1) {
    Serial.println("send_data_message begin failed!");
    return false;
  }
  udpclient.write(m, 264);
  if (udpclient.endPacket() != 1) {
    Serial.println("send_data_message end failed!");
    return false;
  }

  return true;
}

/******************************************************/

/* FiO2 ***********************************************/

void initializeOxygenSensor()
{
  // setup ADC for oxygen sensing
  ads.setGain(GAIN_SIXTEEN);
  ads.begin();
  initialO2 = avgADC(O2CHANNEL);

  Serial.println(-(UNPLUGGED_MAX));

  if (initialO2 <= UNPLUGGED_MAX && initialO2 >= -(UNPLUGGED_MAX))
  {
    Serial.println("WARNING: No oxygen sensor detected.");
    oxygenSensing = false;
  }
}

double avgADC(int adcNumber)
{
  double adc = 0;
  double average;

  for (int i = 0; i <= 19; i++)
  {
    int16_t adsread = ads.readADC_SingleEnded(adcNumber);
    adc = adc + adsread;
    delay(10);
  }

  average = adc / 20;

  return average;
}

/******************************************************/

/* FLOW ***********************************************/


/******************************************************/

/* PRESSURE *******************************************/

void seekUnfoundBME() {
  if (!found_bme[AIRWAY_PRESSURE_SENSOR]) {
    seekBME(AIRWAY_PRESSURE_SENSOR);
  }
  if (!found_bme[AMBIENT_PRESSURE_SENSOR]) {
    seekBME(AMBIENT_PRESSURE_SENSOR);
  }
}

void seekBME(int idx) {
  uint8_t loc_addr = addr[idx];
  // I don't understand why this API does not work, it seemed to work in the previous version....
  found_bme[idx] = bme[idx].begin(loc_addr, true);
  //  found_bme[idx] = bme[idx].begin(addr[idx]);
  if (!found_bme[idx]) {
    Serial.println("Could not find a valid BME680 sensor, check wiring for:");
    Serial.println(loc_addr, HEX);
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

void init_ambient(signed long v) {
  for (int i = 0; i < AMB_WINDOW_SIZE; i++) ambient_window[i] = v;
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

/******************************************************/

/* DISPLAY ********************************************/

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
  display.setCursor(0, 0);
  display.print("Connecting to SSID\n'adafruit':");
  display.print("connected!");
  display.println("IP: 10.0.1.23");
  display.println("Sending val #0");
  display.setCursor(0, 0);
  display.display(); // actually display all of the above
}

void buttonA() {
  Serial.println("A BUTTON");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("A");
}

void buttonB() {
  Serial.println("B BUTTON");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("B");
}

// This is an experimental used of this event!!!
void buttonC() {
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print("C");

  unsigned long ms = millis();
  int n = strlen(SAVE_LOG_TO_FILE);
  char buffer[255];
  strcpy(buffer, SAVE_LOG_TO_FILE);
  strcpy(buffer + n, "test_file_name");
  outputMetaEvent(buffer, ms);
  Serial.println();
}

// Trying to make simple, I will define 3 lines..
void displayLine(int n, char* s) {
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
  sprintf(buffer, "%d.1", display_pressure / 10);
  display.println(buffer);
}

/******************************************************/

/* NETWORKING ******************************************/

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

/******************************************************/
