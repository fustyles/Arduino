/*
ESP32-CAM Stream (slove the problem about "Header fields are too long for server to interpret")
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-11-9 20:30
https://www.facebook.com/francefu
*/


char _lwifi_ssid[] = "teacher";
char _lwifi_pass[] = "87654321";

const char* apssid = "esp32-cam";
const char* appassword = "12345678";

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

WiFiServer server(80);

String Feedback="",Command="",cmd="",p1="",p2="",p3="",p4="",p5="",p6="",p7="",p8="",p9="";
byte receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;
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

void executeCommand() {
  //Serial.println("");
  //Serial.println("Command: "+Command);
  //Serial.println("cmd= "+cmd+" ,p1= "+p1+" ,p2= "+p2+" ,p3= "+p3+" ,p4= "+p4+" ,p5= "+p5+" ,p6= "+p6+" ,p7= "+p7+" ,p8= "+p8+" ,p9= "+p9);
  //Serial.println("");
  if (cmd=="ip") {
    Feedback="AP IP: "+WiFi.softAPIP().toString();
    Feedback+="<br>";
    Feedback+="STA IP: "+WiFi.localIP().toString();
  } else if (cmd=="mac") {
    Feedback="STA MAC: "+WiFi.macAddress();
  } else if (cmd=="restart") {
    ESP.restart();
  } else if (cmd=="digitalwrite") {
    ledcDetachPin(p1.toInt());
    pinMode(p1.toInt(), OUTPUT);
    digitalWrite(p1.toInt(), p2.toInt());
  } else if (cmd=="digitalread") {
    Feedback=String(digitalRead(p1.toInt()));
  } else if (cmd=="analogwrite") {
    if (p1=="4") {
      ledcAttachPin(4, 4);
      ledcSetup(4, 5000, 8);
      ledcWrite(4,p2.toInt());
    } else {
      ledcAttachPin(p1.toInt(), 9);
      ledcSetup(9, 5000, 8);
      ledcWrite(9,p2.toInt());
    }
  } else if (cmd=="analogread") {
    Feedback=String(analogRead(p1.toInt()));
  } else if (cmd=="touchread") {
    Feedback=String(touchRead(p1.toInt()));
  } else if (cmd=="restart") {
    ESP.restart();
  } else if (cmd=="flash") {
    ledcAttachPin(4, 4);
    ledcSetup(4, 5000, 8);
    int val = p1.toInt();
    ledcWrite(4,val);
  } else if(cmd=="servo") {
    ledcAttachPin(p1.toInt(), p3.toInt());
    ledcSetup(p3.toInt(), 50, 16);
    int val = 7864-p2.toInt()*34.59;
    if (val > 7864)
       val = 7864;
    else if (val < 1638)
      val = 1638;
    ledcWrite(p3.toInt(), val);
  } else if (cmd=="relay") {
    pinMode(p1.toInt(), OUTPUT);
    digitalWrite(p1.toInt(), p2.toInt());
  } else if (cmd=="buzzer") {
    pinMode(p1.toInt(),OUTPUT);
    if (p4=="") p4="9";
    ledcSetup(p4.toInt(), 2000, 8);
    ledcAttachPin(p1.toInt(), p4.toInt());
    ledcWriteTone(p4.toInt(), p2.toInt());
    delay(p3.toInt());
    ledcWriteTone(p4.toInt(), 0);
  } else if (cmd=="resetwifi") {
    for (int i=0;i<2;i++) {
      WiFi.begin(p1.c_str(), p2.c_str());
      Serial.print("Connecting to ");
      Serial.println(p1);
      long int StartTime=millis();
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          if ((StartTime+5000) < millis()) break;
      }
      Serial.println("");
      Serial.println("STAIP: "+WiFi.localIP().toString());
      Feedback="STAIP: "+WiFi.localIP().toString();
      if (WiFi.status() == WL_CONNECTED) {
        WiFi.softAP((WiFi.localIP().toString()+"_"+p1).c_str(), p2.c_str());
        break;
      }
    }
  } else if (cmd=="print") {
    Serial.print(p1);
  } else if (cmd=="println") {
    Serial.println(p1);
  } else if (cmd=="delay") {
    delay(p1.toInt());
  } else if (cmd=="framesize") {
    int val = p1.toInt();
    sensor_t * s = esp_camera_sensor_get();
    s->set_framesize(s, (framesize_t)val);
  } else if (cmd=="quality") {
    sensor_t * s = esp_camera_sensor_get();
    s->set_quality(s, p1.toInt());
  } else if (cmd=="contrast") {
    sensor_t * s = esp_camera_sensor_get();
    s->set_contrast(s, p1.toInt());
  } else if (cmd=="brightness") {
    sensor_t * s = esp_camera_sensor_get();
    s->set_brightness(s, p1.toInt());
  } else if (cmd=="saturation") {
    sensor_t * s = esp_camera_sensor_get();
    s->set_saturation(s, p1.toInt());
  } else if (cmd=="special_effect") {
    sensor_t * s = esp_camera_sensor_get();
    s->set_special_effect(s, p1.toInt());
  } else if (cmd=="hmirror") {
    sensor_t * s = esp_camera_sensor_get();
    s->set_hmirror(s, p1.toInt());
  } else if (cmd=="vflip") {
    sensor_t * s = esp_camera_sensor_get();
    s->set_vflip(s, p1.toInt());
  } else {
    //http://192.168.xxx.xxx/?cmd=p1;p2;p3;p4;p5;p6;p7;p8;p9
    if (cmd == String("yourcmd")) {
    } else {
      Feedback = String("Command is not defined.");
    }
  }
}

  void initWiFi() {
    WiFi.mode(WIFI_AP_STA);

    for (int i=0;i<2;i++) {
      if (String(_lwifi_ssid)=="") break;
      WiFi.begin(_lwifi_ssid, _lwifi_pass);

      delay(1000);
      Serial.println("");
      Serial.print("Connecting to ");
      Serial.println(_lwifi_ssid);

      long int StartTime=millis();
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          if ((StartTime+5000) < millis()) break;
      }

      if (WiFi.status() == WL_CONNECTED) {
        WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
        Serial.println("");
        Serial.println("STAIP address: ");
        Serial.println(WiFi.localIP());
        Serial.println("");

  		 ledcAttachPin(4, 4);
  		 ledcSetup(4, 5000, 8);
        for (int i=0;i<5;i++) {
          ledcWrite(4,10);
          delay(200);
          ledcWrite(4,0);
          delay(200);
        }
        break;
      }
    }

    if (WiFi.status() != WL_CONNECTED) {
      WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);

    }

    Serial.println("");
    Serial.println("APIP address: ");
    Serial.println(WiFi.softAPIP());

    server.begin();
  }

  void getRequest() {
    Command="";cmd="";p1="";p2="";p3="";p4="";p5="";p6="";p7="";p8="";p9="";
    receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;

    WiFiClient client = server.available();

    if (client) {
      String currentLine = "";

      while (client.connected()) {
        if (client.available()) {
          char c = client.read();

          getCommand(c);

          if (c == '\n') {
            if (currentLine.length() == 0) {
   	        if (cmd=="getstill") {
     	            camera_fb_t * fb = NULL;

                  String head = "--Taiwan\r\nContent-Type: image/jpeg\r\n\r\n";
                  //String tail = "\r\n--Taiwan--\r\n";
                  client.println("HTTP/1.1 200 OK");
                  client.println("Access-Control-Allow-Origin: *");
                  client.println("Content-Type: multipart/x-mixed-replace; boundary=Taiwan");
                  client.println();  

                  while(client.connected()) {
                    
                    fb = esp_camera_fb_get();
                    if(!fb) {
                      Serial.println("Camera capture failed");
                      delay(1000);
                      ESP.restart();
                    }
                    client.print(head);
                    
                    uint8_t *fbBuf = fb->buf;
                    size_t fbLen = fb->len;
                    for (size_t n=0;n<fbLen;n=n+1024) {
                      if (n+1024<fbLen) {
                        client.write(fbBuf, 1024);
                        fbBuf += 1024;
                      }
                      else if (fbLen%1024>0) {
                        size_t remainder = fbLen%1024;
                        client.write(fbBuf, remainder);
                      }
                    }
                    esp_camera_fb_return(fb);
                    
                    client.print("\r\n");
                  }
             
   	        } else {
   	        	client.println("HTTP/1.1 200 OK");
   	        	client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
   	        	client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
   	        	client.println("Content-Type: text/html; charset=utf-8");
   	        	client.println("Access-Control-Allow-Origin: *");
   	        	client.println("X-Content-Type-Options: nosniff");
   	        	client.println();
   	        	if (Feedback=="")
   	        		Feedback=("<!DOCTYPE html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><script src='https://ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js'></script><script src='https://fustyles.github.io/webduino/GameElements_20190131/gameelements.js'></script></head><body><script>const delay=(seconds)=>{return new Promise((resolve)=>{setTimeout(resolve,seconds*1000);});};const main=async()=>{  image_create_stream('',('?'+'getstill'+'='+(new Date().getTime())),0,0,999,true);};main();</script></body></html>");
   	        	for (int index = 0; index < Feedback.length(); index = index+1024) {
   	        	  client.print(Feedback.substring(index, index+1024));
   	        	}
   	        }
   	        Feedback="";
   	        break;
            } else {
              currentLine = "";
            }
          }
          else if (c != '\r') {
            currentLine += c;
          }

          if ((currentLine.indexOf("/?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1)) {
            if (Command.indexOf("stop")!=-1) {
              client.println();
              client.println();
             client.stop();
            }
            currentLine="";
            Feedback="";
            executeCommand();
          }
        }
      }
      delay(1);
      client.stop();
    }
  }

  void getCommand(char c) {
    if (c=='?') receiveState=1;
    if ((c==' ')||(c=='\r')||(c=='\n')) receiveState=0;

    if (receiveState==1) {
      Command=Command+String(c);

      if (c=='=') cmdState=0;
      if (c==';') pState++;

      if ((cmdState==1)&&((c!='?')||(questionState==1))) cmd=cmd+String(c);
      if ((cmdState==0)&&(pState==1)&&((c!='=')||(equalState==1))) p1=p1+String(c);
      if ((cmdState==0)&&(pState==2)&&(c!=';')) p2=p2+String(c);
      if ((cmdState==0)&&(pState==3)&&(c!=';')) p3=p3+String(c);
      if ((cmdState==0)&&(pState==4)&&(c!=';')) p4=p4+String(c);
      if ((cmdState==0)&&(pState==5)&&(c!=';')) p5=p5+String(c);
      if ((cmdState==0)&&(pState==6)&&(c!=';')) p6=p6+String(c);
      if ((cmdState==0)&&(pState==7)&&(c!=';')) p7=p7+String(c);
      if ((cmdState==0)&&(pState==8)&&(c!=';')) p8=p8+String(c);
      if ((cmdState==0)&&(pState>=9)&&((c!=';')||(semicolonState==1))) p9=p9+String(c);

      if (c=='?') questionState=1;
      if (c=='=') equalState=1;
      if ((pState>=9)&&(c==';')) semicolonState=1;
    }
  }

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  delay(10);
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
  Serial.println();
  initWiFi();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
}

void loop()
{
  getRequest();

}
