/*
ESP32 Make your line bot send messages to users
Author : ChungYi Fu (Kaohsiung, Taiwan)  2024-11-4 17:00
https://www.facebook.com/francefu
*/

String linebot_token = "";    // Channel access token
String linebot_userid = "";   // userId or groupId

char wifi_ssid[] = "teacher";
char wifi_pass[] = "12345678";

#include <WiFi.h>
#include <WiFiClientSecure.h>
WiFiClientSecure client;

void initWiFi() {
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

      break;
    }
  }
}

void LineBotText(String token, String userid, String message){
  String Data = "{\"to\":\""+userid+"\",\"messages\":[{\"type\":\"text\",\"text\":\""+message+"\"}]}";
  LineBotRequest(token, Data);
}

void LineBotSticker(String token, String userid, int packageId, int stickerId){
  String Data = "{\"to\":\""+userid+"\",\"messages\":[{\"type\":\"sticker\",\"packageId\":"+String(packageId)+",\"stickerId\":"+String(stickerId)+"}]}";
  LineBotRequest(token, Data);
}

void LineBotImage(String token, String userid, String originalContentUrl, String previewImageUrl){
  String Data = "{\"to\":\""+userid+"\",\"messages\":[{\"type\":\"image\",\"originalContentUrl\":\""+originalContentUrl+"\",\"previewImageUrl\":\""+previewImageUrl+"\"}]}";
  LineBotRequest(token, Data);
}

String LineBotRequest(String token, String Data) {
  String getAll="", getBody="";
  client.setInsecure();  
  if (client.connect("api.line.me", 443)) {
    client.println("POST /v2/bot/message/push HTTP/1.1");
    client.println("Connection: close");
    client.println("Host: api.line.me");
    client.println("Authorization: Bearer " + token);
    client.println("Content-Type: application/json; charset=utf-8");
    client.println("Content-Length: " + String(Data.length()));
    client.println();
    client.println(Data);
    client.println();
    boolean state = false;
    long startTime = millis();
    while ((startTime + 3000) > millis()) {
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length()==0) state=true;
           getAll = "";
        }
        else if (c != '\r')
          getAll += String(c);
          if (state==true) getBody += String(c);
          startTime = millis();
        }
        if (getBody.length()!= 0) break;
      }
      client.stop();
      //Serial.println(getBody);
  }
  else {
    getBody="Connected to api.line.me failed.";
    Serial.println("Connected to api.line.me failed.");
  }
  return getBody;
}

void setup()
{
  Serial.begin(115200);
  initWiFi();
  // LineBotText(String token, String userid, String message)
  LineBotText(linebot_token, linebot_userid, "Hello World");
  
  // LineBotSticker(String token, String userid, int packageId, int stickerId)
  LineBotSticker(linebot_token, linebot_userid, 1, 2);
  
  // LineBotImage(String token, String userid, String originalContentUrl, String previewImageUrl)
  LineBotImage(linebot_token, linebot_userid, "https://cdn2.ettoday.net/images/4507/d4507139.jpg", "https://cdn2.ettoday.net/images/4507/d4507139.jpg");
}

void loop()
{

}
