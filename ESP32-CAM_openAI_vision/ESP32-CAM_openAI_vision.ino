/*
ESP32-CAM Use OpenAI Vision to analyze still image content.

Author : ChungYi Fu (Kaohsiung, Taiwan)  2024-8-19 14:00
https://www.facebook.com/francefu
*/

char wifi_ssid[] = "teacher";
char wifi_pass[] = "12345678";

String openAI_Key = "sk-proj-xxx-xxx-xxx";

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "Base64.h"
#include <ArduinoJson.h>

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
  initWiFi();
  
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
  s->set_framesize(s, FRAMESIZE_CIF);
  
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  delay(5000);

  String response = "";	
  String openAI_Chat = "Please analyze the image content";

  String openAI_ImageUrl = "https://upload.wikimedia.org/wikipedia/commons/c/c0/Fm_shiba_inu_puppy.jpg";
  response = SendImageUrlToOpenaiVision(openAI_Key, openAI_Chat, openAI_ImageUrl); 
  Serial.println(response);
    
  response = SendStillToOpenaiVision(openAI_Key, openAI_Chat);
  Serial.println(response);

  String openAI_Behavior = "Please follow the guidelines: (1) If the scenario description indicates the presence of people, return '1'. (2) If the scenario description indicates there are no people, return '0'. (3) If it cannot be determined, return '-1'. (4) Do not provide additional explanations.";
  openAI_Chat = response;
  response = SendMessageToChatGPT(openAI_Key, openAI_Behavior, openAI_Chat); 
  Serial.println(response);	
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

String SendStillToOpenaiVision(String key, String message) {
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  const char* myDomain = "api.openai.com";  
  Serial.println("Connect to " + String(myDomain));
  String getResponse="", Feedback="";
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
	
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if(!fb) {
      Serial.println("Camera capture failed");
      delay(1000);
      ESP.restart();
      return "Camera capture failed";
    }
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += String(output);
    }
    
    String Data = "{\"model\": \"gpt-4o-mini\", \"messages\": [{\"role\": \"user\",\"content\": [{ \"type\": \"text\", \"text\": \""+message+"\"},{\"type\": \"image_url\", \"image_url\": {\"url\": \""+imageFile+"\"}}]}]}";

    client_tcp.println("POST /v1/chat/completions HTTP/1.1");
    client_tcp.println("Connection: close");
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("Authorization: Bearer " + key);
    client_tcp.println("Content-Type: application/json; charset=utf-8");
    client_tcp.println("Content-Length: " + String(Data.length()));
    client_tcp.println("Connection: close");
    client_tcp.println();

    int Index;
    for (Index = 0; Index < Data.length(); Index = Index+1024) {
      client_tcp.print(Data.substring(Index, Index+1024));
    }
    esp_camera_fb_return(fb);
	  
    int waitTime = 10000;
    long startTime = millis();
    boolean state = false;
    boolean markState = false;
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);
      while (client_tcp.available())  {
          char c = client_tcp.read();
          if (String(c)=="{") markState=true;
          if (state==true&&markState==true) Feedback += String(c);
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
    client_tcp.stop();
    Serial.println("");
    //Serial.println(Feedback);

    JsonObject obj;
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, Feedback);
    obj = doc.as<JsonObject>();
    getResponse = obj["choices"][0]["message"]["content"].as<String>();
    if (getResponse == "null")
      getResponse = obj["error"]["message"].as<String>();
    //Serial.println(getResponse);
  }
  else {
    getResponse = "Connected to " + String(myDomain) + " failed.";
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }

  return getResponse;
}

String SendImageUrlToOpenaiVision(String key, String message, String url) {
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  const char* myDomain = "api.openai.com";  
  Serial.println("Connect to " + String(myDomain));
  String getResponse="", Feedback="";
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    String Data = "{\"model\": \"gpt-4o-mini\", \"messages\": [{\"role\": \"user\",\"content\": [{ \"type\": \"text\", \"text\": \""+message+"\"},{\"type\": \"image_url\", \"image_url\": {\"url\": \""+url+"\"}}]}]}";

    client_tcp.println("POST /v1/chat/completions HTTP/1.1");
    client_tcp.println("Connection: close");
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("Authorization: Bearer " + key);
    client_tcp.println("Content-Type: application/json; charset=utf-8");
    client_tcp.println("Content-Length: " + String(Data.length()));
    client_tcp.println("Connection: close");
    client_tcp.println();

    int Index;
    for (Index = 0; Index < Data.length(); Index = Index+1024) {
      client_tcp.print(Data.substring(Index, Index+1024));
    }
    
    int waitTime = 10000;
    long startTime = millis();
    boolean state = false;
    boolean markState = false;
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);
      while (client_tcp.available())  {
          char c = client_tcp.read();
          if (String(c)=="{") markState=true;
          if (state==true&&markState==true) Feedback += String(c);
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
    client_tcp.stop();
    Serial.println("");
    //Serial.println(Feedback);

    JsonObject obj;
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, Feedback);
    obj = doc.as<JsonObject>();
    getResponse = obj["choices"][0]["message"]["content"].as<String>();
    if (getResponse == "null")
      getResponse = obj["error"]["message"].as<String>();
    //Serial.println(getResponse);
  }
  else {
    getResponse = "Connected to " + String(myDomain) + " failed.";
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }

  return getResponse;
}

String SendMessageToChatGPT(String key, String behavior, String message) {
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  const char* myDomain = "api.openai.com";  
  Serial.println("Connect to " + String(myDomain));
  String getResponse="", Feedback="";
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    String Data = "{\"model\": \"gpt-4o-mini\", \"messages\": [{\"role\": \"system\", \"content\": \""+behavior+"\"}, {\"role\": \"user\", \"content\": \""+message+"\"}]}";

    client_tcp.println("POST /v1/chat/completions HTTP/1.1");
    client_tcp.println("Connection: close");
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("Authorization: Bearer " + key);
    client_tcp.println("Content-Type: application/json; charset=utf-8");
    client_tcp.println("Content-Length: " + String(Data.length()));
    client_tcp.println("Connection: close");
    client_tcp.println();

    int Index;
    for (Index = 0; Index < Data.length(); Index = Index+1024) {
      client_tcp.print(Data.substring(Index, Index+1024));
    }
    
    int waitTime = 10000;
    long startTime = millis();
    boolean state = false;
    boolean markState = false;
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);
      while (client_tcp.available())  {
          char c = client_tcp.read();
          if (String(c)=="{") markState=true;
          if (state==true&&markState==true) Feedback += String(c);
          if (c == '\n') {
            if (getResponse.length()==0) state=true;
            getResponse = "";
          } else if (c != '\r')
            getResponse += String(c);
          startTime = millis();
       }
       if (Feedback.length()>0) break;
    }
    client_tcp.stop();
    Serial.println("");
    //Serial.println(Feedback);

    JsonObject obj;
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, Feedback);
    obj = doc.as<JsonObject>();
    getResponse = obj["choices"][0]["message"]["content"].as<String>();
    if (getResponse == "null")
      getResponse = obj["error"]["message"].as<String>();   // api Key?image file?image resolution?
  }
  else {
    getResponse = "Connected to " + String(myDomain) + " failed.";
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }

  return getResponse;
}
