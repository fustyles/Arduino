/* 
NodeMCU (ESP32)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-10-25 17:00
https://www.facebook.com/francefu
*/

#include <WiFi.h>             
#include <WiFiClientSecure.h>

const char* ssid     = "*****";   // your network SSID
const char* password = "*****";   // your network password
String token = "*****";  // LineNotify Token

void setup() 
{
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
  {
    Serial.println("Unable to connect!"); 
    delay(2000);
    ESP.restart();
  }
  
  //Push a message to LineNotify
  String message = encodeMessage("message=Test message\nI'm a \"Maker\"");
  String request = message;
  Serial.println(LineNotify(request, 1)); 

  //Push a emoji to LineNotify
  String message_emoji = encodeMessage("message=Test emoji\nI'm a \"Maker\"");    
  String emoji = "&stickerPackageId=1&stickerId=2";
  String request_emoji = message_emoji + emoji;
  Serial.println(LineNotify(request_emoji, 1)); 
  
  //Push a image to LineNotify
  String message_image = encodeMessage("message=Test image\nI'm a \"Maker\"");  
  String imageFullsize = "https://video.nextmag.com.tw/photo/2016/04/25/1461565395_f66f-tile_1461575684432_555961_ver1.0.jpg";
  String imageThumbnail = "https://video.nextmag.com.tw/photo/2016/01/26/B766ENT02-01_1453804306593_487920_ver1.0.jpg";
  String image = "&imageFullsize="+imageFullsize+"&imageThumbnail="+imageThumbnail;
  String request_image = message_image + image;
  Serial.println(LineNotify(request_image, 1)); 
}

void loop()
{
}

String LineNotify(String request, byte wait)
{ 
  WiFiClientSecure client_tcp;
  //client_tcp.setInsecure();   //version 1.0.6
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

String encodeMessage(String message)
{
  message.replace("%","%25");  
  message.replace(" ","%20");
  message.replace("&","%20");
  message.replace("#","%20");
  //message.replace("\'","%27");
  message.replace("\"","%22");
  message.replace("\n","%0D%0A"); 
  return message;
}
