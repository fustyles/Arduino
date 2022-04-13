/*
ESP32-CAM Send still using MQTT
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-4-13 20:00
https://www.facebook.com/francefu

Library: 
https://www.arduino.cc/reference/en/libraries/pubsubclient/
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"

const char* ssid = "teacher";
const char* password = "87654321";

// ws://broker.emqx.io:8083/mqtt  (MQTT.js)
// wss://broker.emqx.io:8084/mqtt  (MQTT.js)
const char* mqtt_server = "broker.emqx.io";
const unsigned int mqtt_port = 1883;
#define MQTT_USER               ""
#define MQTT_PASSWORD           ""
#define MQTT_PUBLISH_TOPIC    "yourTopic/sendstill"
#define MQTT_SUBSCRIBE_TOPIC    "yourTopic/getstill"
    
WiFiClient espClient;
PubSubClient client_mqtt(espClient);
String getData = "";

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

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //關閉電源不穩就重開機的設定
    
  Serial.begin(115200);
  randomSeed(micros());

  initCamera();
  initWiFi();
  client_mqtt.setServer(mqtt_server, mqtt_port);
  client_mqtt.setCallback(callback);
  client_mqtt.setBufferSize(2048);
}

void loop() {
  if (!client_mqtt.connected()) {
    reconnect();
  }
  client_mqtt.loop();

  if (getData=="getstill") {
    getData = "";
    sendImage();  
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (length>0) {
    getData = "";
    for (int i = 0; i < length; i++) {
      char c = payload[i];
      getData+=String(c);
    }
    Serial.println(getData);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client_mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    if (client_mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("connected");
      client_mqtt.subscribe(MQTT_SUBSCRIBE_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client_mqtt.state());
      Serial.println(" try again in 5 seconds");
      
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
  s->set_framesize(s, FRAMESIZE_HQVGA);    //解析度 SVGA(800x600), VGA(640x480), CIF(400x296), QVGA(320x240), HQVGA(240x176), QQVGA(160x120), QXGA(2048x1564 for OV3660)

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
    if (client_mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      client_mqtt.publish(MQTT_PUBLISH_TOPIC, text.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client_mqtt.state());
    }
}

String sendImage() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed, Reset");
    delay(100);
    ESP.restart();
  }

  int imgSize = fb->len;
  int ps = MQTT_MAX_PACKET_SIZE;
  client_mqtt.beginPublish(MQTT_PUBLISH_TOPIC, imgSize, false);
  for (int i = 0; i < imgSize; i += ps) {
    int s = (imgSize - i < s) ? (imgSize - i) : ps;
    client_mqtt.write((uint8_t *)(fb->buf) + i, s);
  }

  boolean isPublished = client_mqtt.endPublish();
  if (isPublished)
    Serial.println("Publishing Photo to MQTT Successfully");
  else
    Serial.println("Publishing Photo to MQTT Failed");

  esp_camera_fb_return(fb);
}
