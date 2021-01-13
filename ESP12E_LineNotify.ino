/* 
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-1-1 21:00
https://www.facebook.com/francefu
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
HTTPClient http;

const char* ssid     = "";   //your network SSID
const char* password = "";   //your network password
String token = "";  // LineNotify Token
String fingerprint = """bf16ae79d2ab7144bed8e755a2c70b3968dbb5d2";

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

  //LineNotify push a message
  if (WiFi.status() == WL_CONNECTED) {
    String message = "Taiwan\nI'm a \"Maker\"";
    
    //LineNotify_http_get(message);
    LineNotify_https_post(message);
  }
}

void loop()
{
}

String LineNotify_https_post(String message)
{
  message.replace(" ","%20");
  message.replace("&","%20");
  message.replace("#","%20");
  //message.replace("\'","%27");
  message.replace("\"","%22");
  message.replace("\n","%0D%0A");
  
  http.begin("https://notify-api.line.me/api/notify", fingerprint);
  http.addHeader("Host", "notify-api.line.me");
  http.addHeader("Authorization", "Bearer " + token);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
  http.addHeader("Connection", "close");

  int httpCode = http.POST("message="+message);
  http.end();
  
  if(httpCode > 0) {
      if(httpCode == 200) 
        Serial.println(http.getString());
  } else 
      Serial.println("Connection Error!");
}

String LineNotify_http_get(String message)
{
  message.replace("%","%25");  
  message.replace(" ","%20");
  message.replace("&","%20");
  message.replace("#","%20");
  //message.replace("\'","%27");
  message.replace("\"","%22");
  message.replace("\n","%0D%0A");
  
  http.begin("http://linenotify.com/notify.php?token="+token+"&message="+message);
  int httpCode = http.GET();
  if(httpCode > 0) {
      if(httpCode == 200) 
        Serial.println(http.getString());
  } else 
      Serial.println("Connection Error!");
}
