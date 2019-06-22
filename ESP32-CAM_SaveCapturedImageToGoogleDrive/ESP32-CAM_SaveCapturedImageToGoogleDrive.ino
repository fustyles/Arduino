/*
ESP32-CAM Save a captured image to Google Drive
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-6-22 09:30
https://www.facebook.com/francefu

Google Script
https://github.com/fustyles/webduino/blob/gs/SendCapturedImageToGoogleDriveAndLinenotify.gs
You should allow anyone to execute the google script.

https://script.google.com/home
https://script.google.com/home/executions
https://drive.google.com/drive/my-drive
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"

#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "fd_forward.h"
#include "dl_lib.h"
#include "fr_forward.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

WiFiServer server(80);

// Enter your WiFi ssid and password
const char* ssid     = "xxxxx";   //your network SSID
const char* password = "xxxxx";   //your network password

const char* apssid = "MyFirmata ESP32";
const char* appassword = "12345678";         //AP password require at least 8 characters.

const char *post_domain = "script.google.com"; // Location to send POSTed data
String post_path = "/macros/s/AKfycbxQFDRkhSSsNp-3oL_TjcCStIykkLGb5n_MEPlS9dPw1053vUo123/exec?myFoldername=TEST&myFilename=test&myToken=lHaUbj8vv1ZCvoxzwhpoarxNYR4PKYIHtVOS72qS6Lt&myFile=data:image/jpeg;base64,";

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_AP_STA);

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  WiFi.begin(ssid, password);

  delay(1000);
  Serial.println("");
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
  }  

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());

  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
  }
  else
    WiFi.softAP(apssid, appassword);
    
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());    
  server.begin(); 

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
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void loop()
{
  saveCapturedImage();
  delay(10000);
}

void saveCapturedImage() {
  camera_fb_t * fb = NULL;
  
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  // encoding
  int encodedLen = base64_enc_len(fb->len);
  char post_file[encodedLen];
  base64_encode(post_file, (char *)fb->buf, fb->len);

  WiFiClientSecure client;

  if (client.connect(post_domain, 443)) {
    Serial.println("Save a captured image to Google Drive.");
    String postDomain = String(post_domain);
    String postData = post_path+urlencode(String(post_file));
    client.println("GET " + postData + " HTTP/1.1");
    client.println("Host: " + postDomain);
    client.println("Connection: close");
    client.println();
    Serial.println("Wait for response...");
    long int StartTime=millis();
    while (!client.available()) 
    {
      Serial.print(".");
      delay(100);
      if ((StartTime+10000) < millis()) break;
    }     
    while (client.available()) {
      client.read();
    }
    Serial.println("Finished");
  }
  else
    Serial.println("Connection failed.");
  client.stop();
}

//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/blob/master/helloWorld_urlencoded/urlencode.ino
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
