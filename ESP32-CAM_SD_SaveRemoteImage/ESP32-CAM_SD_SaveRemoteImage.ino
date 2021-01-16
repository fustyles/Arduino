#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "FS.h"                  //檔案系統函式
#include "SD_MMC.h"              //SD卡存取函式

// Enter your WiFi ssid and password
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  WiFi.begin(ssid, password);

  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+10000) < millis()) break;
  } 

  if (WiFi.status() == WL_CONNECTED)
  {
    pinMode(2, OUTPUT);
    for (int i=0;i<5;i++)
    {
      digitalWrite(2,HIGH);
      delay(100);
      digitalWrite(2,LOW);
      delay(100);
    }
  } 
  else
  Serial.println("Connection failed.");

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());

  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return;
  }  
    
  getRemoteImage("fustyles.github.io", "/webduino/1.jpg", 443);
}

void loop()
{

}

void getRemoteImage(String domain,String request,int port)
{
  String getResponse="";
  
  File file = SD_MMC.open("/1.jpg", FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file for reading");
    SD_MMC.end();
    return;    
  } else {
    WiFiClientSecure client_tcp;
    
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(domain);
      
    if (client_tcp.connect(domain.c_str(), port)) 
    {
      Serial.println("GET " + request);
      client_tcp.println("GET " + request + " HTTP/1.1");
      client_tcp.println("Host: " + domain);
      client_tcp.println("Content-type:image/jpeg; charset=utf-8");
      client_tcp.println("Connection: close");
      client_tcp.println();
  
      String getResponse="";
      boolean state = false;
      int waitTime = 1000;   // timeout 1 seconds
      long startTime = millis();
      while ((startTime + waitTime) > millis()) {
        while (client_tcp.available()) {
            char c = client_tcp.read();
            
            if (state==true) file.print(c);
            if (c == '\n') {
              if (getResponse.length()==0) 
                state=true;
              getResponse = "";
            } 
            else if (c != '\r') {
              getResponse += String(c);
            }
            

            startTime = millis();
         }
      }
      client_tcp.stop();
    }  
  }
  
  file.close();
  SD_MMC.end();
  
  Serial.print("Save OK");
}
