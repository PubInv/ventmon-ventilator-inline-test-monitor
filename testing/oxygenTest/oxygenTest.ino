#include <Wire.h>
#include <Adafruit_ADS1015.h>

#define AVG_WINDOW 20

// channel the O2 sensor os connected to
#define O2CHANNEL 3

// The ADC input range (or gain) can be changed via the following
// functions, but be careful never to exceed VDD +0.3V max, or to
// exceed the upper and lower limits if you adjust the input range!
// Setting these values incorrectly may destroy your ADC!
//                                                                ADS1015  ADS1115
//                                                                -------  -------
// ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
// ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
// ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
// ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
// ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
// ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

int initialO2;

void setup(void)
{
  Serial.begin(9600);
  
  ads.setGain(GAIN_SIXTEEN);      

  ads.begin();

  initialO2 = avgADC(O2CHANNEL);
  if(initialO2 <= 100 && initialO2 >= -100)
  {
    Serial.println("Oxygen Sensor is not installed. Proceeding without oxygen sensing.");
  }

  Serial.println(initialO2);
}

void loop(void)
{
  double voltage = avgADC(O2CHANNEL);

  float voltagemV = voltage * 0.0078125;
  
  float o2 = (voltage / initialO2) * 20.9;

  Serial.print("\ninital: ");
  Serial.println(initialO2);
  Serial.print("raw adc: ");
  Serial.println(voltage);
  Serial.print("mV: ");
  Serial.println(voltagemV);
  Serial.print("O2: ");
  Serial.println(o2);
 
  Serial.println("-----");

  delay(1000);
}

void printRawmV(int adcNumber)
{
  int16_t adc = 0;

  adc = ads.readADC_SingleEnded(adcNumber);
  Serial.print("ADC"); 
  Serial.print(adcNumber); 
  Serial.print(": "); 
  Serial.print(adc, HEX);
  Serial.print(", ");
  Serial.println(adc);
}

double avgADC(int adcNumber)
{
  double adc = 0;
  double result;

  adc = 0;
  
  for (int i = 0; i <= AVG_WINDOW - 1; i++)
  {
    int16_t adsread = ads.readADC_SingleEnded(adcNumber);
    adc = adc + adsread;
    delay(10);
  }

  result = adc / AVG_WINDOW;
  
  return result;
}

void checkAllChannels()
{
  double voltage0 = 0;
  double voltage1 = 0;
  double voltage2 = 0;
  double voltage3 = 0;

  Serial.println("\n0: ");
  printRawmV(0);
  voltage0 = avgADC(0);
  Serial.println(voltage0);

  Serial.println("\n1: ");
  printRawmV(1);  
  voltage1 = avgADC(1);
  Serial.println(voltage1);  

  Serial.println("\n2: ");
  printRawmV(2);  
  voltage2 = avgADC(2);
  Serial.println(voltage2);

  Serial.println("\n3: ");
  printRawmV(3);
  voltage3 = avgADC(3);
  Serial.println(voltage3);
}
