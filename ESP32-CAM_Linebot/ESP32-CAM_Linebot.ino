/*
ESP32-CAM (Line Bot)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-4-4 23:30
https://www.facebook.com/francefu


Google Apps Script
You must allow anyone and anonymous to execute the google script.
https://script.google.com/home
https://script.google.com/home/executions

ESP32-CAM Line Bot
https://github.com/fustyles/webduino/blob/gs/esp32-cam_linebot_dht.gs

ESP32-CAM Line Bot with MQTT
https://github.com/fustyles/webduino/blob/gs/esp32-cam_linebot_dht_mqtt.txt

Line Bot with MQTT
https://github.com/fustyles/webduino/blob/gs/esp32_linebot_mqtt.txt


Google spreadsheet
https://docs.google.com/spreadsheets/u/0/


Line bot Message API (Using Google Apps Script as a Webhook)
https://developers.line.biz/en/services/messaging-api/


Line Notify
https://notify-bot.line.me/







function doPost(e) {

  var BOT_ACCESS_TOKEN = 'Nwyfihc0pKey868MefCc9Er028u7E33OPJuRwdLEi/mmyjBSh0jFOJKvS3AMaFvKUKAp1k7JKdj2tpd8nr8/aJVF45aQajMY0anwVABxPkvJk3oPUeGlmdDWBiQt6qKBLzGhYimXU377SkcT03hhBwdB04t89/1O/w1cDnyilFU=123';
  var NOTIFY_ACCESS_TOKEN = 'RXDcVAhLhvVJjX0fxarGLcrbjqyWWIJPKPu0QdpomFE123';
  var SPREADSHEET_ID = '1VVONSSJSNY8Xj2-hO3swD7EEfky6vA99jp5CzZkxDKM123';

  var SpreadSheet = SpreadsheetApp.openById(SPREADSHEET_ID);
  var Sheet = SpreadSheet.getSheets()[0];
  var lastRow = Sheet.getLastRow();

  if (e.parameter.myFile) {
    var humidity = e.parameter.humidity;
    var temperature = e.parameter.temperature;    
    var myFile = e.parameter.myFile;
    var filename = e.parameter.myFilename;

    Sheet.getRange(lastRow+1,1).setValue("'"+Utilities.formatDate(new Date(), "GMT+8", "yyyy/MM/dd HH:mm:ss"));
    Sheet.getRange(lastRow+1,2).setValue(humidity);
    Sheet.getRange(lastRow+1,3).setValue(temperature); 
    Sheet.getRange(lastRow+1,4).setValue(filename);           
    Sheet.getRange(lastRow+1,5).setValue(myFile);
    
  } else {
  
    var msg = JSON.parse(e.postData.contents);
    const userMessage = msg.events[0].message.text.trim();
    const user_id = msg.events[0].source.userId;
    const event_type = msg.events[0].source.type;
    const replyToken = msg.events[0].replyToken;
      
    var reply_message;
    var Time = Sheet.getRange(lastRow,1).getValue();

    if (userMessage=="image") {
      var myFile = Sheet.getRange(lastRow,5).getValue();
      var imageData = myFile.substring(myFile.indexOf(",")+1);
      imageData = Utilities.base64Decode(imageData);
      var contentType = myFile.substring(myFile.indexOf(":")+1, myFile.indexOf(";"));
      var filename = Sheet.getRange(lastRow,4).getValue();
      var message =  filename;

      var blob = Utilities.newBlob(imageData, contentType, filename);
      var boundary = "------------------------------";
      var imageData = Utilities.newBlob(
          "--" + boundary + "\r\n"
          + "Content-Disposition: form-data; name=\"message\"; \r\n\r\n" + message + "\r\n"
          + "--" + boundary + "\r\n"
          + "Content-Disposition: form-data; name=\"imageFile\"; filename=\"" + filename + "\"\r\n"
          + "Content-Type: " + 'image/jpeg' +"\r\n\r\n"
          ).getBytes();
      imageData = imageData.concat(blob.getBytes());
      imageData = imageData.concat(Utilities.newBlob("\r\n--" + boundary + "--\r\n").getBytes());
      
      var options = {
        "method" : "post",
        "contentType" : "multipart/form-data; boundary=" + boundary,
        "payload" : imageData,    
        "headers" : {"Authorization" : "Bearer " + NOTIFY_ACCESS_TOKEN}
      };
      UrlFetchApp.fetch("https://notify-api.line.me/api/notify", options);
    } 
    else if (userMessage=="humidity") {
      var humidity = Time+"\nhumidity = "+Sheet.getRange(lastRow,2).getValue()+" %";
      reply_message = [{
        "type":"text",
        "text": humidity
      }]
      sendMessageToLineBot(BOT_ACCESS_TOKEN,replyToken,reply_message);
    } 
    else if (userMessage=="temperature") {
      var temperature = Time+"\ntemperature = "+Sheet.getRange(lastRow,3).getValue()+" °C";
      reply_message = [{
        "type":"text",
        "text": temperature
      }]      
      sendMessageToLineBot(BOT_ACCESS_TOKEN,replyToken,reply_message);
    } 
    else if (userMessage=="help") {
      reply_message = [{
            "type": "text",
            "text": "Command list",
            "quickReply": {
                "items": [
                    {
                        "type": "action",
                        "action": {
                            "type": "message",
                            "label": "humidity",
                            "text": "humidity"
                        }
                    },
                    {
                        "type": "action",
                        "action": {
                            "type": "message",
                            "label": "temperature",
                            "text": "temperature"
                        }
                    },
                    {
                        "type": "action",
                        "action": {
                            "type": "message",
                            "label": "image",
                            "text": "image"
                        }
                    }
                ]
            }
      }] 
      sendMessageToLineBot(BOT_ACCESS_TOKEN,replyToken,reply_message);           
    }
  } 
  return  ContentService.createTextOutput("Return = OK");
}

function sendMessageToLineBot(accessToken, replyToken, reply_message) {
  var url = 'https://api.line.me/v2/bot/message/reply';
  UrlFetchApp.fetch(url, {
    'headers': {
      'Content-Type': 'application/json; charset=UTF-8',
      'Authorization': 'Bearer ' + accessToken,
    },
    'method': 'post',
    'payload': JSON.stringify({
      'replyToken': replyToken,
      'messages': reply_message
    }),
  });
} 


*/

// Enter your WiFi ssid and password
const char* ssid     = "teacher";   //your network SSID
const char* password = "87654321";   //your network password

String myScript = "/macros/s/AKfycbwjXH4ahYN8xiVGcU5k4Dbu-Fjx95Q9O4gCvLMU19OghCfD6dd2DEDKGPpqT8pxYjh-123/exec";    //Create your Google Apps Script and replace the "myScript" path.
String myFilename = "&myFilename=ESP32-CAM.jpg";
String myImage = "&myFile=";

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"

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

#include <dht.h>   
#define dht_dpin 2   //DHT11 IO2
dht DHT;
String dhtData = ""; 

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
    Serial.println("Reset");
    
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    delay(200);
    ledcWrite(3,0);
    delay(200);    
    ledcDetachPin(3);
        
    delay(1000);
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
  s->set_framesize(s, FRAMESIZE_VGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
}

void loop()
{
  DHT.read11(dht_dpin);
  dhtData = "&humidity=" +String(DHT.humidity)+"&temperature="+String(DHT.temperature);

  SendCapturedImageToLineBot();
  delay(60000);  
}

String SendCapturedImageToLineBot() {
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
  client_tcp.setInsecure();   //run version 1.0.5 or above
  
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    String Data = dhtData+myFilename+myImage;
    
    client_tcp.println("POST " + myScript + " HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
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
    getBody="Connected to " + String(myDomain) + " failed.";
    Serial.println("Connected to " + String(myDomain) + " failed.");
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
