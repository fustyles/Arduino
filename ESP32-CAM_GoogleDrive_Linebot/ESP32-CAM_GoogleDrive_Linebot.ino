/*
ESP32-CAM Upload a captured photo to Google Drive and send the uploaded image link to Line Notify or Line Bot.
Author : ChungYi Fu (Kaohsiung, Taiwan)  2024-11-10 10:30
https://www.facebook.com/francefu

Google Apps Script
https://github.com/fustyles/webduino/blob/gs/SendCapturedImageToGoogleDriveAndLinenotify_doPost.gs
You must allow anyone, including anonymous users, to execute the Google Script.

How to add a new script
https://www.youtube.com/watch?v=f46VBqWwUuI

https://script.google.com/home
https://script.google.com/home/executions
https://drive.google.com/drive/my-drive
*/

String myAppsScriptID = "";    // https://github.com/fustyles/webduino/blob/gs/SendCapturedImageToGoogleDriveAndLinenotify_doPost.gs
String myFoldername = "ESP32-CAM";   // If you want to see the thumbnail of a photo from Google Drive in a conversation message on the Line app on your mobile phone, you need to change the permissions of the Google Drive folder, setting it so that anyone with the link can view it.
String myFilename = "HOME";

// If you want to send a uploaded image link to Line Notify.
String myLineNotifyToken = "";

// If you want to send a uploaded image link to Line Bot.
String myLineBotToken = "";    // Channel access token
String myLineBotUserID = "";   // userId or groupId

char wifi_ssid[] = "teacher";
char wifi_pass[] = "12345678";

#include <WiFi.h>
#include <WiFiClientSecure.h>
WiFiClientSecure client;
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
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

#include "Base64.h"

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

void SendStillToGoogleDrive(String scriptid, String foldername, String filename) {
  String data = "&myFoldername="+urlencode(foldername)+"&myFilename="+urlencode(filename)+"&myLineType=&myToken=&myUserID=&myFile=";
  SendStillToGoogleDrive(scriptid, data);
}

void SendStillToGoogleDriveLineNotify(String scriptid, String foldername, String filename, String token) {
  String data = "&myFoldername="+urlencode(foldername)+"&myFilename="+urlencode(filename)+"&myLineType=notify&myToken="+urlencode(token)+"&myUserID=&myFile=";
  SendStillToGoogleDrive(scriptid, data);
}

void SendStillToGoogleDriveLineBot(String scriptid, String foldername, String filename, String token, String userID) {
  String data = "&myFoldername="+urlencode(foldername)+"&myFilename="+urlencode(filename)+"&myLineType=bot&myToken="+urlencode(token)+"&myUserID="+urlencode(userID)+"&myFile=";
  SendStillToGoogleDrive(scriptid, data);
}

String SendStillToGoogleDrive(String myScriptID, String myData) {
  const char* myDomain = "script.google.com";
  String myScript = "/macros/s/"+myScriptID+"/exec";  
  String getAll="", getBody = "";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }

  client.setInsecure();
  Serial.println("Connect to " + String(myDomain));
  if (client.connect(myDomain, 443)) {
    Serial.println("Connection successful");

    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }

    client.println("POST " + myScript + " HTTP/1.1");
    client.println("Host: " + String(myDomain));
    client.println("Content-Length: " + String(myData.length()+imageFile.length()));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.println();

    client.print(myData);
    int Index;
    for (Index = 0; Index < imageFile.length(); Index = Index+1024) {
      client.print(imageFile.substring(Index, Index+1024));
    }
    esp_camera_fb_return(fb);

    int waitTime = 10000;
    long startTime = millis();
    boolean state = false;

    while ((startTime + waitTime) > millis())
    {
      Serial.print(".");
      delay(100);
      while (client.available())
      {
          char c = client.read();
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
    client.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to " + String(myDomain) + " failed.";
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }

  return getBody;
}

String urlencode(String str) {
  const char *msg = str.c_str();
  const char *hex = "0123456789ABCDEF";
  String encodedMsg = "";
  while (*msg != '\0') {
    if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9') || *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~') {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[(unsigned char)*msg >> 4];
      encodedMsg += hex[*msg & 0xf];
    }
    msg++;
  }
  return encodedMsg;
}

void setup()
{
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.setDebugOutput(true);
  Serial.println();
  
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
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }
  s->set_framesize(s, FRAMESIZE_QVGA);
  
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  initWiFi();

  // Google drive
  SendStillToGoogleDrive(myAppsScriptID, myFoldername, myFilename);
  
  // Google drive, Line Notify
  SendStillToGoogleDriveLineNotify(myAppsScriptID, myFoldername, myFilename, myLineNotifyToken);

  // Google drive, Line Bot  
  SendStillToGoogleDriveLineBot(myAppsScriptID, myFoldername, myFilename, myLineBotToken, myLineBotUserID);
}

void loop()
{

}
