/*
ESP32-CAM Using keyboard in Telegram Bot

Author : ChungYi Fu (Kaohsiung, Taiwan)  2021-7-5 15:00
https://www.facebook.com/francefu

ArduinoJson Library：
https://github.com/bblanchon/ArduinoJson

Telegram Bot API
https://core.telegram.org/bots/api
*/

//輸入WIFI連線帳號密碼
const char* ssid     = "teacher";   //your network SSID
const char* password = "87654321";   //your network password

String myToken = "1636695736:AAGLi8kHrFbRWd6gpsq1ToxBTmvxH-UQJ7s";   // Create your bot and get the token -> https://telegram.me/fatherbot
String myChatId = "901901847";                                       // Get chat_id -> https://telegram.me/chatid_echo_bot

/*
If "sendHelp" variable is equal to "true", it will send the command list to Telegram Bot when the board boots. 
If you don't want to get the command list when the board restarts every time, you can set the value to "false".
But you need to input the command "help" or "/help" in Telegram APP to get the command list after the board booting.
*/
boolean sendHelp = false;   


#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"             //用於電源不穩不重開機 
#include "soc/rtc_cntl_reg.h"    //用於電源不穩不重開機 
#include "esp_camera.h"          //視訊函式
#include <ArduinoJson.h>         //解析json格式函式 

//ESP32-CAM 安信可模組腳位設定
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

WiFiClientSecure client_tcp;
long message_id_last = 0;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //關閉電源不穩就重開機的設定
    
  Serial.begin(115200);
  Serial.setDebugOutput(true);  //開啟診斷輸出
  Serial.println();

  //視訊組態設定  https://github.com/espressif/esp32-camera/blob/master/driver/include/esp_camera.h
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
  
  //
  // WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
  //            Ensure ESP32 Wrover Module or other board with PSRAM is selected
  //            Partial images will be transmitted if image exceeds buffer size
  //   
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){  //是否有PSRAM(Psuedo SRAM)記憶體IC
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  //視訊初始化
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  //可自訂視訊框架預設大小(解析度大小)
  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_VGA);    //解析度 UXGA(1600x1200), SXGA(1280x1024), XGA(1024x768), SVGA(800x600), VGA(640x480), CIF(400x296), QVGA(320x240), HQVGA(240x176), QQVGA(160x120), QXGA(2048x1564 for OV3660)

  //s->set_vflip(s, 1);  //垂直翻轉
  //s->set_hmirror(s, 1);  //水平鏡像
          
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
  } 

  //設定閃光燈為低電位
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
                
  //取得Telegram Bot最新訊息
  getTelegramMessage(myToken);  
}

void loop()
{
}

//執行指令
void executeCommand(String text) {
  if (!text||text=="") return;
    
  // 自訂指令
  if (text=="help"||text=="/help"||text=="/start") {
    String command = "/help Command list\n/capture Get still\n/on Turn on the flash\n/off Turn off the flash\n/restart Restart the board";

    //鍵盤按鈕
    //One row
    //String keyboard = "{\"keyboard\":[[{\"text\":\"/on\"},{\"text\":\"/off\"},{\"text\":\"/capture\"},{\"text\":\"/restart\"}]],\"one_time_keyboard\":false}";
    //Two rows
    String keyboard = "{\"keyboard\":[[{\"text\":\"/on\"},{\"text\":\"/off\"}], [{\"text\":\"/capture\"},{\"text\":\"/restart\"}]],\"one_time_keyboard\":false}";
    
    sendMessage2Telegram(myToken, myChatId, command, keyboard);  //傳送功能清單
  } else if (text=="/capture") {  //取得視訊截圖
    sendCapturedImage2Telegram(myToken, myChatId);
  } else if (text=="/on") {  //開啟閃光燈
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    sendMessage2Telegram(myToken, myChatId, "Turn on the flash", "");
  } else if (text=="/off") {  //關閉閃光燈
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,0);
    sendMessage2Telegram(myToken, myChatId, "Turn off the flash", "");
  } else if (text=="/restart") {  //重啟電源
    sendMessage2Telegram(myToken, myChatId, "Restart the board", "");
    ESP.restart();
  } else if (text=="null") {   //不可刪除此條件 Server sends the response unexpectedly. Don't delete the code.
    client_tcp.stop();
    getTelegramMessage(myToken);
  } else
    sendMessage2Telegram(myToken, myChatId, "Command is not defined", "");
}

//取得最新訊息
void getTelegramMessage(String token) {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = ""; 
  JsonObject obj;
  DynamicJsonDocument doc(1024);
  String result;
  long update_id;
  String message;
  long message_id;
  String text;  

  client_tcp.setInsecure();   //run version 1.0.5 or above
  if (message_id_last == 0) Serial.println("Connect to " + String(myDomain));
  if (client_tcp.connect(myDomain, 443)) {
    if (message_id_last == 0) Serial.println("Connection successful");

    while (client_tcp.connected()) { 
      /*
        //新增PIR感測器
        //If you want to add PIR sensor in your project, please put the code here.
        pinMode(pinPIR, INPUT_PULLUP);
        if (digitalRead(pinPIR)==1) {
          sendCapturedImage2Telegram();
        }        
      */
      
      getAll = "";
      getBody = "";

      String request = "limit=1&offset=-1&allowed_updates=message";
      client_tcp.println("POST /bot"+token+"/getUpdates HTTP/1.1");
      client_tcp.println("Host: " + String(myDomain));
      client_tcp.println("Content-Length: " + String(request.length()));
      client_tcp.println("Content-Type: application/x-www-form-urlencoded");
      client_tcp.println("Connection: keep-alive");
      client_tcp.println();
      client_tcp.print(request);
      
      int waitTime = 5000;   // timeout 5 seconds
      long startTime = millis();
      boolean state = false;
      
      while ((startTime + waitTime) > millis()) {
        //Serial.print(".");
        delay(100);      
        while (client_tcp.available()) {
            char c = client_tcp.read();
            if (c == '\n') {
              if (getAll.length()==0) state=true; 
              getAll = "";
            } else if (c != '\r')
              getAll += String(c);
            if (state==true) getBody += String(c);
            startTime = millis();
         }
         if (getBody.length()>0) break;
      }

      //取得最新訊息json格式取值
      deserializeJson(doc, getBody);
      obj = doc.as<JsonObject>();
      //result = obj["result"].as<String>();
      //update_id =  obj["result"][0]["update_id"].as<String>().toInt();
      //message = obj["result"][0]["message"].as<String>();
      message_id = obj["result"][0]["message"]["message_id"].as<String>().toInt();
      text = obj["result"][0]["message"]["text"].as<String>();

      if (message_id!=message_id_last&&message_id) {
        int id_last = message_id_last;
        message_id_last = message_id;
        if (id_last==0) {
          message_id = 0;
          if (sendHelp == true)   // Send the command list to Telegram Bot when the board boots.
            text = "/help";
          else
            text = "";
        } else {
          Serial.println(getBody);
          Serial.println();
        }
        
        if (text!="") {
          Serial.println("["+String(message_id)+"] "+text);
          executeCommand(text);
        }
      }
      delay(1000);
    }
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connected failed.");
    WiFi.begin(ssid, password);  
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      if ((StartTime+10000) < millis())  {
        StartTime=millis();
        WiFi.begin(ssid, password);
      }
    }
    Serial.println("Reconnection is successful.");
  }

  //伺服器約3分鐘斷線，重新取得連線。
  getTelegramMessage(myToken);   // Client's connection time out after about 3 minutes.
}

//傳送視訊截圖
void sendCapturedImage2Telegram(String token, String chat_id) {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }  
  
  String head = "--Taiwan\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + chat_id + "\r\n--Taiwan\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--Taiwan--\r\n";

  uint16_t imageLen = fb->len;
  uint16_t extraLen = head.length() + tail.length();
  uint16_t totalLen = imageLen + extraLen;

  client_tcp.println("POST /bot"+token+"/sendPhoto HTTP/1.1");
  client_tcp.println("Host: " + String(myDomain));
  client_tcp.println("Content-Length: " + String(totalLen));
  client_tcp.println("Content-Type: multipart/form-data; boundary=Taiwan");
  client_tcp.println("Connection: keep-alive");
  client_tcp.println();
  client_tcp.print(head);

  uint8_t *fbBuf = fb->buf;
  size_t fbLen = fb->len;
  for (size_t n=0;n<fbLen;n=n+1024) {
    if (n+1024<fbLen) {
      client_tcp.write(fbBuf, 1024);
      fbBuf += 1024;
    } else if (fbLen%1024>0) {
      size_t remainder = fbLen%1024;
      client_tcp.write(fbBuf, remainder);
    }
  }  
  
  client_tcp.print(tail);
  
  esp_camera_fb_return(fb);
  
  int waitTime = 10000;   // timeout 10 seconds
  long startTime = millis();
  boolean state = false;
  
  while ((startTime + waitTime) > millis()) {
    Serial.print(".");
    delay(100);      
    while (client_tcp.available()) {
        char c = client_tcp.read();
        if (state==true) getBody += String(c);      
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } else if (c != '\r')
          getAll += String(c);
        startTime = millis();
     }
     if (getBody.length()>0) break;
  }
  Serial.println(getBody);
  Serial.println();
}

//傳送訊息與鍵盤按鈕
void sendMessage2Telegram(String token, String chat_id, String text, String keyboard) {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = "";
  
  String request = "parse_mode=HTML&chat_id="+chat_id+"&text="+text;
  if (keyboard!="") request += "&reply_markup="+keyboard;
  
  client_tcp.println("POST /bot"+token+"/sendMessage HTTP/1.1");
  client_tcp.println("Host: " + String(myDomain));
  client_tcp.println("Content-Length: " + String(request.length()));
  client_tcp.println("Content-Type: application/x-www-form-urlencoded");
  client_tcp.println("Connection: keep-alive");
  client_tcp.println();
  client_tcp.print(request);
  
  int waitTime = 5000;   // timeout 5 seconds
  long startTime = millis();
  boolean state = false;
  
  while ((startTime + waitTime) > millis()) {
    Serial.print(".");
    delay(100);      
    while (client_tcp.available()) {
        char c = client_tcp.read();
        if (state==true) getBody += String(c);      
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } else if (c != '\r')
          getAll += String(c);
        startTime = millis();
     }
     if (getBody.length()>0) break;
  }
  Serial.println(getBody);
  Serial.println();
}
