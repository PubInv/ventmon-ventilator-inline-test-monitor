// hardcoded sensor values can be found in SFM3X00.h
#include <SFM3X00.h>

#define SAMPLE_DELAY   550

void setup()
{
  Wire.begin();
  Serial.begin(9600);

  long serialNo = requestSerialNumber(SENSOR_ADDRESS);
  Serial.println();
  Serial.print("sensor serial number: ");
  Serial.println(serialNo, HEX);
  Serial.println();
 
  uint16_t scaleFactor = requestScaleFactor(SENSOR_ADDRESS);
  Serial.print("read scale factor (HEX, DEC): ");
  Serial.print(scaleFactor, HEX);
  Serial.print(", ");
  Serial.println(scaleFactor, DEC);
  Serial.print("hardcoded scale factor (DEC): ");
  Serial.println((uint16_t) FLOW_SCALE, DEC);
  Serial.println();


  uint16_t offset = requestOffset(SENSOR_ADDRESS);
  Serial.print("read flow offset (HEX, DEC): ");
  Serial.print(offset, HEX);
  Serial.print(", ");
  Serial.println(offset, DEC);
  Serial.print("hardcoded flow offset (DEC): ");
  Serial.println((uint16_t) FLOW_OFFSET, DEC);
  Serial.println();


  Serial.print("sample rate: ");
  Serial.print(1000.0 / (float)SAMPLE_DELAY);
  Serial.println(" Hz");
  Serial.println();

  startContinuousMeasurement(SENSOR_ADDRESS);
  
  delay(5);
}


void loop()
{
  float flow = readFlow(SENSOR_ADDRESS);
  Serial.print("flow (slm): ");
  Serial.println(flow);
  delay(SAMPLE_DELAY);
}
