/* 
NodeMCU (ESP32)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-12-12 21:30
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
}

void loop()
{
}

String openAI(String request) { 
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above

  Serial.println("\n\nME: \n"+request);
  request = "{\"model\":\"text-davinci-003\",\"prompt\":\"" + request + "\",\"temperature\":0.9,\"max_tokens\":800,\"frequency_penalty\":0,\"presence_penalty\":0.6,\"top_p\":1.0}";
      
  if (client_tcp.connect("api.openai.com", 443)) {
    client_tcp.println("POST /v1/completions HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("User-Agent: ESP8266/1.0");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Type: application/json; charset=utf-8");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    client_tcp.println(request);
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 3000;   // timeout 3 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (state==true) Feedback += String(c);        
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
    Serial.println();
    client_tcp.stop();

    JsonObject obj;
    DynamicJsonDocument doc(1024); 
    deserializeJson(doc, Feedback);
    obj = doc.as<JsonObject>();
     
    return obj["choices"][0]["text"].as<String>(); 
  }
  else
    return "Connection failed";  
}

//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str) {
    String encodedString="";
    char c;
    char code0;
    char code1;
    for (int i =0; i < str.length(); i++) {
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        encodedString+="%";
        encodedString+=code0;
        encodedString+=code1;
      }
      yield();
    }
    return encodedString;
}
