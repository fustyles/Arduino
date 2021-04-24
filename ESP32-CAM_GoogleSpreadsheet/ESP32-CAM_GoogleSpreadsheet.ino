/*
ESP32-CAM 影像上傳Google試算表
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-5-28 23:00
https://www.facebook.com/francefu

如何新增Google Script
https://www.youtube.com/watch?v=f46VBqWwUuI

Google Script管理介面
https://script.google.com/home
https://script.google.com/home/executions

Google Script程式碼

function doPost(e) {
  var myFile = e.parameter.myFile;  //取得影像檔
  var myFilename = Utilities.formatDate(new Date(), "GMT", "yyyyMMddHHmmss")+"_"+e.parameter.myFilename;  //取得影像檔名
  var mySpreadsheetId = e.parameter.mySpreadsheetId;  //試算表Id
  var myCellRow = e.parameter.myCellRow;   //取得插入影像儲存格Row
  var myCellCol = e.parameter.myCellCol;   //取得插入影像儲存格Col 
  
  var contentType = myFile.substring(myFile.indexOf(":")+1, myFile.indexOf(";"));
  var data = myFile.substring(myFile.indexOf(",")+1);
  data = Utilities.base64Decode(data);
  var blob = Utilities.newBlob(data, contentType, myFilename);

  var ss = SpreadsheetApp.openById(mySpreadsheetId);
  ss.getActiveSheet().setHiddenGridlines(true);
  var sheet = ss.getSheets()[0];
  sheet.insertImage(blob, myCellRow, myCellCol);

  var images = sheet.getImages();
  while (images.length>2) {   //影像數超過兩張即刪除最早影像，僅保留最新兩張影像
    images[0].remove();
  }
     
  return  ContentService.createTextOutput("ok");
}

*/

//輸入Wi-Fi帳密
const char* ssid     = "*****";   //Wi-Fi帳號
const char* password = "*****";   //Wi-Fi密碼

String myScript = "/macros/s/********************/exec";    //Create your Google Apps Script and replace the "myScript" path.
String myFilename = "&myFilename=ESP32-CAM.jpg";
String myImage = "&myFile=";

//How to get Spreadsheet Id from spreadsheet url?  
//https://docs.google.com/spreadsheets/d/*****SpreadsheetId*****/edit#gid=0
String mySpreadsheetId = "&mySpreadsheetId=********************";  //Google Spreadsheet Id

String myCellRow = "&myCellRow=1";
String myCellCol = "&myCellCol=1";

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"  //不可使用Arduino IDE內建的函式庫，請從github下載Base64.cpp, Base64.h置於同一資料夾
#include "esp_camera.h"

//Arduino IDE開發版選擇 ESP32 Wrover Module

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //關閉電壓不穩時重啟電源設定
  
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);

  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    if ((StartTime+10000) < millis()) break;
  } 

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());
    
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reset");
    
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    delay(200);
    ledcWrite(3,0);
    delay(200);    
    ledcDetachPin(3);
        
    delay(1000);
    ESP.restart();   //若未連上Wi-Fi閃燈兩次後重啟
  }
  else {
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    for (int i=0;i<5;i++) {   //若連上Wi-Fi閃燈五次
      ledcWrite(3,10);
      delay(200);
      ledcWrite(3,0);
      delay(200);    
    }
    ledcDetachPin(3);      
  }

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  //可自訂解析度
  s->set_framesize(s, FRAMESIZE_QVGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
}

void loop()
{
  SendCapturedImage2Spreadsheet();
  delay(10000);
}

String SendCapturedImage2Spreadsheet() {
  const char* myDomain = "script.google.com";  
  String getAll="", getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }
        
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  //client_tcp.setInsecure();   //version 1.0.6
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
        
      char *input = (char *)fb->buf;
      char output[base64_enc_len(3)];
      String imageFile = "data:image/jpeg;base64,";
      for (int i=0;i<fb->len;i++) {
        base64_encode(output, (input++), 3);
        if (i%3==0) imageFile += urlencode(String(output));
      }
      String Data = myFilename+mySpreadsheetId+myCellRow+myCellCol+myImage;
      
      client_tcp.println("POST " + myScript + " HTTP/1.1");
      client_tcp.println("Host: " + String(myDomain));
      client_tcp.println("Connection: keep-alive");
      client_tcp.println("Content-Length: " + String(Data.length()+imageFile.length()));
      client_tcp.println("Content-Type: application/x-www-form-urlencoded");
      client_tcp.println("Connection: keep-alive");
      client_tcp.println();
      
      client_tcp.print(Data);
      int Index;
      for (Index = 0; Index < imageFile.length(); Index = Index+1000) {
        client_tcp.print(imageFile.substring(Index, Index+1000));
      }
      esp_camera_fb_return(fb);
      
      int waitTime = 5000;   // timeout 10 seconds
      long startTime = millis();
      boolean state = false;
      
      while ((startTime + waitTime) > millis())
      {
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
  }
  else {
    Serial.println("Connected to " + String(myDomain) + " failed.");
    return "Connected to " + String(myDomain) + " failed.";
  }

  return getBody;
}

//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}
