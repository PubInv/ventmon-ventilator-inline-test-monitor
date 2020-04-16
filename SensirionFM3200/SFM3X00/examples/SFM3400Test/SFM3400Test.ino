#include <SFM3X00.h>

void setup()
{
  Wire.begin();
  Serial.begin(115200);

  long serialNo = requestSerialNumber();
  Serial.println(serialNo, HEX);
  
  startContinuousMeasurement();
  
  delay(5);
}


void loop()
{
  float flow = readFlow(SFM3400);
  Serial.println(flow);
  delay(550);
}
