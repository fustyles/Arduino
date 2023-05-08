/* 
ESP32 Use ChatGPT API (GPT-3.5)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2023-5-5 19:30
https://www.facebook.com/francefu

Tutorial
https://beta.openai.com/docs/guides/completion
https://beta.openai.com/docs/api-reference/completions/create
*/

#include <WiFi.h>             
#include <WiFiClientSecure.h>

char wifi_ssid[] = "teacher";
char wifi_pass[] = "87654321";
String openaiKey = "";  // openAI API Key

//ChatGPT
String role = "You are a helpful assistant.";
String model = "gpt-3.5-turbo";
String system_content = "{\"role\": \"system\", \"content\":\""+ role +"\"}";
String historical_messages = system_content;

//Image generator
String imageSize = "256x256"; // 256x256, 512x512 , 1024x1024

void setup() 
{
  Serial.begin(115200);
  delay(10);
  
  initWiFi();

  Serial.println(openAI_chat("I am from Taiwan."));
  Serial.println(openAI_chat("Where am I?"));
  Serial.println(openAI_image("Beautiful Taiwan."));   
}

void loop()
{
}

void initWiFi() {
  WiFi.mode(WIFI_STA);

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  for (int i=0;i<2;i++) {
    WiFi.begin(wifi_ssid, wifi_pass);
  
    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);
    
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

  if (WiFi.status() != WL_CONNECTED) {
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
  
  message.replace("\"","'");
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

String openAI_image(String message) { 
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above

  String request = "{\"prompt\":\""+ message+"\", \"size\":\""+imageSize+"\", \"n\":1}";
  if (client_tcp.connect("api.openai.com", 443)) {
    client_tcp.println("POST /v1/images/generations HTTP/1.1");
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
          if (String(c)=="\""&&state==true)
            break;           
          if (state==true)
            getResponse += String(c);
          if (c == '\n')
            Feedback = "";
          else if (c != '\r')
            Feedback += String(c);
          if (Feedback.indexOf("\"url\": \"")!=-1)
            state=true;             
          startTime = millis();
       }
       if (getResponse.length()>0) {
          client_tcp.stop();
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
