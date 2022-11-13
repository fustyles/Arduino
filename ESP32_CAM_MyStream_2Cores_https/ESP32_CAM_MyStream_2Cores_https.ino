/*
ESP32-CAM My Stream (For solving the problem about "Header fields are too long for server to interpret")
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-11-11 11:11

https://www.facebook.com/francefu

library:
https://github.com/fhessel/esp32_https_server

stream (https)
https://yourIP

stop stream
https://yourIP/stop
http://yourIP/?stop

custom command (http)
http://yourIP/?cmd=p1;p2;p3;p4;p5;p6;p7;p8;p9 
*/


const char* ssid = "teacher";
const char* password = "12345678";

const char* apssid = "esp32-cam";
const char* appassword = "12345678";

// Include certificate data (see note above)
#include "cert.h"
#include "private_key.h"

#include <WiFi.h>
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Includes for the server
// Note: We include HTTPServer and HTTPSServer
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>

// The HTTPS Server comes in a separate namespace. For easier use, include it here.
using namespace httpsserver;

// Create an SSL certificate object from the files included above
SSLCert cert = SSLCert(
  example_crt_DER, example_crt_DER_len,
  example_key_DER, example_key_DER_len
);

// First, we create the HTTPSServer with the certificate created above
HTTPSServer secureServer = HTTPSServer(&cert);

// Declare some handler functions for the various URLs on the server
void handleRoot(HTTPRequest * req, HTTPResponse * res);
void handle404(HTTPRequest * req, HTTPResponse * res);


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

String Feedback="",Command="",cmd="",p1="",p2="",p3="",p4="",p5="",p6="",p7="",p8="",p9="";
byte receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;
boolean connectionState = false;

WiFiServer server80(80);

void cameraInitial() {
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
}

void initWiFi() {
  WiFi.mode(WIFI_AP_STA);

  for (int i=0;i<2;i++) {
    if (String(ssid)=="") break;
    WiFi.begin(ssid, password);

    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);

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
    for (int i=0;i<3;i++) {
      ledcWrite(4,10);
      delay(500);
      ledcWrite(4,0);
      delay(500);
    }    
  }

  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());

  // For every resource available on the server, we need to create a ResourceNode
  // The ResourceNode links URL and HTTP method to a handler function
  ResourceNode * nodeRoot = new ResourceNode("/", "GET", &handleRoot);
  ResourceNode * nodeStop = new ResourceNode("/stop", "GET", &handleStop);
  ResourceNode * node404  = new ResourceNode("", "GET", &handle404);

  // Add the root node to the servers. We can use the same ResourceNode on multiple
  // servers (you could also run multiple HTTPS servers)
  secureServer.registerNode(nodeRoot);

  // We do the same for the default Node
  secureServer.setDefaultNode(node404);

  Serial.println("Starting HTTPS server...");
  secureServer.start();
  if (secureServer.isRunning()) {
    Serial.println("Servers ready.");
  }
}

void getRequest80() {
  Command="";cmd="";p1="";p2="";p3="";p4="";p5="";p6="";p7="";p8="";p9="";
  receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;

  WiFiClient client = server80.available();

  if (client) {
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        getCommand(c);

        if (c == '\n') {
          if (currentLine.length() == 0) {

            //Serial.println("cmd= "+cmd+" ,p1= "+p1+" ,p2= "+p2+" ,p3= "+p3+" ,p4= "+p4+" ,p5= "+p5+" ,p6= "+p6+" ,p7= "+p7+" ,p8= "+p8+" ,p9= "+p9);
            //Serial.println("");

            if (cmd=="stop") {
               connectionState = false;
             } else {
              client.println("HTTP/1.1 200 OK");
              client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
              client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
              client.println("Content-Type: text/html; charset=utf-8");
              client.println("Access-Control-Allow-Origin: *");
              client.println("X-Content-Type-Options: nosniff");
              client.println();
              if (Feedback=="")
                Feedback=("<!DOCTYPE html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'></head><body><a onclick=\"location.href='https://'+location.hostname;\" target=\"_blank\">https://ip/</a><br><br><a href=\"?stop\" target=\"_blank\">http://ip/?stop</a></body></html>");
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

TaskHandle_t Task0;
void codeForTask0( void * parameter ) {
  while (true) {
    getRequest80();
    vTaskDelay(10);
  }
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  cameraInitial();
  initWiFi();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); 

  xTaskCreatePinnedToCore(
    codeForTask0,
    "Task 0",
    8192,
    NULL,
    1,
    &Task0,
    0
  );
  vTaskDelay(100);  

  server80.begin();
}

void loop()
{
  secureServer.loop();
}

void handleRoot(HTTPRequest * req, HTTPResponse * res) {
  String head = "--Taiwan\r\nContent-Type: image/jpeg\r\n\r\n";
  //String tail = "\r\n--Taiwan--\r\n";
              
  res->setHeader("Access-Control-Allow-Origin", "*");
  res->setHeader("Content-Type", "multipart/x-mixed-replace; boundary=Taiwan");
  connectionState = true;
  
  while(connectionState) {
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if(!fb) {
      Serial.println("Camera capture failed");
      delay(1000);
      ESP.restart();
    }
    res->print(head);
    
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        res->write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        res->write(fbBuf, remainder);
      }
    }
    esp_camera_fb_return(fb);
              
    res->print("\r\n");
      
    delay(10);
  }     
}

void handleStop(HTTPRequest * req, HTTPResponse * res) {
  connectionState = false;

  // Set content type of the response
  res->setHeader("Content-Type", "text/html");

  // Write a tiny HTTP page
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title></title></head>");
  res->println("<body></body>");
  res->println("</html>");  
}

void handle404(HTTPRequest * req, HTTPResponse * res) {
  // Discard request body, if we received any
  // We do this, as this is the default node and may also server POST/PUT requests
  req->discardRequestBody();

  // Set the response status
  res->setStatusCode(404);
  res->setStatusText("Not Found");

  // Set content type of the response
  res->setHeader("Content-Type", "text/html");

  // Write a tiny HTTP page
  res->println("<!DOCTYPE html>");
  res->println("<html>");
  res->println("<head><title>Not Found</title></head>");
  res->println("<body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body>");
  res->println("</html>");
}
