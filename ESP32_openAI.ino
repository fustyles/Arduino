/* 
NodeMCU (ESP32)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-12-12 15:30
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
  
  Serial.println("\n\nAI: " + openAI("How are you?", 1)); 
  Serial.println("\n\nAI: " + openAI("What is your name?", 1)); 
  Serial.println("\n\nAI: " + openAI("Where are you going?", 1));     
}

void loop()
{
}

String openAI(String request, byte wait) { 
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above

  Serial.println("\n\nME: \n"+request);
  request = "{\"model\":\"text-davinci-003\",\"prompt\":\""+request+"\",\"temperature\":0.9,\"max_tokens\":1024,\"frequency_penalty\":0,\"presence_penalty\":0.6,\"top_p\":1.0}";
      
  if (client_tcp.connect("api.openai.com", 443)) {
    client_tcp.println("POST /v1/completions HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("User-Agent: ESP8266/1.0");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Type: application/json");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    client_tcp.println(request);
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 3000;   // timeout 3 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis()) {
      while (client_tcp.available())  {
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

    JsonObject obj;
    DynamicJsonDocument doc(1024); 
    deserializeJson(doc, Feedback);
    obj = doc.as<JsonObject>();
     
    return obj["choices"][0]["text"].as<String>(); 
  }
  else
    return "Connection failed";  
}
