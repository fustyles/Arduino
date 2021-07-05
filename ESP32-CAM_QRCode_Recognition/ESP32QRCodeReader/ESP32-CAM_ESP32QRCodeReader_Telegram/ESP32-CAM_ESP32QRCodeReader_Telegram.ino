/*
ESP32-CAM (Save a captured photo to Telegram)
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

String myToken = "1636695736:AAGLi8kHrFbRWd6gpsq1ToxBTmvxH-UQJ7s";   // Create your bot and get the token -> https://telegram.me/fatherbot
String myChatId = "901901847";       // Get chat_id -> https://telegram.me/chatid_echo_bot

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
  //傳送影像
  Serial.println(sendCapturedImage2LineNotify());
  //傳送文字
  Serial.println(sendRequest2LineNotify("message=\nHello\nWorld"));
  //傳送貼圖
  Serial.println(sendRequest2LineNotify("message=Hello World&stickerPackageId=1&stickerId=2"));
  //傳送網址  
  String imageThumbnail = "https://s2.lookerpets.com/imgs/202008/14/11/15973742786521.jpg";
  String imageFullsize = "https://i.ytimg.com/vi/WLUEXiTAPaI/maxresdefault.jpg";
  Serial.println(sendRequest2LineNotify("message=Hello World&imageThumbnail="+imageFullsize+"&imageFullsize="+imageThumbnail));
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
      Serial.println(sendMessage2Telegram(myToken, myChatId, qrcode));
    } else {
      String qrcode = (const char *)qrCodeData.payload;
      Serial.print("Invalid: ");
      Serial.print(qrcode);
    }
  }
  delay(300);
}

String sendMessage2Telegram(String token, String chat_id, String text) {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = "";
  
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");

    String message = "chat_id="+chat_id+"&text="+text;
    client_tcp.println("POST /bot"+token+"/sendMessage HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Length: " + String(message.length()));
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println();
    client_tcp.print(message);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTime = millis();
    boolean state = false;
    
    while ((startTime + waitTime) > millis())
    {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) 
      {
          char c = client_tcp.read();
          if (state==true) getBody += String(c);        
          if (c == '\n') 
          {
            if (getAll.length()==0) state=true; 
            getAll = "";
          } 
          else if (c != '\r')
            getAll += String(c);
          startTime = millis();
       }
       if (getBody.length()>0) break;
    }
    client_tcp.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  
  return getBody;
}
