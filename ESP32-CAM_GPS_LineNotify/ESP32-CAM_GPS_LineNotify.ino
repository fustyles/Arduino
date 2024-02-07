/*
ESP32-CAM (with flash) Send Google map url and still to Line notify
Author : ChungYi Fu (Kaohsiung, Taiwan)  2024-2-7 10:40
https://www.facebook.com/francefu

Arduino IDE: 
Arduino core for the ESP32 V1.0.6
ESP32 Wrover Module

GPS Module TX -> ESP32-CAM RX (IO3)   燒錄前要移除以免無法燒錄。
GPS Module RX -> Don't connect to ESP32-CAM TX

Library
https://www.arduino.cc/reference/en/libraries/tinygps/

Google Maps Embed (div html)
<iframe width="300" height="300" style="border:0" loading="lazy" allowfullscreen referrerpolicy="no-referrer-when-downgrade" src="https://www.google.com/maps/embed/v1/place?key=填入已啟用Embed免費金鑰&q=填入經度,填入緯度"> </iframe>
*/

char wifi_ssid[] = "teacher";
char wifi_pass[] = "87654321";
String lineToken = "";

#include <WiFi.h>
#include <WiFiClientSecure.h>
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

#include <TinyGPS.h>
TinyGPS gps;

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

String sendStillToLineNotify(String token, String message) {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "Camera capture failed";
  }

  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  if (client_tcp.connect("notify-api.line.me", 443)) {
    Serial.println("Connection successful");

    if (message=="") message = "ESP32-CAM";
    String head = "--Taiwan\r\nContent-Disposition: form-data; name=\"message\"; \r\n\r\n" + message + "\r\n--Taiwan\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--Taiwan--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;

    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close");
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Length: " + String(totalLen));
    client_tcp.println("Content-Type: multipart/form-data; boundary=Taiwan");
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

    String getResponse="",Feedback="";
    int waitTime = 10000;
    long startTime = millis();
    boolean state = false;

    while ((startTime + waitTime) > millis()) {
      //Serial.print(".");
      delay(100);
      while (client_tcp.available())  {
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
       if (Feedback.length()>0) break;
    }
    //Serial.println();
    client_tcp.stop();
    return Feedback;
  }
  else {
    return "Connected to notify-api.line.me failed.";
  }
};

String TinyGPS_coordinateToString(float coordinate) {
  int digits = 6;
  String val = String(coordinate*pow(10, digits));
  int point = val.indexOf(".");
  val = val.substring(0, point);
  int len = val.length();
  if (len>=digits) {
    return val.substring(0, len-digits)+"."+val.substring(len-digits, len);
  } else {
    return "";
  }
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  
  Serial.begin(9600);
  
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
  
  pinMode(4, OUTPUT);  //若改良版無閃光燈拿掉此行
  digitalWrite(4, LOW);  //若改良版無閃光燈拿掉此行

  initWiFi();
  
  sendStillToLineNotify(lineToken, "https://www.google.com/maps/search/?api=1&map_action=map&zoom=16&query=22.62521720623603,120.37221859329584");
}

void loop()
{
  bool gpsNewData = false;
  if (Serial.available()) {
    while (Serial.available()) {
      char uartData = Serial.read();
      if (gps.encode((uartData))) 
        gpsNewData = true;
    }
  }

  if (gpsNewData) {
    float TinyGPS_flat, TinyGPS_flon;
    unsigned long TinyGPS_age;
    gps.f_get_position(&TinyGPS_flat, &TinyGPS_flon, &TinyGPS_age);

    /*
    unsigned long TinyGPS_chars;
    unsigned short TinyGPS_sentences, TinyGPS_failed;    
    gps.stats(&TinyGPS_chars, &TinyGPS_sentences, &TinyGPS_failed);    
    */
    
    String flat = (TinyGPS_flat == TinyGPS::GPS_INVALID_F_ANGLE ? "0" : TinyGPS_coordinateToString(TinyGPS_flat));
    String flon = (TinyGPS_flon == TinyGPS::GPS_INVALID_F_ANGLE ? "0" : TinyGPS_coordinateToString(TinyGPS_flon));
  
    String mapURL = "https://www.google.com/maps/search/?api=1&map_action=map&zoom=16&query="+flat+","+flon;
    sendStillToLineNotify(lineToken, mapURL);
  
    delay(72000);  //Line Notify上限 50張/時，平均間隔72秒
  }
}
