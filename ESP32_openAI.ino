/* 
ESP32 Use GPT-3.5 with the OpenAI API
Author : ChungYi Fu (Kaohsiung, Taiwan)  2023-4-1 23:30
https://www.facebook.com/francefu

Tutorial
https://beta.openai.com/docs/guides/completion
https://beta.openai.com/docs/api-reference/completions/create

Page
https://fustyles.github.io/webduino/openAI.html
*/

#include <WiFi.h>             
#include <WiFiClientSecure.h>

const char* ssid     = "teacher";   // your network SSID
const char* password = "87654321";   // your network password
String openaiKey = "";  // openAI API Key
String role = "You are a helpful assistant.";

String model = "gpt-3.5-turbo";
String system_content = "{\"role\": \"system\", \"content\":\""+ role +"\"}";
String historical_messages = system_content;

void setup() 
{
  Serial.begin(115200);
  delay(10);
  
  initWiFi();

  Serial.println("\nYou can call me Mike Fu and I live in Taiwan.");
  Serial.println(openAI_chat("You can call me Mike Fu and I live in Taiwan.")); 
  Serial.println("\nWhat is my name?"); 
  Serial.println(openAI_chat("What is my name?")); 
  Serial.println("\nWhere am I?"); 
  Serial.println(openAI_chat("Where am I?"));     
}

void loop()
{
}

void initWiFi() {
  WiFi.mode(WIFI_STA);

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

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
      Serial.println(WiFi.localIP());
      Serial.println("");
  
      pinMode(2, OUTPUT);
      for (int j=0;j<5;j++) {
        digitalWrite(2,HIGH);
        delay(100);
        digitalWrite(2,LOW);
        delay(100);
      }
      
      break;
    }
  } 

  if (WiFi.status() != WL_CONNECTED) {    //若連線失敗
    pinMode(2, OUTPUT);
    for (int k=0;k<2;k++) {
      digitalWrite(2,HIGH);
      delay(1000);
      digitalWrite(2,LOW);
      delay(1000);
    }
  } 
}

String openAI_chat(String message) { 
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above

  String user_content = "{\"role\": \"user\", \"content\":\""+ message+"\"}";
  historical_messages += ", "+user_content;
  String request = "{\"model\":\""+model+"\",\"messages\":[" + historical_messages + "]}";

  if (client_tcp.connect("api.openai.com", 443)) {
    client_tcp.println("POST /v1/chat/completions HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("Authorization: Bearer " + openaiKey);
    client_tcp.println("Content-Type: application/json; charset=utf-8");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    for (int i = 0; i < request.length(); i += 1024) {
      client_tcp.print(request.substring(i, i+1024));
    }
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 20000;   // timeout 20 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) {
          char c = client_tcp.read();
          //Serial.print(String(c));
          if (state==true) 
            getResponse += String(c);
          if (c == '\n')
            Feedback = "";
          else if (c != '\r')
            Feedback += String(c);
          if (Feedback.indexOf("\",\"content\":\"")!=-1)
            state=true; 
          if (Feedback.indexOf("\"},")!=-1)
            state=false;            
          startTime = millis();
       }
       if (getResponse.length()>0) {
          client_tcp.stop();
          getResponse = getResponse.substring(0,getResponse.length()-3);
          String assistant_content = "{\"role\": \"assistant\", \"content\":\""+ getResponse+"\"}";
          historical_messages += ", "+assistant_content;
          Serial.println("");
          return getResponse;
       }
    }
    
    client_tcp.stop();
    Serial.println(Feedback);
    return "error";
  }
  else
    return "Connection failed";  
}

void openAI_chat_reset() {
  historical_messages = system_content;
}
