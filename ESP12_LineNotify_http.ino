#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
HTTPClient http;

const char* ssid     = "";   //your network SSID
const char* password = "";   //your network password
String token = "";  // LineNotify Token

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

  if (WiFi.localIP().toString()!="0.0.0.0")
  {
    pinMode(2, OUTPUT);
    for (int i=0;i<5;i++)
    {
      digitalWrite(2,LOW);
      delay(100);
      digitalWrite(2,HIGH);
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
  if (WiFi.localIP().toString()!="0.0.0.0") {
    String message = "Taiwan\nI\'m a \"Maker\"";
    message.replace(" ","%20");
    message.replace("&","%20");
    message.replace("#","%20");
    message.replace("\'","%27");
    message.replace("\"","%22");
    message.replace("\n","%0D%0A");
    LineNotify(token, message);
  }
}

void loop()
{
}

String LineNotify(String token, String message)
{
  http.begin("http://linenotify.com/notify.php?token="+token+"&message="+message);
  int httpCode = http.GET();
  if(httpCode > 0) {
      if(httpCode == 200) 
        Serial.println(http.getString());
  } else 
      Serial.println("Connection Error!");
}
