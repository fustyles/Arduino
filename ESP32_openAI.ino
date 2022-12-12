/* 
NodeMCU (ESP32)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-12-12 23:00
https://www.facebook.com/francefu
*/

#include <WiFi.h>             
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid     = "teacher";   // your network SSID
const char* password = "12345678";   // your network password
String token = "";  // openAI Token

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
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if ((StartTime+10000) < millis()) break;
  } 

  if (WiFi.status() == WL_CONNECTED) {
    pinMode(2, OUTPUT);
    for (int i=0;i<5;i++) {
      digitalWrite(2,HIGH);
      delay(100);
      digitalWrite(2,LOW);
      delay(100);
    }
      
    Serial.println();
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();     
  } else {
    Serial.println("Unable to connect!"); 
    delay(2000);
    ESP.restart();
  }
  
  Serial.println(openAI("How are you?")); 
  Serial.println(openAI("What is your name?")); 
  Serial.println(openAI("Where are you going?"));  
  Serial.println(openAI("請寫出讚美台灣的一首詩"));       
}

void loop()
{
}

String openAI(String request) { 
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above

  Serial.println(request);
  request = "{\"model\":\"text-davinci-003\",\"prompt\":\"" + request + "\",\"temperature\":0.9,\"max_tokens\":1024,\"frequency_penalty\":0,\"presence_penalty\":0.6,\"top_p\":1.0}";
  
  if (client_tcp.connect("api.openai.com", 443)) {
    client_tcp.println("POST /v1/completions HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Type: application/json; charset=utf-8");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    client_tcp.println(request);
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 30000;   // timeout 10 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (state==true) {
            Feedback += String(c);
            if (Feedback.indexOf("\"text\":\"\\n\\n")!=-1)
               Feedback = "";
            if (Feedback.indexOf("\",\"index\"")!=-1) {
              client_tcp.stop();
              Serial.println();
              return Feedback.substring(0,Feedback.length()-9);               
            }
          }
          if (c == '\n') {
            if (getResponse.length()==0) state=true; 
            getResponse = "";
          } 
          else if (c != '\r')
            getResponse += String(c);
          startTime = millis();
       }
       if (getResponse.length()>0) break;
    }
    //Serial.println(Feedback);
    client_tcp.stop();
    return "";
  }
  else
    return "Connection failed";  
}
