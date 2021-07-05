/*
ESP32-CAM (Save a captured photo to Line Notify)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2021-7-4 13:30
https://www.facebook.com/francefu

You could only send up to 50 images to Line Notify in one hour.
The maximum size of post-upload image is XGA(1024*768).

Refer to the code
https://github.com/alvarowolfx/ESP32QRCodeReader
*/

// Enter your WiFi ssid and password
const char* ssid     = "teacher";   //your network SSID
const char* password = "87654321";   //your network password

String lineNotifyToken = "";    //Line Notify Token

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"             //用於電源不穩不重開機 
#include "soc/rtc_cntl_reg.h"    //用於電源不穩不重開機 

#include "ESP32CameraPins.h"     //ESP32 Camera腳位定義
#include "ESP32QRCodeReader.h"   //ESP32 QRCode Reader函式

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);   //安信可ESP32-CAM腳位設定
struct QRCodeData qrCodeData;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //關閉電源不穩就重開機的設定
    
  Serial.begin(115200);
  Serial.setDebugOutput(true);  //開啟診斷輸出
  Serial.println();

  //閃光燈(GPIO4)
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);
  
  WiFi.mode(WIFI_STA);  //其他模式 WiFi.mode(WIFI_AP); WiFi.mode(WIFI_STA);

  //指定Client端靜態IP
  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  for (int i=0;i<2;i++) {
    WiFi.begin(ssid, password);    //執行網路連線
  
    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+5000) < millis()) break;    //等待10秒連線
    } 
  
    if (WiFi.status() == WL_CONNECTED) {    //若連線成功
      Serial.println("");
      Serial.println("STAIP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");
  
      for (int i=0;i<5;i++) {   //若連上WIFI設定閃光燈快速閃爍
        ledcWrite(4,10);
        delay(200);
        ledcWrite(4,0);
        delay(200);    
      }
      break;
    }
  } 

  if (WiFi.status() != WL_CONNECTED) {    //若連線失敗
    for (int i=0;i<2;i++) {    //若連不上WIFI設定閃光燈慢速閃爍
      ledcWrite(4,10);
      delay(1000);
      ledcWrite(4,0);
      delay(1000);    
    }
    ESP.restart();
  } 

  /*
  //傳送文字
  Serial.println(sendRequest2LineNotify(lineNotifyToken, "message=\nHello\nWorld"));
  //傳送貼圖
  Serial.println(sendRequest2LineNotify(lineNotifyToken, "message=Hello World&stickerPackageId=1&stickerId=2"));
  //傳送網址  
  String imageThumbnail = "https://s2.lookerpets.com/imgs/202008/14/11/15973742786521.jpg";
  String imageFullsize = "https://i.ytimg.com/vi/WLUEXiTAPaI/maxresdefault.jpg";
  Serial.println(sendRequest2LineNotify(lineNotifyToken, "message=Hello World&imageThumbnail="+imageFullsize+"&imageFullsize="+imageThumbnail));
  */

  reader.setup();
  //reader.setDebug(true);
  Serial.println("Setup QRCode Reader");

  reader.begin();
  Serial.println("Begin QR Code reader");  
}

void loop() {
  if (reader.receiveQrCode(&qrCodeData, 100)) {
    if (qrCodeData.valid) {
      String qrcode = (const char *)qrCodeData.payload;
      Serial.print("Payload: ");
      Serial.println(qrcode);

      //上傳Line Notify
      Serial.println(sendRequest2LineNotify(lineNotifyToken, "message=\n"+qrcode));
    } else {
      String qrcode = (const char *)qrCodeData.payload;
      Serial.print("Invalid: ");
      Serial.print(qrcode);
    }
  }
  delay(300);
}


String sendRequest2LineNotify(String token, String request) {
  request.replace("%","%25");
  request.replace(" ","%20");
  //request.replace("&","%20");
  request.replace("#","%20");
  //request.replace("\'","%27");
  request.replace("\"","%22");
  request.replace("\n","%0D%0A");
  request.replace("%3Cbr%3E","%0D%0A");
  request.replace("%3Cbr/%3E","%0D%0A");
  request.replace("%3Cbr%20/%3E","%0D%0A");
  request.replace("%3CBR%3E","%0D%0A");
  request.replace("%3CBR/%3E","%0D%0A");
  request.replace("%3CBR%20/%3E","%0D%0A"); 
  request.replace("%20stickerPackageId","&stickerPackageId");
  request.replace("%20stickerId","&stickerId");    

  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above
  
  Serial.println("Connect to notify-api.line.me");  
  
  if (client_tcp.connect("notify-api.line.me", 443)) {
    Serial.println("Connection successful");
        
    //Serial.println(request);    
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
    return Feedback;
  }
  else
    return "Connection failed";  
}
