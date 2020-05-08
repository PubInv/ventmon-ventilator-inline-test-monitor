/* =====================================================================================
 *
 *       Filename:  VentMon_samplestructure.ino
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  04/15/2020 10:35:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lauria Clarke (lauriaclarke), lauria.clarke@gmail.com
 *   Organization:  Public Invention
 *
 * =====================================================================================
 */

 
#include <VentMon.h>


// address and name of server
IPAddress logHostAddr;
char* logHost = strdup(PARAMHOST);
uint16_t logPort = PARAMPORT;

// MAC address
byte macAddress[6];
char macAddressString[18];

// udp client
EthernetUDP udpClient;

// communication channel for events
Stream* eventChannel;

// container to hold events sent and received
PIRDSEvent* events[maximum_events];

// pressure sensors
Adafruit_BME680 airwayPressure;
Adafruit_BME680 ambientPressure;

// timing variables
unsigned long TP1 {0};
unsigned long TP2 {0};
unsigned long TF1 {0};
unsigned long TF2 {0};

// packets to send from circular buffer
packet_t packets[CSIZE];


// head and tail indicies of buffer
uint8_t cbuf_head = 0;
uint8_t cbuf_tail = 0;



void setup() 
{
   // use serial port for event communication
  Serial.begin(115200);
  eventChannel = &Serial;
  
  // start I2C
  Wire.begin();

  // setup ethernet connection
  setupEthernet(logHost, &logHostAddr, macAddress, macAddressString, &udpClient); 

  // setup flow sensor on I2C bus
  Meta m0;
  events[0] = &m0;
  // this is unclear do we really need to 
  setupFlowSensor(&m0);
  
  delay(1000);
  
  // setup pressure sensors on I2C bus
  Meta m1;
  events[1] = &m1;
  setupPressureSensors(&m1, &airwayPressure, &ambientPressure);

  // send the events over the event channel
  // sendPIRDSEvents(eventChannel, events, 2);
}

bool Firstrun = true;
uint32_t LastDHCP = 0;
#define DHCPWAIT 5000
uint32_t lastcap = 0;
uint8_t cnt = 0;
uint8_t d = 2;

uint32_t LogWait = 1000 * 1;
uint32_t ParamWait = 1000 * 60 * 15;
uint16_t LoopWait = 0;

uint32_t LastLog = 0;
uint32_t LastParam = 0;
uint32_t LastCheck = 0;

void loop() 
{
  // count of measurements taken - should always be 3
  // can be used for error checking...need to revisit  
  int eventCount = 0;


  //-- RECEIVE EVENTS (doesn't work, testing only!!) -----------
  // receive any incoming events on the channel
  // eventCount += receivePIRDSEvents(eventChannel, events);


  //-- GENERATE DATA --------------------------------------------
  // set timer values equal to the current time 
  TP1 = millis();
  TF1 = millis();

  // if the current sample time minus the last sample time is greater than 
  // or equal to the interval, take a sample and increment measurement count
//  Measurement p0;
//  Measurement p1;
//  if(TP1 - TP2 >= pressure_sample_interval)
//  {
//    eventChannel->println("sampling pressure...");
//
//    events[eventCount] = &p0;
//    samplePressure(&p0, ambient_pressure);
//    eventCount++;
//    
//    events[eventCount] = &p1;
//    samplePressure(&p1, internal_pressure);
//    eventCount++;
//    
//    TP2 = TP1;
//  }

  // if the current sample time minus the last sample time is greater than 
  // or equal to the interval, take a sample and increment measurement count
  Measurement f0;
  if(TF1 - TF2 >= flow_sample_interval)
  {
    eventChannel->println("sampling flow...");
    
    events[eventCount] = &f0;
    sampleFlow(&f0);
    eventCount++;
    
    TF2 = TF1;
  }

//  Measurement f0;
//  f0.setValues('P', 'A', 0, millis(), 2222);

  if (millis() - lastcap > 25)
  {
    pushData(packets, &cbuf_head, &f0);
    lastcap = millis();
  }

  if (d < 101)
  {
    if (cnt < 3)
    {
      if (Firstrun || millis() - LastLog >= d * 25)
      {
        cnt++;
        Serial.print(F("send data "));
        Serial.println(cnt);
        popData(&cbuf_tail, &cbuf_head, &udpClient, &f0, &logHostAddr);
        LastLog = millis();
      }
    }
    else
    {
      d++;
      cnt = 0;
    }
  }

  if (millis() - LastDHCP >= DHCPWAIT)
  {
    Ethernet.maintain();
    LastDHCP = millis();
  }

  Firstrun = 0;
  delay(LoopWait);

  //-- SEND EVENTS -----------------------------
  // send the events over the event channel
  // sendPIRDSEvents(eventChannel, events, eventCount);
}
