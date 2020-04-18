// hardcoded sensor values can be found in SFM3X00.h
#include <SFM3X00.h>

#define SAMPLE_DELAY   550


void setup()
{
  Wire.begin();
  Serial.begin(9600);

  long serialNo = requestSerialNumber(sensor_address);
  Serial.println();
  Serial.print("sensor serial number: ");
  Serial.println(serialNo, HEX);
  Serial.println();
 
  uint16_t scaleFactor = requestScaleFactor(sensor_address);
  Serial.print("read scale factor (HEX, DEC): ");
  Serial.print(scaleFactor, HEX);
  Serial.print(", ");
  Serial.println(scaleFactor, DEC);
  Serial.print("hardcoded scale factor (DEC): ");
  Serial.println((uint16_t) flow_scale, DEC);
  Serial.println();


  uint16_t offset = requestOffset(sensor_address);
  Serial.print("read flow offset (HEX, DEC): ");
  Serial.print(offset, HEX);
  Serial.print(", ");
  Serial.println(offset, DEC);
  Serial.print("hardcoded flow offset (DEC): ");
  Serial.println((uint16_t) flow_offset, DEC);
  Serial.println();


  Serial.print("sample rate: ");
  Serial.print(1000.0 / (float)SAMPLE_DELAY);
  Serial.println(" Hz");
  Serial.println();

  startContinuousMeasurement(sensor_address);
  
  delay(5);
}


void loop()
{
  float flow = readFlow(sensor_address);
  Serial.print("flow (slm): ");
  Serial.println(flow);
  delay(SAMPLE_DELAY);
}
