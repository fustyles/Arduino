/*
ESP32-CAM DHT11
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-6-25 01:00
https://www.facebook.com/francefu

DHT11 Library
https://github.com/fustyles/Arduino/blob/master/ESP32-CAM_DHT11/DHT.zip
*/

#include <dht.h>   
#define dht_dpin 2   //DHT11 IO2
dht DHT;  

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(10);
}
  
void loop(){   
  DHT.read11(dht_dpin);
  Serial.print("Humidity = ");   
  Serial.print(DHT.humidity);   
  Serial.print("% ");   
  Serial.print("temperature = ");   
  Serial.print(DHT.temperature);   
  Serial.println("C ");   
  delay(2000);  
} 
