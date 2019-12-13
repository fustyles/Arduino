/*
ESP32-CAM MyBlock
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-12-13 01:00
https://www.facebook.com/francefu

Command Format :  
http://APIP/control?var=cmd&val=cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/control?var=cmd&val=cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

Default APIP： 192.168.4.1
http://192.168.4.1/control?var=cmd&val=ip
http://192.168.4.1/control?var=cmd&val=mac
http://192.168.4.1/control?var=cmd&val=restart
http://192.168.4.1/control?var=cmd&val=inputpullup=pin
http://192.168.4.1/control?var=cmd&val=pinmode=pin;value
http://192.168.4.1/control?var=cmd&val=digitalwrite=pin;value
http://192.168.4.1/control?var=cmd&val=analogwrite=pin;value
http://192.168.4.1/control?var=cmd&val=digitalread=pin
http://192.168.4.1/control?var=cmd&val=analogread=pin
http://192.168.4.1/control?var=cmd&val=touchread=pin
http://192.168.4.1/control?var=cmd&val=tcp=domain;port;request;wait
--> wait = 0 or 1  (waiting for response)
--> request = /xxxx/xxxx
http://192.168.4.1/control?var=cmd&val=ifttt=event;key;value1;value2;value3
http://192.168.4.1/control?var=cmd&val=thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8
http://192.168.4.1/control?var=cmd&val=thingspeakread=request
--> request = /channels/xxxxx/fields/1.jsoncontrol?var=cmd&val=results=1

http://192.168.4.1/control?var=cmd&val=flash=value        //vale= 0~255
http://192.168.4.1/control?var=cmd&val=servo=value        //vale= 1700~8000
http://192.168.4.1/control?var=cmd&val=servo1=value       //vale= 1700~8000
http://192.168.4.1/control?var=cmd&val=servo2=value       //vale= 1700~8000
http://192.168.4.1/control?var=cmd&val=speedL=value       //vale= 0~255
http://192.168.4.1/control?var=cmd&val=speedR=value       //vale= 0~255
http://192.168.4.1/control?var=cmd&val=decelerate=value   //vale= 0~100  (%)
http://192.168.4.1/control?var=cmd&val=car=state          //state= 1(Front),2(Left),3(Stop),4(Right),5(Back),6(FrontLeft),7(FrontRight),8(LeftAfter),9(RightAfter)
http://192.168.4.1/control?var=cmd&val=getstill           //base64
http://192.168.4.1/control?var=cmd&val=getstill=img       //<img id='gameimage_getstill' src='base64'>
http://192.168.4.1/control?var=cmd&val=framesize=size     //size= UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

STAIP：
Query：http://192.168.4.1/control?var=cmd&val=ip
Link：http://192.168.4.1/control?var=cmd&val=resetwifi=ssid;password

If you don't need to get response from ESP8266 and want to execute commands quickly, 
you can append a parameter value "stop" at the end of command.
For example:
http://192.168.4.1/control?var=cmd&val=digitalwrite=gpio;value;stop
http://192.168.4.1/control?var=cmd&val=restart=stop
*/

const char* ssid = "xxxxx";
const char* password = "xxxxx";

#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

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

void startCameraServer();

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
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
  s->set_framesize(s, FRAMESIZE_QVGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  
  //Flash
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);
  
  //Wheel
  ledcAttachPin(12, 5);
  ledcSetup(5, 2000, 8);      
  ledcAttachPin(13, 6);
  ledcSetup(6, 2000, 8); 
  ledcWrite(6, 0);  
  ledcAttachPin(15, 7);
  ledcSetup(7, 2000, 8);      
  ledcAttachPin(14, 8);
  ledcSetup(8, 2000, 8); 

  Serial.println("ssid: " + (String)ssid);
  Serial.println("password: " + (String)password);
  
  WiFi.begin(ssid, password);

  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+10000) < millis()) break;
  } 

  startCameraServer();

  char* apssid = "ESP32-CAM";
  char* appassword = "12345678";         //AP password require at least 8 characters.
  Serial.println("");
  Serial.println("WiFi connected");    
  Serial.print("Camera Ready! Use 'http://");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);    
    
    for (int i=0;i<5;i++) {
      ledcWrite(4,10);
      delay(200);
      ledcWrite(4,0);
      delay(200);    
    }        
  }
  else {
    Serial.print(WiFi.softAPIP());
    Serial.println("' to connect");
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);    
    
    for (int i=0;i<2;i++) {
      ledcWrite(4,10);
      delay(1000);
      ledcWrite(4,0);
      delay(1000);    
    }  
  }     
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}
