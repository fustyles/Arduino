/*
ESP32 NTP SERVER
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-5-17 21:30
https://www.facebook.com/francefu
*/

const char* ssid = "teacher";
const char* password = "87654321";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;

int currentTimeValue[6] = {0,0,0,0,0,0};
String currentTime[3] = {"","",""};
#include <WiFi.h>
#include "time.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void getLocalTime()
{
  if(!getLocalTime(&timeinfo)){
    return;
  }
  
  currentTimeValue[0] = timeinfo.tm_year+1900;
  currentTimeValue[1] = timeinfo.tm_mon+1;
  currentTimeValue[2] = timeinfo.tm_mday;
  currentTimeValue[3] = timeinfo.tm_hour;
  currentTimeValue[4] = timeinfo.tm_min;
  currentTimeValue[5] = timeinfo.tm_sec;
  currentTime[0] = String(timeinfo.tm_year+1900)+"/"+ String(timeinfo.tm_mon+1)+"/"+ String(timeinfo.tm_mday);
  currentTime[1] = String(timeinfo.tm_hour)+":"+ String(timeinfo.tm_min)+":"+ String(timeinfo.tm_sec);
  currentTime[2] = String(timeinfo.tm_year+1900)+"/"+ String(timeinfo.tm_mon+1)+"/"+ String(timeinfo.tm_mday) + " "+ String(timeinfo.tm_hour)+":"+ String(timeinfo.tm_min)+":"+ String(timeinfo.tm_sec);
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  initWiFi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  getLocalTime();
  Serial.println(currentTime[0]);
  Serial.println(currentTime[1]);
  Serial.println(currentTime[2]);
  Serial.printf("%d/%d/%d %d:%d:%d",currentTimeValue[0],currentTimeValue[1],currentTimeValue[2],currentTimeValue[3],currentTimeValue[4],currentTimeValue[5]);
}

void loop() {
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  for (int i=0;i<2;i++) {
    WiFi.begin(ssid, password);
    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("STAIP address: ");
      Serial.println("");
     pinMode(2, OUTPUT);
     for (int i=0;i<5;i++) {
       digitalWrite(2, HIGH);
       delay(100);
       digitalWrite(2, LOW);
       delay(100);
     }
      break;
    }
  }
  if (WiFi.status() != WL_CONNECTED) {
    pinMode(2, OUTPUT);
    for (int i=0;i<3;i++) {
      digitalWrite(2, HIGH);
      delay(500);
      digitalWrite(2, LOW);
      delay(500);
    }
  }
  Serial.println("");
}
