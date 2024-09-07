/*
ESP32-CAM Upload the image file to Google spreadsheet and Google drive
Author : ChungYi Fu (Kaohsiung, Taiwan)  2024-9-7 14:30
https://www.facebook.com/francefu

Google spreadsheet
https://docs.google.com/spreadsheets/u/0/

Google Apps Script
https://script.google.com/home
https://script.google.com/home/executions
*/

String spreadsheetUrl = "xxxxx";
String spreadsheetName = "xxxxx";

//Apps script:  https://github.com/fustyles/webduino/blob/gs/SendCapturedImageToSpreadsheet_base64_doPost.gs
String myAppsScriptURL = "xxxxx";

String myFolderName = "&foldername=ESP32-CAM_IMAGES";      // Google drive folder name. Set anyone with the link can view the files in the folder.
String myDatetime = "&datetime=gmt_datetime";              // gmt_datetime (Column A,B), gmt_date (Column A), gmt_time (Column A)
String myPosition = "&position=insertfirst";               // insertfirst, insertsecond, insertlast  (insert a new row)
String myCellwidth = "&cellwidth=240";                     // If the format setting is "jpg", the image cell width is 240px.
String myCellheight = "&cellheight=160";                   // If the format setting is "jpg", the image cell height is 160px.
String myColumn = "&column=3";                             // The image data inserts into the column 3 (Column C).
String myRow = "&row=1";                                   // If the position setting is "custom" or not specified, updates the data in the row 1.
String myFormat = "&format=jpg";                           // base64, link, jpg
String myLinenotifyToken = "&linetoken=xxxxx";             // Line Notify token

char wifi_ssid[] = "teacher";
char wifi_pass[] = "12345678";

#include <WiFi.h>
#include <WiFiClientSecure.h>
WiFiClientSecure client;

#include "Base64.h"

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

void setup()
{
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.setDebugOutput(true);

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
  s->set_framesize(s, FRAMESIZE_CIF);
  
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  initWiFi();  

  delay(3000);
  
  myFormat = "&format=base64";
  SendStillToSpreadsheet();
  
  myFormat = "&format=link";
  SendStillToSpreadsheet();
  
  myFormat = "&format=jpg";
  SendStillToSpreadsheet();  
}

void loop()
{

}


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

String SendStillToSpreadsheet() {
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
  client.setInsecure();
  if (client.connect(myDomain, 443)) {
    Serial.println("Connection successful");

    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    
    String mySpreadsheetUrl = "&spreadsheeturl=" + spreadsheetUrl;
    String mySpreadsheetName = "&spreadsheetname=" + urlencode(spreadsheetName);
    String myFile = "&file=";
    String Data = mySpreadsheetUrl+mySpreadsheetName+myFolderName+myLinenotifyToken+myDatetime+myPosition+myCellwidth+myCellheight+myColumn+myRow+myFormat+myFile;
	
    client.println("POST " + myAppsScriptURL + " HTTP/1.1");
    client.println("Host: " + String(myDomain));
    client.println("Content-Length: " + String(Data.length()+imageFile.length()));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.println();

    client.print(Data);
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
