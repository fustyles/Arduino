/* 
LinkIt7697 (Line Notify)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-11-14 00:00
https://www.facebook.com/francefu
*/

const char* ssid     = "teacher";   //your network SSID
const char* password = "12345678";   //your network password
String token = "";

#include <LWiFi.h>
WiFiServer server(80);

static const char rootCA[] = "-----BEGIN CERTIFICATE-----\r\n"
"MIIDBjCCAe6gAwIBAgIBATANBgkqhkiG9w0BAQsFADA8MRYwFAYDVQQDDA1teWVz\r\n"
"cDMyLmxvY2FsMRUwEwYDVQQKDAxGYW5jeUNvbXBhbnkxCzAJBgNVBAYTAkRFMB4X\r\n"
"DTE5MDEwMTAwMDAwMFoXDTMwMDEwMTAwMDAwMFowPDEWMBQGA1UEAwwNbXllc3Az\r\n"
"Mi5sb2NhbDEVMBMGA1UECgwMRmFuY3lDb21wYW55MQswCQYDVQQGEwJERTCCASIw\r\n"
"DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANpf4boK1Eox4tNBrePNiSwFs4D5\r\n"
"9vDd/j/HuLtxiQlvcP6k7+jeoVF8dYUh6wzi2o53jTB1FeFJqeJz0SmlFBxVf/Gg\r\n"
"nQZ4ARYZ/EaEgF3/Vec8yEtuPYKC8+REGzmj667T/3Gyqw4rSin11KrVg2NIxNeE\r\n"
"BvDRn7kSIPseCbtlbJaif1XLokeOFyynbr6Cr+H6Co9QPe3SOP4yhrW8efsw5Plj\r\n"
"CrfIacWLB6XiUipPrbV/T5vYh5IuYTjAq1EWrhSv46IyTOQoNV5OEph6ELR3iNwC\r\n"
"StLF/nI97CQPE95vU/N6i7q4sOB9eBRYth6vKzd/HIvgfJ0rNlGqJzQOh60CAwEA\r\n"
"AaMTMBEwDwYDVR0TBAgwBgEB/wIBADANBgkqhkiG9w0BAQsFAAOCAQEA2Q4sNW4D\r\n"
"X4KLIulCZrv0rAR5byGhIQ43nYdMEntlb9Aeb6Pb1wCC0xhVrknce+9jWFimRTVo\r\n"
"EqBPuwb6gXyDtSlwVEAJG+NXQ52x9s/ymbXLQ0zRvRvR6DmHmvjUmxZIsIM2oj1R\r\n"
"cHsyzIStZpWslbdNkZqSxoC3AEzkdcJEYIgiyPOlFM3/cG09no8rpCwPcHLGxCtA\r\n"
"2wzd/z3F4ywxaysmdqVWOizVJOPIN5B5X5wn/fC9n3VuY93K/9Qp1v+TO7YJWsP/\r\n"
"h7ZwlOsXbj2+uGtsXL9jEsl0L9pEmHPQDXX6GUlTPabQTiAIJTIkASh46HOw4XiE\r\n"
"16lLWc41LGJriA==\r\n"
"-----END CERTIFICATE-----\r\n";

void setup()
{
    Serial.begin(9600);
    delay(10);
    //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

    WiFi.begin(ssid, password);

    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+10000) < millis()) break;
    } 
  
    if (WiFi.status() == WL_CONNECTED) {
      pinMode(LED_BUILTIN, OUTPUT);
      for (int i=0;i<5;i++) {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN,LOW);
        delay(100);
      }

    } else {
      pinMode(LED_BUILTIN, OUTPUT);
      for (int i=0;i<3;i++) {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN,LOW);
        delay(500);
      }
    }

    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
  
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
  String imageFullsize = "https://img.soundofhope.org/2020-09/lzl-1600614896059.jpg";
  String imageThumbnail = "https://img.ltn.com.tw/Upload/ent/page/800/2022/05/25/phpsBPOHo.jpg";
  String image = "&imageFullsize="+imageFullsize+"&imageThumbnail="+imageThumbnail;
  String request_image = message_image + image;
  Serial.println(LineNotify(request_image, 1)); 
}

void loop()
{
}

String LineNotify(String request, byte wait) { 
  TLSClient client_tcp;
  client_tcp.setRootCA(rootCA, sizeof(rootCA));
  
  if (client_tcp.connect("notify-api.line.me", 443)) {
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
    while ((startTime + waitTime) > millis()) {
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (state==true) Feedback += String(c);        
          if (c == '\n') {
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

String encodeMessage(String message) {
  message.replace("%","%25");  
  message.replace(" ","%20");
  message.replace("&","%20");
  message.replace("#","%20");
  //message.replace("\'","%27");
  message.replace("\"","%22");
  message.replace("\n","%0D%0A"); 
  return message;
}
