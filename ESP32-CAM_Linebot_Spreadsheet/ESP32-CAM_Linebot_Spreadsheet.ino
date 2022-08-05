/*
ESP32-CAM LineBot using spreadsheet
Author : ChungYi Fu (Kaohsiung, Taiwan)   2022/8/5 01:30
https://www.facebook.com/francefu

apps Script
https://github.com/fustyles/webduino/blob/gs/Linebot_Spreadsheet_googledrive.gs
 */
 
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Base64.h"
#include "esp_camera.h"

char _lwifi_ssid[] = "teacher";
char _lwifi_pass[] = "87654321";
String spreadsheetID = "1zztiZMyQ7HplFp0cHc0dKpiomZLDDfu8nJuStz_hFIss";
String spreadsheetName = "工作表1";
String appsScriptID = "AKfycbx-9F6o1gl6-404JBXkpiJQ0-wCyn8tFlxPXZOqiX3qeuoxjlMlv9FdhsXkZY4jYHmss";

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

String spreadsheetQueryData = "{\"values\":[]}";

void setup()
{
  Serial.begin(115200);
  initWiFi();
  initCamera();
}

void loop()
{
  spreadsheetQueryData = Spreadsheet_query("select A, B limit 1 offset 0", String(spreadsheetID), String(spreadsheetName));
  String message = Spreadsheet_getcell_query(0, 0);
  String replyToken = Spreadsheet_getcell_query(0, 1);
  if (message != "") {
    Serial.println(message);
    Serial.println(replyToken);
    if (message == "on") {
      message = "Led on";
      tcp_https_esp32("POST", "script.google.com", "/macros/s/"+appsScriptID+"/exec?response="+urlencode(message)+"&token="+String(replyToken), 443, 3000);
    } else if (message == "off") {
      message = "Led off";
      tcp_https_esp32("POST", "script.google.com", "/macros/s/"+appsScriptID+"/exec?response="+urlencode(message)+"&token="+String(replyToken), 443, 3000);
    } else if (message == "getstill") {
      SendCapturedImage(replyToken);
    }
  }
  delay(1000);
}

void initWiFi() {

  for (int i=0;i<2;i++) {
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
      Serial.println("");
      Serial.println("STAIP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");

      break;
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

String Spreadsheet_query(String sql, String mySpreadsheetid, String mySpreadsheetname) {
  sql = urlencode(sql);
  mySpreadsheetname = urlencode(mySpreadsheetname);
  const char* myDomain = "docs.google.com";
  String getAll="", getBody = "", getData = "";
  //Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  if (client_tcp.connect(myDomain, 443)) {
    //Serial.println("Connection successful");
    String url = "https://docs.google.com/spreadsheets/d/"+mySpreadsheetid+"/gviz/tq?tqx=out:json&sheet="+mySpreadsheetname+"&tq="+sql;
    client_tcp.println("GET "+url+" HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Type: application/json");
    client_tcp.println();
    int waitTime = 10000;
    long startTime = millis();
    boolean state = false;
    boolean start = false;

    while ((startTime + waitTime) > millis()) {
      //Serial.print(".");
      delay(100);
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (getBody.indexOf("\"rows\":[")!=-1) start = true;
          if (getData.indexOf("],")!=-1) start = false;
          if (state==true&&c!='\n'&&c!='\r') getBody += String(c);
          if (start==true&&c!='\n'&&c!='\r') getData += String(c);
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
    //Serial.println("");
    if (getBody.indexOf("error")!=-1||getData=="")
    	return "{\"values\":[]}";
    getData = "{\"values\":[" + getData.substring(0, getData.length()-2) + "]}";
    return getData;
  }
  else {
    Serial.println("Connected to " + String(myDomain) + " failed.");
    return "{\"values\":[]}";
  }
}

String Spreadsheet_getcell_query(int row, int col) {
    if (spreadsheetQueryData!="") {
    	JsonObject obj;
    	DynamicJsonDocument doc(1024);
    	deserializeJson(doc, spreadsheetQueryData);
    	obj = doc.as<JsonObject>();
  		if ((obj["values"].size()<row+1)||(obj["values"][0]["c"].size()<col+1))
  			return "";
      	return obj["values"][row]["c"][col]["v"].as<String>();
    }
    else
		return "";
}

String tcp_https_esp32(String type,String domain,String request,int port,int waittime) {
  String getAll="", getBody="";
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  if (client_tcp.connect(domain.c_str(), port)) {
    //Serial.println("Connected to "+domain+" successfully.");
    client_tcp.println(type + " " + request + " HTTP/1.1");
    client_tcp.println("Host: " + domain);
    client_tcp.println("Connection: close");
    client_tcp.println("Content-Length: 0");
    client_tcp.println();
    boolean state = false;
    long startTime = millis();
    while ((startTime + waittime) > millis()) {
      while (client_tcp.available()) {
        char c = client_tcp.read();
        if (c == '\n') {
          if (getAll.length()==0) state=true;
           getAll = "";
        }
        else if (c != '\r')
          getAll += String(c);
          if (state==true) getBody += String(c);
          startTime = millis();
        }
        if (getBody.length()!= 0) break;
      }
      client_tcp.stop();
  }
  else {
    getBody="Connected to "+domain+" failed.";
    Serial.println("Connected to "+domain+" failed.");
  }
  return getBody;
}

String urlencode(String str)
{
  String encodedString="";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i =0; i < str.length(); i++) {
    c=str.charAt(i);
    if (c == ' ') {
      encodedString+= '+';
    } else if (isalnum(c)) {
      encodedString+=c;
    } else {
      code1=(c & 0xf)+'0';
      if ((c & 0xf) >9) {
          code1=(c & 0xf) - 10 + 'A';
      }
      c=(c>>4)&0xf;
      code0=c+'0';
      if (c > 9) {
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

String SendCapturedImage(String token) {
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
  
  //Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above
  
  if (client_tcp.connect(myDomain, 443)) {
    //Serial.println("Connection successful");
    
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    String Data = "token="+token+"&myFile=";
    
    client_tcp.println("POST /macros/s/"+appsScriptID+"/exec HTTP/1.1");
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
      //Serial.print(".");
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
