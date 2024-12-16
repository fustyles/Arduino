/* 
ESP32 Use Gemini API
Author : ChungYi Fu (Kaohsiung, Taiwan)  2024-12-16 00:00
https://www.facebook.com/francefu

Tutorial
https://ai.google.dev/gemini-api/docs
*/

char wifi_ssid[] = "teacher";
char wifi_pass[] = "12345678";

String Gemini_apikey = "xxxxx";
String Gemini_model = "gemini-1.5-flash";    // "gemini-1.5-flash" or "gemini-pro"
String system_content = "{\"role\": \"model\", \"parts\":[{ \"text\": \"You are a smart assistant.\" }]}";

#include <WiFi.h>
#include <WiFiClientSecure.h>
WiFiClientSecure client;

String historical_messages = system_content;

void initWiFi() {
  for (int i = 0; i < 2; i++) {
    WiFi.begin(wifi_ssid, wifi_pass);
    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    long int StartTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if ((StartTime + 5000) < millis()) break;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("STAIP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");
      break;
    }
  }
}

String Gemini_chat_request(String message) {
  client.setInsecure();
  String user_content = "{\"role\": \"user\", \"parts\":[{ \"text\": \"" + message + "\" }]}";
  historical_messages += ", " + user_content;
  String request = "{\"contents\": [" + historical_messages + "],}";
  
  if (client.connect("generativelanguage.googleapis.com", 443)) {
    client.println("POST /v1beta/models/" + Gemini_model + ":generateContent?key=" + Gemini_apikey + " HTTP/1.1");
    client.println("Connection: close");
    client.println("Host: generativelanguage.googleapis.com");
    client.println("Content-Type: application/json; charset=utf-8");
    client.println("Content-Length: " + String(request.length()));
    client.println();
    
    for (int i = 0; i < request.length(); i += 1024) {
      client.print(request.substring(i, i + 1024));
    }
    
    String getResponse = "", Feedback = "";
    boolean state = false;
    int waitTime = 20000;
    long startTime = millis();
    
    while ((startTime + waitTime) > millis()) {
      delay(100);
      while (client.available()) {
        char c = client.read();
        if (state == true) getResponse += String(c);
        if (c == '\n') Feedback = "";
        else if (c != '\r') Feedback += String(c);
        if (Feedback.indexOf("\",\"text\":\"") != -1) state = true;
        if (Feedback.indexOf("\"},") != -1) state = false;
        startTime = millis();
        if (Feedback.indexOf("\",\"text\":\"") != -1 || Feedback.indexOf("\"text\": \"") != -1) state = true;
        if (getResponse.indexOf("\"\n\r") != -1 && state == true) {
          state = false;
          getResponse = getResponse.substring(0, getResponse.length() - 4);
        } else if (getResponse.indexOf("\"") != -1 && c == '\n' && state == true) {
          state = false;
          getResponse = getResponse.substring(0, getResponse.length() - 4);
        }
      }
      
      if (getResponse.length() > 0) {
        client.stop();
        String assistant_content = "{\"role\": \"model\", \"parts\":[{ \"text\": \"" + getResponse + "\" }]}";
        historical_messages += ", " + assistant_content;
        Serial.println("");
        return getResponse;
      }
    }
    
    client.stop();
    return "error";
  } else {
    return "Connection failed";
  }
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  
  Serial.println((Gemini_chat_request("Hello! I am from Taiwan and my name is France.")));
  Serial.println((Gemini_chat_request("Do you know my name?")));
}

void loop() {
  // Empty loop
}
