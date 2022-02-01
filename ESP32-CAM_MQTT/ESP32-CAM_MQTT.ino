/*
ESP32-CAM MQTT
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-1-30 13:00
https://www.facebook.com/francefu

Library: 
https://www.arduino.cc/reference/en/libraries/pubsubclient/

Command Format :  
?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

?ip
?mac
?restart
?resetwifi=ssid;password
?inputpullup=pin
?pinmode=pin;value
?digitalwrite=pin;value
?analogwrite=pin;value;channel
?digitalread=pin
?analogread=pin
?touchread=pin  
?getstill 
?framesize=size      //size= UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
?quality=value       // value = 10 to 63
?brightness=value    // value = -2 to 2
?contrast=value      // value = -2 to 2 
?flash=value;channel         //value= 0~255
?servo=pin;value;channel     //value= 0~180
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "Base64.h"

const char* ssid = "teacher";
const char* password = "87654321";

const char* mqtt_server = "broker.emqx.io";
const unsigned int mqtt_port = 1883;
#define MQTT_USER               ""
#define MQTT_PASSWORD           ""
#define MQTT_PUBLISH_TOPIC    "yourtopic/send"
#define MQTT_SUBSCRIBE_TOPIC    "yourtopic/get"
    
WiFiClient espClient;
PubSubClient client(espClient);

//Arduino IDE開發版選擇 ESP32 Wrover Module

//ESP32-CAM 安信可模組腳位設定
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

//自訂指令參數值
String command="";
String cmd="";
String P1="";
String P2="";
String P3="";
String P4="";
String P5="";
String P6="";
String P7="";
String P8="";
String P9="";

//自訂指令拆解狀態值
byte receiveState=0;
byte cmdState=1;
byte pState=1;
byte questionState=0;
byte equalState=0;
byte semicolonState=0;

String feedback = "";

void executeCommand() {
  //Serial.println("");
  //Serial.println("command: "+command);
  //Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
  //Serial.println("");
  
  if (cmd=="your cmd") {
    // You can do anything
    // feedback="Hello World";
  }
  else if (cmd=="ip") {
    feedback=WiFi.localIP().toString();
  }  
  else if (cmd=="mac") {
    feedback=WiFi.macAddress();
  }  
  else if (cmd=="restart") {
    ESP.restart();
  }
  else if (cmd=="resetwifi") {
    WiFi.begin(P1.c_str(), P2.c_str());
    Serial.print("Connecting to ");
    Serial.println(P1);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("STAIP: "+WiFi.localIP().toString());
        feedback=WiFi.localIP().toString();
    }
    else
        feedback="failed";
  }     
  else if (cmd=="inputpullup") {
    pinMode(P1.toInt(), INPUT_PULLUP);
  }  
  else if (cmd=="pinmode") {
    if (P2.toInt()==1)
      pinMode(P1.toInt(), OUTPUT);
    else
      pinMode(P1.toInt(), INPUT);
  }        
  else if (cmd=="digitalwrite") {
    ledcDetachPin(P1.toInt());
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(), P2.toInt());
  }   
  else if (cmd=="digitalread") {
    feedback=String(digitalRead(P1.toInt()));
  }
  else if (cmd=="analogwrite") {
    if (P3=="") P3="3";
    int channel = P3.toInt();
    ledcAttachPin(P1.toInt(), channel);
    ledcSetup(channel, 5000, 8);
    ledcWrite(channel,P2.toInt());
  }     
  else if (cmd=="analogread") {
    feedback=String(analogRead(P1.toInt()));
  }
  else if (cmd=="touchread") {
    feedback=String(touchRead(P1.toInt()));
  }
  else if (cmd=="flash") {
    if (P3=="") P3="4";
    int channel = P3.toInt();       
    ledcAttachPin(4, channel);  
    ledcSetup(channel, 5000, 8);   
    int val = P1.toInt();
    ledcWrite(channel,val);  
  }
  else if(cmd=="servo") {
    if (P3=="") P3="5";
    int channel = P3.toInt();     
    ledcAttachPin(P1.toInt(), channel);
    ledcSetup(channel, 50, 16);

    int val = 7864-P2.toInt()*34.59; 
    if (val > 7864)
       val = 7864;
    else if (val < 1638)
      val = 1638; 
    ledcWrite(channel, val);
  }
  else if (cmd=="framesize") { 
    sensor_t * s = esp_camera_sensor_get();  
    if (P1=="QQVGA")
      s->set_framesize(s, FRAMESIZE_QQVGA);
    else if (P1=="HQVGA")
      s->set_framesize(s, FRAMESIZE_HQVGA);
    else if (P1=="QVGA")
      s->set_framesize(s, FRAMESIZE_QVGA);
    else if (P1=="CIF")
      s->set_framesize(s, FRAMESIZE_CIF);
    else if (P1=="VGA")
      s->set_framesize(s, FRAMESIZE_VGA);  
    else if (P1=="SVGA")
      s->set_framesize(s, FRAMESIZE_SVGA);
    else if (P1=="XGA")
      s->set_framesize(s, FRAMESIZE_XGA);
    else if (P1=="SXGA")
      s->set_framesize(s, FRAMESIZE_SXGA);
    else if (P1=="UXGA")
      s->set_framesize(s, FRAMESIZE_UXGA);           
    else 
      s->set_framesize(s, FRAMESIZE_QVGA);     
  }
  else if (cmd=="quality") { 
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_quality(s, val);
  }
  else if (cmd=="contrast") {
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_contrast(s, val);
  }
  else if (cmd=="brightness") {
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt();  
    s->set_brightness(s, val);  
  }    
  else if (cmd=="getstill") {
    feedback=sendImage();
    if (feedback=="") {
      Serial.println("feedback: "+feedback);
      Serial.println("");
      return;
    }
  }     
  else {
    feedback="Command is not defined";
  } 

  Serial.println("feedback: "+feedback);
  Serial.println("");
  if (feedback!="") 
      sendText(feedback); 
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //關閉電源不穩就重開機的設定
    
  Serial.begin(115200);
  randomSeed(micros());

  initCamera();
  initWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(2048);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  feedback="";command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;
    
  Serial.print("[");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    char c = payload[i];
    getCommand(payload[i]);
    Serial.print(c);
  }
  Serial.println();
    
  if (cmd!="") 
      executeCommand();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(MQTT_PUBLISH_TOPIC, "hello world");
      // ... and resubscribe
      client.subscribe(MQTT_SUBSCRIBE_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void initCamera() {
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
  
  //
  // WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
  //            Ensure ESP32 Wrover Module or other board with PSRAM is selected
  //            Partial images will be transmitted if image exceeds buffer size
  //   
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){  //是否有PSRAM(Psuedo SRAM)記憶體IC
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  //視訊初始化
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  //可自訂視訊框架預設大小(解析度大小)
  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);    //解析度 SVGA(800x600), VGA(640x480), CIF(400x296), QVGA(320x240), HQVGA(240x176), QQVGA(160x120), QXGA(2048x1564 for OV3660)

  //s->set_vflip(s, 1);  //垂直翻轉
  //s->set_hmirror(s, 1);  //水平鏡像

  //閃光燈(GPIO4)
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  
  for (int i=0;i<2;i++) {
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
      Serial.println("");
      Serial.println("STAIP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");
  
      pinMode(2, OUTPUT);
      for (int j=0;j<5;j++) {
        digitalWrite(2,HIGH);
        delay(100);
        digitalWrite(2,LOW);
        delay(100);
      }
      break;
    }
  } 

  if (WiFi.status() != WL_CONNECTED) {    //若連線失敗
    pinMode(2, OUTPUT);
    for (int k=0;k<2;k++) {
      digitalWrite(2,HIGH);
      delay(1000);
      digitalWrite(2,LOW);
      delay(1000);
    }
  } 
}

void sendText(String text) {
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      client.publish(MQTT_PUBLISH_TOPIC, text.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
    }
}

String sendImage() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return "Camera capture failed";
  }  

  char *input = (char *)fb->buf;
  char output[base64_enc_len(3)];
  String imageFile = "data:image/jpeg;base64,";
  for (int i=0;i<fb->len;i++) {
    base64_encode(output, (input++), 3);
    if (i%3==0) imageFile += String(output);
  }
  int fbLen = imageFile.length();
  
  String clientId = "ESP32-";
  clientId += String(random(0xffff), HEX);
  if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
    //https://github.com/knolleary/pubsubclient/blob/master/src/PubSubClient.h
    
    client.beginPublish(MQTT_PUBLISH_TOPIC, fbLen, true);

    String str = "";
    for (size_t n=0;n<fbLen;n=n+2048) {
      if (n+2048<fbLen) {
        str = imageFile.substring(n, n+2048);
        client.write((uint8_t*)str.c_str(), 2048);
      }
      else if (fbLen%2048>0) {
        size_t remainder = fbLen%2048;
        str = imageFile.substring(n, n+remainder);
        client.write((uint8_t*)str.c_str(), remainder);
      }
    }  
    
    client.endPublish();
    
    esp_camera_fb_return(fb);
    
    return "";
  }
  esp_camera_fb_return(fb);
  return "failed, rc="+client.state();
}

void getCommand(char c) {
  if (c=='?') receiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) receiveState=0;
  
  if (receiveState==1) {
    command=command+String(c);
    
    if (c=='=') cmdState=0;
    if (c==';') pState++;
  
    if ((cmdState==1)&&((c!='?')||(questionState==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(pState==1)&&((c!='=')||(equalState==1))) P1=P1+String(c);
    if ((cmdState==0)&&(pState==2)&&(c!=';')) P2=P2+String(c);
    if ((cmdState==0)&&(pState==3)&&(c!=';')) P3=P3+String(c);
    if ((cmdState==0)&&(pState==4)&&(c!=';')) P4=P4+String(c);
    if ((cmdState==0)&&(pState==5)&&(c!=';')) P5=P5+String(c);
    if ((cmdState==0)&&(pState==6)&&(c!=';')) P6=P6+String(c);
    if ((cmdState==0)&&(pState==7)&&(c!=';')) P7=P7+String(c);
    if ((cmdState==0)&&(pState==8)&&(c!=';')) P8=P8+String(c);
    if ((cmdState==0)&&(pState>=9)&&((c!=';')||(semicolonState==1))) P9=P9+String(c);
    
    if (c=='?') questionState=1;
    if (c=='=') equalState=1;
    if ((pState>=9)&&(c==';')) semicolonState=1;
  }
}
