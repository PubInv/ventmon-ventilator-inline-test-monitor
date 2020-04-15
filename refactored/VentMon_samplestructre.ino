#include <Wire.h>
#include <VentMon.h>
#include <PIRDSEvent.h>

// pick wich channel you're using to send events
#define I2C_EVENTS      0
#define SERIAL_EVENTS   1

// communication channel for events
Stream* eventChannel;

// timing variables
unsigned long TP1 {0};
unsigned long TP2 {0};
unsigned long TF1 {0};
unsigned long TF2 {0};

// container to hold events sent and received
PIRDSEvent* events[maximum_events];


void setup() 
{
   // use serial port for event communication
  Serial.begin(115200);
  eventChannel = &Serial;
  
  // start I2C
  Wire.begin();

  // setup flow sensor on I2C bus
  Meta m0;
  events[0] = &m0;
  setupFlowSensor(eventChannel, &m0);
  
  delay(1000);
  
  // setup pressure sensors on I2C bus
  Meta m1;
  events[1] = &m1;
  setupPressureSensors(eventChannel, &m1);

  // send the events over the event channel
  sendPIRDSEvents(eventChannel, events, 2);
}

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
  Measurement p0;
  Measurement p1;
  if(TP1 - TP2 >= pressure_sample_interval)
  {
    eventChannel->println("sampling pressure...");

    events[eventCount] = &p0;
    samplePressure(&p0, ambient_pressure);
    eventCount++;
    
    events[eventCount] = &p1;
    samplePressure(&p1, internal_pressure);
    eventCount++;
    
    TP2 = TP1;
  }

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

  //-- SEND EVENTS -----------------------------
  // send the events over the event channel
  sendPIRDSEvents(eventChannel, events, eventCount);
  
}
