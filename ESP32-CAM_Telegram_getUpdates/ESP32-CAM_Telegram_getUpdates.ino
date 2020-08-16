/*
ESP32-CAM Get your latest message from Telegram Bot
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-8-16 09:30
https://www.facebook.com/francefu

ArduinoJson Library：
https://github.com/bblanchon/ArduinoJson

Telegram Bot API
https://core.telegram.org/bots/api
*/

// Enter your WiFi ssid and password
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

String token = "*****:*****";   // Create your bot and get the token -> https://telegram.me/fatherbot
String chat_id = "*****";   // Get chat_id -> https://telegram.me/chatid_echo_bot

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
  
// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled

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

WiFiClientSecure client_tcp;
int message_id_last = 0;

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
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
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    delay(200);
    ledcWrite(3,0);
    delay(200);    
    ledcDetachPin(3);  
    
    ESP.restart();
  }
  else {
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    for (int i=0;i<5;i++) {
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
  s->set_framesize(s, FRAMESIZE_CIF);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

  //Get your latest message from Telegram Bot.
  getTelegramMessage();  
}

void loop()
{
}

void getTelegramMessage() {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = ""; 
  JsonObject obj;
  DynamicJsonDocument doc(1024);

  Serial.println("Connect to " + String(myDomain));
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    delay(2000);
    ledcWrite(3,0);
      
    while (client_tcp.connected()) {            
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
      
      while ((startTime + waitTime) > millis())
      {
        //Serial.print(".");
        delay(100);      
        while (client_tcp.available()) 
        {
            char c = client_tcp.read();
            if (c == '\n') 
            {
              if (getAll.length()==0) state=true; 
              getAll = "";
            } 
            else if (c != '\r')
              getAll += String(c);
            if (state==true) getBody += String(c);
            startTime = millis();
         }
         if (getBody.length()>0) break;
      }
      
      deserializeJson(doc, getBody);
      obj = doc.as<JsonObject>();
      String result = obj["result"];
      long update_id = obj["result"][0]["update_id"];
      String message = obj["result"][0]["message"];
      int message_id = obj["result"][0]["message"]["message_id"];
      String text = obj["result"][0]["message"]["text"];

      if (message_id!=message_id_last&&message_id) {
        int id_last = message_id_last;
        message_id_last = message_id;
        if (id_last==0) {
          message_id = 0;
          text = "/help";      
        }
        else {
          Serial.println(getBody);
          Serial.println();
        }
          
        //Serial.println(String(update_id));
        //Serial.println(String(message_id));
        //Serial.println(text);
        Serial.println("["+String(message_id)+"] "+text);
        
        // If client gets new message, do what you want to do.
        if (text=="help"||text=="/help"||text=="/start") {
          sendMessage2Telegram("/help Command list\n/capture Take a photo\n/on Turn on the flash\n/off Turn off the flash\n/restart Restart the board");
        }        
        else if (text=="/capture") {
          sendCapturedImage2Telegram();
        }
        else if (text=="/on") {
          ledcAttachPin(4, 3);
          ledcSetup(3, 5000, 8);
          ledcWrite(3,10);
          sendMessage2Telegram("Turn on the flash");
        }
        else if (text=="/off") {
          ledcAttachPin(4, 3);
          ledcSetup(3, 5000, 8);
          ledcWrite(3,0);
          sendMessage2Telegram("Turn off the flash");
        }
        else if (text=="/restart") {
          sendMessage2Telegram("Restart the board");
          ESP.restart();
        }        
        else
          sendMessage2Telegram("Command is not defined");
      }
      
      delay(1000);
    }
  }
  
  Serial.println("Connected to api.telegram.org failed.");
  if (WiFi.status() != WL_CONNECTED)
    ESP.restart();
  else
    getTelegramMessage();
}

void sendCapturedImage2Telegram() {
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
    }
    else if (fbLen%1024>0) {
      size_t remainder = fbLen%1024;
      client_tcp.write(fbBuf, remainder);
    }
  }  
  
  client_tcp.print(tail);
  
  esp_camera_fb_return(fb);
  
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
        if (c == '\n') 
        {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        if (state==true) getBody += String(c);
        startTime = millis();
     }
     if (getBody.length()>0) break;
  }
  Serial.println(getBody);
  Serial.println();
}

void sendMessage2Telegram(String text) {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = "";
  
  String request = "chat_id="+chat_id+"&text="+text;
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
  
  while ((startTime + waitTime) > millis())
  {
    Serial.print(".");
    delay(100);      
    while (client_tcp.available()) 
    {
        char c = client_tcp.read();
        if (c == '\n') 
        {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        if (state==true) getBody += String(c);
        startTime = millis();
     }
     if (getBody.length()>0) break;
  }
  Serial.println(getBody);
  Serial.println();
}
