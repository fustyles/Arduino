/*
ESP32-CAM Using keyboard in Telegram Bot with PIR sensor and SD card

Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-5-24 16:00
https://www.facebook.com/francefu

ArduinoJson Library：
https://github.com/bblanchon/ArduinoJson

Telegram Bot API
https://core.telegram.org/bots/api
*/

// Enter your WiFi ssid and password
const char* ssid     = "teacher";   //your network SSID
const char* password = "87654321";   //your network password

String token = "****:*****";   // Create your bot and get the token -> https://telegram.me/fatherbot
String chat_id = "*****";   // Get chat_id -> https://telegram.me/userinfobot

int pinPIR = 2;
/*
If "sendHelp" variable is equal to "true", it will send the command list to Telegram Bot when the board boots. 
If you don't want to get the command list when the board restarts every time, you can set the value to "false".
But you need to input the command "help" or "/help" in Telegram APP to get the command list after the board booting.
*/
boolean sendHelp = true;   

#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"
#include <Preferences.h>
Preferences preferences;
  
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
long message_id_last = 0;

boolean pirState = false;

void executeCommand(String text) {
  if (!text||text=="") return;
    
  // Custom command
  if (text=="help"||text=="/help"||text=="/start") {
    String command = "/help Command list\n/capture Get still\n/on Turn on the flash\n/off Turn off the flash\n/piron PIR ON\n/piroff PIR OFF\n/restart Restart the board";
    
    String keyboard = "{\"keyboard\":[[{\"text\":\"/on\"},{\"text\":\"/off\"},{\"text\":\"/capture\"}],[{\"text\":\"/piron\"},{\"text\":\"/piroff\"},{\"text\":\"/restart\"}]],\"one_time_keyboard\":false}";
    sendMessage2Telegram(command, keyboard);
  }        
  else if (text=="/capture") {
    sendCapturedImage2Telegram();
  }
  else if (text=="/on") {
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    sendMessage2Telegram("Turn on the flash", "");
  }
  else if (text=="/off") {
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,0);
    sendMessage2Telegram("Turn off the flash", "");
  }
  else if (text=="/restart") {
    sendMessage2Telegram("Restart the board", "");
    ESP.restart();
  }
  else if (text=="/piron") {
      pirState = true;
      sendMessage2Telegram("PIR ON", "");
  }
  else if (text=="/piroff") {
      pirState = false;
      sendMessage2Telegram("PIR OFF", "");
  }   
  else if (text=="null") {   //Server sends the response unexpectedly. Don't delete the code.
    client_tcp.stop();
    getTelegramMessage();
  }
  else
    sendMessage2Telegram("Command is not defined", "");
}

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

  //SD Card
  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD_MMC card attached");
    SD_MMC.end();
    return;
  }

  Serial.print("SD_MMC Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }    

  Serial.printf("SD_MMC Card Size: %lluMB\n", SD_MMC.cardSize() / (1024 * 1024));
  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
  Serial.println();
  
  SD_MMC.end();  
   
  /*
  //檔案流水號重設
  preferences.begin("SD", false);
  preferences.putUInt("number", 0);
  preferences.end(); 
  */   

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
  String result;
  long update_id;
  String message;
  long message_id;
  String text;  

  //client_tcp.setInsecure();   //version 1.0.6
  if (message_id_last == 0) Serial.println("Connect to " + String(myDomain));
  if (client_tcp.connect(myDomain, 443)) {
    if (message_id_last == 0) Serial.println("Connection successful");

    while (client_tcp.connected()) { 

      // PIR Sensor
      if (pirState==true) {
          pinMode(pinPIR, INPUT_PULLUP);
          int val = digitalRead(pinPIR);
          Serial.println(val);
          if (val==1) {
            preferences.begin("SD", false);
            int n = preferences.getUInt("number", 0);
            saveCapturedImage2SD(String(n+1));
            preferences.putUInt("number", (n+1));
            preferences.end();
              
            sendCapturedImage2Telegram();
          }     
      }
      
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
        }
        else {
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
 
  getTelegramMessage();   // Client's connection time out after about 3 minutes.
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
  Serial.println(getBody);
  Serial.println();
}

void sendMessage2Telegram(String text, String keyboard) {
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
  Serial.println(getBody);
  Serial.println();
}

void saveCapturedImage2SD(String filename) {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  //SD Card
  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return;
  }  
  
  String path = "/"+filename+".jpg";

  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path.c_str());

  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len);
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  file.close();
  
  file = fs.open(path.c_str());
  Serial.println("File Size = " + String(file.size()));
  
  if (file.size()==0) {
    Serial.println("Failed");
    file.close();
    
    Serial.println("Try again...");

    file = fs.open(path.c_str(), FILE_WRITE);
    if (file) {
      file.write(fb->buf, fb->len);
      file.close();
    
      file = fs.open(path.c_str());
      Serial.println("File Size = " + String(file.size()));
      if (file.size()==0) {
        Serial.println("Failed");
      }
      else {
        Serial.println("Success");
      }
    }
    else {
      Serial.println("Failed");
    }
  }
  else {
    Serial.println("Success");
  }
  file.close();
  SD_MMC.end();

  Serial.println("");  
  esp_camera_fb_return(fb);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);  
}


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}
