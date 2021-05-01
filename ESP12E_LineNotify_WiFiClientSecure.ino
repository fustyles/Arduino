/* 
Author : ChungYi Fu (Kaohsiung, Taiwan)  2021-4-22 20:30
https://www.facebook.com/francefu
*/

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

const char* ssid     = "";   // your network SSID
const char* password = "";   // your network password
String token = "";  // LineNotify Token

const char fingerprint[] PROGMEM = "BF 16 AE 79 D2 AB 71 44 BE D8 E7 55 A2 C7 0B 39 68 DB B5 D2";

void setup() {
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(1000);
  Serial.println();
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
      
    Serial.println();
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();     
  }  
  else
     Serial.println("Unable to connect!"); 

  //Push a message to LineNotify
  if (WiFi.status() == WL_CONNECTED) {
    String request = "message=Taiwan\nI'm a \"Maker\"";
    request.replace("%","%25");    
    request.replace(" ","%20");
    request.replace("&","%20");
    request.replace("#","%20");
    //request.replace("\'","%27");
    request.replace("\"","%22");
    request.replace("\n","%0D%0A");
    
    request += "&stickerPackageId=1&stickerId=2";
    String Response = LineNotify(request, 1);
    Serial.println(Response);
  }
}

void loop()
{
}

String LineNotify(String request, byte wait)
{
  WiFiClientSecure client_tcp;
  client_tcp.setFingerprint(fingerprint);
  client_tcp.setInsecure();   //run version 1.0.5 or above
  
  if (client_tcp.connect("notify-api.line.me", 443)) 
  {
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("User-Agent: ESP8266/1.0");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    client_tcp.println(request);
    client_tcp.println();
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 3000;   // timeout 3 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis())
    {
      while (client_tcp.available()) 
      {
          char c = client_tcp.read();
          if (state==true) Feedback += String(c);        
          if (c == '\n') 
          {
            if (getResponse.length()==0) state=true; 
            getResponse = "";
          } 
          else if (c != '\r')
            getResponse += String(c);
          if (wait==1)
            startTime = millis();
       }
       if (wait==0)
        if ((state==true)&&(Feedback.length()!= 0)) break;
    }
    client_tcp.stop();
    return Feedback;
  }
  else
    return "Connection failed";  
}
