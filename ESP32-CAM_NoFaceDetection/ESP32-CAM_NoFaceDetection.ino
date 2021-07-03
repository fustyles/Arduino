/*
ESP32-CAM Remove Face detection function
Author : ChungYi Fu (Kaohsiung, Taiwan)  2021-6-30 21:00
https://www.facebook.com/francefu

AP IP: 192.168.4.1
http://192.168.xxx.xxx             //網頁首頁管理介面
http://192.168.xxx.xxx:81/stream   //取得串流影像        網頁語法 <img src="http://192.168.xxx.xxx:81/stream">
http://192.168.xxx.xxx/capture     //取得影像           網頁語法 <img src="http://192.168.xxx.xxx/capture">
http://192.168.xxx.xxx/status      //取得視訊參數值
http://192.168.xxx.xxx/wifi        //自訂Wi-Fi設定網頁
http://192.168.xxx.xxx/info        //自訂查詢網路資訊網頁

自訂指令格式 http://192.168.xxx.xxx/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

http://192.168.xxx.xxx/control?ip                      //取得APIP, STAIP
http://192.168.xxx.xxx/control?mac                     //取得MAC位址
http://192.168.xxx.xxx/control?digitalwrite=pin;value  //數位輸出
http://192.168.xxx.xxx/control?analogwrite=pin;value   //類比輸出
http://192.168.xxx.xxx/control?digitalread=pin         //數位讀取
http://192.168.xxx.xxx/control?analogread=pin          //類比讀取
http://192.168.xxx.xxx/control?touchread=pin           //觸碰讀取
http://192.168.xxx.xxx/control?restart                 //重啟電源
http://192.168.xxx.xxx/control?flash=value             //閃光燈 value= 0~255
http://192.168.xxx.xxx/control?servo=value             //伺服馬達 value= 0~180
http://192.168.xxx.xxx/control?relay=value             //繼電器 value = 0, 1

設定視訊參數(官方指令格式)  http://192.168.xxx.xxx/control?var=*****&val=*****

http://192.168.xxx.xxx/control?var=flash&val=value          //閃光燈 value= 0~255
http://192.168.xxx.xxx/control?var=framesize&val=value      //解析度 value = 10->UXGA(1600x1200), 9->SXGA(1280x1024), 8->XGA(1024x768) ,7->SVGA(800x600), 6->VGA(640x480), 5 selected=selected->CIF(400x296), 4->QVGA(320x240), 3->HQVGA(240x176), 0->QQVGA(160x120), 11->QXGA(2048x1564 for OV3660)
http://192.168.xxx.xxx/control?var=quality&val=value        //畫質 value = 10 ~ 63
http://192.168.xxx.xxx/control?var=brightness&val=value     //亮度 value = -2 ~ 2
http://192.168.xxx.xxx/control?var=contrast&val=value       //對比 value = -2 ~ 2
http://192.168.xxx.xxx/control?var=saturation&val=value     //飽和度 value = -2 ~ 2 
http://192.168.xxx.xxx/control?var=gainceiling&val=value    //自動增益上限(開啟時) value = 0 ~ 6
http://192.168.xxx.xxx/control?var=colorbar&val=value       //顏色條畫面 value = 0 or 1
http://192.168.xxx.xxx/control?var=awb&val=value            //白平衡 value = 0 or 1 
http://192.168.xxx.xxx/control?var=agc&val=value            //自動增益控制 value = 0 or 1 
http://192.168.xxx.xxx/control?var=aec&val=value            //自動曝光感測器 value = 0 or 1 
http://192.168.xxx.xxx/control?var=hmirror&val=value        //水平鏡像 value = 0 or 1 
http://192.168.xxx.xxx/control?var=vflip&val=value          //垂直翻轉 value = 0 or 1 
http://192.168.xxx.xxx/control?var=awb_gain&val=value       //自動白平衡增益 value = 0 or 1 
http://192.168.xxx.xxx/control?var=agc_gain&val=value       //自動增益(關閉時) value = 0 ~ 30
http://192.168.xxx.xxx/control?var=aec_value&val=value      //曝光值 value = 0 ~ 1200
http://192.168.xxx.xxx/control?var=aec2&val=value           //自動曝光控制 value = 0 or 1 
http://192.168.xxx.xxx/control?var=dcw&val=value            //使用自訂影像尺寸 value = 0 or 1 
http://192.168.xxx.xxx/control?var=bpc&val=value            //黑色像素校正 value = 0 or 1 
http://192.168.xxx.xxx/control?var=wpc&val=value            //白色像素校正 value = 0 or 1 
http://192.168.xxx.xxx/control?var=raw_gma&val=value        //原始伽瑪 value = 0 or 1 
http://192.168.xxx.xxx/control?var=lenc&val=value           //鏡頭校正 value = 0 or 1 
http://192.168.xxx.xxx/control?var=special_effect&val=value //特效 value = 0 ~ 6
http://192.168.xxx.xxx/control?var=wb_mode&val=value        //白平衡模式 value = 0 ~ 4
http://192.168.xxx.xxx/control?var=ae_level&val=value       //自動曝光層級 value = -2 ~ 2 

視訊參數說明
https://heyrick.eu/blog/index.php?diary=20210418
*/

const char* ssid = "teacher";        //WIFI連線帳號
const char* password = "87654321";   //WIFI連線密碼 (至少8碼)

//新增AP 連線帳號密碼 (首頁網址 http://192.168.4.1)
const char* apssid = "ESP32-CAM";          //不同台可設不同流水號區別
const char* appassword = "12345678";         //AP密碼至少要8個字元以上

char return_html[1024];

#include "soc/soc.h"             //用於電源不穩不重開機 
#include "soc/rtc_cntl_reg.h"    //用於電源不穩不重開機 

#include <WiFi.h>
#include "esp_camera.h"          //視訊函式
#include "img_converters.h"      //影像格式轉換函式
#include "fb_gfx.h"              //影像繪圖函式
#include "esp_http_server.h"     //HTTP Server函式
#include "esp_timer.h"           //計時器函式

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

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

//影像傳輸網頁標頭設定
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

void startCameraServer();

String Feedback="";   //自訂指令回傳客戶端訊息

//自訂指令參數值
String Command="";
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
byte ReceiveState=0;
byte cmdState=1;
byte strState=1;
byte questionstate=0;
byte equalstate=0;
byte semicolonstate=0;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //關閉電源不穩就重開機的設定
    
  Serial.begin(115200);
  Serial.setDebugOutput(true);  //開啟診斷輸出
  Serial.println();

  //視訊組態設定  https://github.com/espressif/esp32-camera/blob/master/driver/include/esp_camera.h
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
  s->set_framesize(s, FRAMESIZE_QVGA);    //解析度 UXGA(1600x1200), SXGA(1280x1024), XGA(1024x768), SVGA(800x600), VGA(640x480), CIF(400x296), QVGA(320x240), HQVGA(240x176), QQVGA(160x120), QXGA(2048x1564 for OV3660)

  //s->set_vflip(s, 1);    //設定垂直翻轉
  //s->set_hmirror(s, 1);  //設定水平鏡像

  //新增閃光燈
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);    

  //新增ssid顯示IP
  if (ssid!="") {
    for (int i=0;i<2;i++) {
      WiFi.begin(ssid, password);    //執行網路連線
    
      delay(1000);
      Serial.println("");
      Serial.print("Connecting to ");
      Serial.println(ssid);
      
      long int StartTime=millis();
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          if ((StartTime+5000) < millis()) break;    //等待10秒連線
      } 
    
      if (WiFi.status() == WL_CONNECTED) {    //若連線成功
        WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);   //設定SSID顯示客戶端IP         
        Serial.println("");
        Serial.println("STAIP address: ");
        Serial.println(WiFi.localIP());
        Serial.println("");
    
        for (int i=0;i<5;i++) {   //若連上WIFI設定閃光燈快速閃爍
          ledcWrite(4,10);
          delay(200);
          ledcWrite(4,0);
          delay(200);    
        }
    
        break;
      }
    } 
  }

  if (WiFi.status() != WL_CONNECTED) {    //若連線失敗
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);         

    for (int i=0;i<2;i++) {    //若連不上WIFI設定閃光燈慢速閃爍
      ledcWrite(4,10);
      delay(1000);
      ledcWrite(4,0);
      delay(1000);    
    }
  } 

  //指定AP端IP
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());  
  Serial.println("");
  
  startCameraServer();    //啟動視訊服務器

  //設定閃光燈為低電位
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);     
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}

//指令參數控制
static esp_err_t cmd_handler(httpd_req_t *req){
    char*  buf;    //存取網址後帶的參數字串
    size_t buf_len;
    char variable[128] = {0,};  //存取參數var值
    char value[128] = {0,};     //存取參數val值
    String myCmd = "";

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
          if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
            httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
          } 
          else {
            myCmd = String(buf);   //如果非官方格式不含var, val，則為自訂指令格式
          }
        }
        free(buf);
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    Feedback="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
    ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;     
    if (myCmd.length()>0) {
      myCmd = "?"+myCmd;  //網址後帶的參數字串轉換成自訂指令格式
      for (int i=0;i<myCmd.length();i++) {
        getCommand(char(myCmd.charAt(i)));  //拆解自訂指令參數字串
      }
    }

    if (cmd.length()>0) {
      Serial.println("");
      //Serial.println("Command: "+Command);
      Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
      Serial.println(""); 

      //自訂指令區塊  http://192.168.xxx.xxx/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
      if (cmd=="your cmd") {
        // You can do anything
        // Feedback="<font color=\"red\">Hello World</font>";   //可為一般文字或HTML語法
      }
      else if (cmd=="ip") {  //查詢APIP, STAIP
        Feedback="AP IP: "+WiFi.softAPIP().toString();    
        Feedback+="<br>";
        Feedback+="STA IP: "+WiFi.localIP().toString();
      }  
      else if (cmd=="mac") {  //查詢MAC位址
        Feedback="STA MAC: "+WiFi.macAddress();
      }  
      else if (cmd=="restart") {  //重設WIFI連線
        ESP.restart();
      }  
      else if (cmd=="digitalwrite") {  //數位輸出
        ledcDetachPin(P1.toInt());
        pinMode(P1.toInt(), OUTPUT);
        digitalWrite(P1.toInt(), P2.toInt());
      }   
      else if (cmd=="digitalread") {  //數位輸入
        Feedback=String(digitalRead(P1.toInt()));
      }
      else if (cmd=="analogwrite") {  //類比輸出
        if (P1=="4") {
          ledcAttachPin(4, 4);  
          ledcSetup(4, 5000, 8);
          ledcWrite(4,P2.toInt());     
        }
        else {
          ledcAttachPin(P1.toInt(), 9);
          ledcSetup(9, 5000, 8);
          ledcWrite(9,P2.toInt());
        }
      }       
      else if (cmd=="analogread") {  //類比讀取
        Feedback=String(analogRead(P1.toInt()));
      }
      else if (cmd=="touchread") {  //觸碰讀取
        Feedback=String(touchRead(P1.toInt()));
      }   
     else if (cmd=="restart") {  //重啟電源
        ESP.restart();
      }         
      else if (cmd=="flash") {  //閃光燈
        ledcAttachPin(4, 4);  
        ledcSetup(4, 5000, 8);   
        int val = P1.toInt();
        ledcWrite(4,val);  
      }
      else if(cmd=="servo") {  //伺服馬達接於IO2 (SG90 1638-7864)
        ledcAttachPin(2, 3);
        ledcSetup(3, 50, 16);
         
        int val = 7864-P1.toInt()*34.59; 
        if (val > 7864)
           val = 7864;
        else if (val < 1638)
          val = 1638; 
        ledcWrite(3, val);
      }
      else if (cmd=="relay") {  //繼電器接於IO13
        pinMode(13, OUTPUT);  
        digitalWrite(13, P1.toInt());  
      }     
      else if (cmd=="resetwifi") {  //重設網路連線  
        for (int i=0;i<2;i++) {
          WiFi.begin(P1.c_str(), P2.c_str());
          Serial.print("Connecting to ");
          Serial.println(P1);
          long int StartTime=millis();
          while (WiFi.status() != WL_CONNECTED) {
              delay(500);
              if ((StartTime+5000) < millis()) break;
          } 
          Serial.println("");
          Serial.println("STAIP: "+WiFi.localIP().toString());
          Feedback="STAIP: "+WiFi.localIP().toString();
  
          if (WiFi.status() == WL_CONNECTED) {
            WiFi.softAP((WiFi.localIP().toString()+"_"+P1).c_str(), P2.c_str());
            for (int i=0;i<2;i++) {    //若連不上WIFI設定閃光燈慢速閃爍
              ledcWrite(4,10);
              delay(300);
              ledcWrite(4,0);
              delay(300);    
            }
            break;
          }
        }
      } 
      else {
        Feedback="Command is not defined";
      }

      if (Feedback=="") Feedback=Command;  //若沒有設定回傳資料就回傳Command值
    
      const char *resp = Feedback.c_str();
      httpd_resp_set_type(req, "text/html");  //設定回傳資料格式
      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");  //允許跨網域讀取
      return httpd_resp_send(req, resp, strlen(resp));
    } 
    else {
      //官方指令區塊  http://192.168.xxx.xxx/control?var=xxx&val=xxx
      int val = atoi(value);
      sensor_t * s = esp_camera_sensor_get();
      int res = 0;

      if(!strcmp(variable, "framesize")) {  //解析度
          if(s->pixformat == PIXFORMAT_JPEG) res = s->set_framesize(s, (framesize_t)val);
      }
      else if(!strcmp(variable, "quality")) res = s->set_quality(s, val);  //畫質
      else if(!strcmp(variable, "contrast")) res = s->set_contrast(s, val);  //對比
      else if(!strcmp(variable, "brightness")) res = s->set_brightness(s, val);  //亮度
      else if(!strcmp(variable, "saturation")) res = s->set_saturation(s, val);  //飽和度
      else if(!strcmp(variable, "gainceiling")) res = s->set_gainceiling(s, (gainceiling_t)val);  //自動增益上限(開啟時)
      else if(!strcmp(variable, "colorbar")) res = s->set_colorbar(s, val);  //顏色條畫面
      else if(!strcmp(variable, "awb")) res = s->set_whitebal(s, val);  //白平衡
      else if(!strcmp(variable, "agc")) res = s->set_gain_ctrl(s, val);  //自動增益控制
      else if(!strcmp(variable, "aec")) res = s->set_exposure_ctrl(s, val);  //自動曝光感測器
      else if(!strcmp(variable, "hmirror")) res = s->set_hmirror(s, val);  //水平鏡像
      else if(!strcmp(variable, "vflip")) res = s->set_vflip(s, val);  //垂直翻轉
      else if(!strcmp(variable, "awb_gain")) res = s->set_awb_gain(s, val);  //自動白平衡增益
      else if(!strcmp(variable, "agc_gain")) res = s->set_agc_gain(s, val);  //自動增益(關閉時)
      else if(!strcmp(variable, "aec_value")) res = s->set_aec_value(s, val);  //曝光值
      else if(!strcmp(variable, "aec2")) res = s->set_aec2(s, val);  //自動曝光控制
      else if(!strcmp(variable, "dcw")) res = s->set_dcw(s, val);  //使用自訂影像尺寸
      else if(!strcmp(variable, "bpc")) res = s->set_bpc(s, val);  //黑色像素校正
      else if(!strcmp(variable, "wpc")) res = s->set_wpc(s, val);  //白色像素校正
      else if(!strcmp(variable, "raw_gma")) res = s->set_raw_gma(s, val);  //原始伽瑪
      else if(!strcmp(variable, "lenc")) res = s->set_lenc(s, val);  //鏡頭校正
      else if(!strcmp(variable, "special_effect")) res = s->set_special_effect(s, val);  //特效
      else if(!strcmp(variable, "wb_mode")) res = s->set_wb_mode(s, val);  //白平衡模式
      else if(!strcmp(variable, "ae_level")) res = s->set_ae_level(s, val);  //自動曝光層級
  
      if(res){
          return httpd_resp_send_500(req);
      }

      if (buf) {
        Feedback = String(buf);
        const char *resp = Feedback.c_str();
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        return httpd_resp_send(req, resp, strlen(resp));  //回傳參數字串
      }
      else {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        return httpd_resp_send(req, NULL, 0);
      }
    }
}

//網頁首頁程式碼變數 (ov2640)
static const char PROGMEM index_ov2640_html[] = R"rawliteral(
<!doctype html>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <title>ESP32 OV3660</title>
    </head>
    <body>
    ov2640
    </body>
</html>
)rawliteral";

//網頁首頁處理程序  http://192.168.xxx.xxx
static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    sensor_t * s = esp_camera_sensor_get();

    return httpd_resp_send(req, (const char *)index_ov2640_html, strlen(index_ov2640_html));
}

//自訂WI-FI設定網頁變數
static const char PROGMEM index_wifi_html[] = R"rawliteral(
  <!doctype html>
  <html>
      <head>
          <meta charset="utf-8">
          <meta name="viewport" content="width=device-width,initial-scale=1">
          <title>ESP32-CAM Wi-Fi</title>  
      </head>
      <body>
      WIFI SSID: <input type="text" id="ssid"><br>
      WIFI  PWD: <input type="text" id="pwd"><br>
      <input type="button" value="設定" onclick="location.href='/control?resetwifi='+document.getElementById('ssid').value+';'+document.getElementById('pwd').value;">
      </body>
  </html> 
)rawliteral";

//自訂WI-FI設定網頁處理程序  http://192.168.xxx.xxx/wifi
static esp_err_t wifi_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, (const char *)index_wifi_html, strlen(index_wifi_html));
}

//自訂網路資訊動態查詢網頁處理程序  http://192.168.xxx.xxx/info
static esp_err_t info_handler(httpd_req_t *req){
  //自訂網路資訊動態查詢網頁變數
  const char index_info_html[] PROGMEM = R"rawliteral(<!doctype html>
  <html>
  <head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  </head>
  <body>
  <table border="1">
  <tr><td>Soft AP IP</td><td>%s</td></tr>
  <tr><td>Local IP</td><td>%s</td></tr>
  <tr><td>Mac Address</td><td>%s</td></tr>
  </table>
  </body>
  </html>)rawliteral";

  //http://tw.gitbook.net/c_standard_library/c_function_sprintf.html
  sprintf(return_html, index_info_html, WiFi.softAPIP().toString().c_str(), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str());
  
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, return_html, strlen(return_html));
}

//test
static esp_err_t test_handler(httpd_req_t *req){
    String html = "<font color='red'>"+WiFi.localIP().toString()+"</font>";
    sprintf(return_html, html.c_str());
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, return_html, strlen(return_html));
}

static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

//影像截圖
static esp_err_t capture_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;

    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    size_t fb_len = 0;
    if(fb->format == PIXFORMAT_JPEG){
        fb_len = fb->len;
        res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    } else {
        jpg_chunking_t jchunk = {req, 0};
        res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
        httpd_resp_send_chunk(req, NULL, 0);
        fb_len = jchunk.len;
    }
    esp_camera_fb_return(fb);
    return res;
}

//影像串流
static esp_err_t stream_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
          if(fb->format != PIXFORMAT_JPEG){
              bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
              esp_camera_fb_return(fb);
              fb = NULL;
              if(!jpeg_converted){
                  Serial.println("JPEG compression failed");
                  res = ESP_FAIL;
              }
          } else {
              _jpg_buf_len = fb->len;
              _jpg_buf = fb->buf;
          }
        }

        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }                
        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if(res != ESP_OK){
            break;
        }
    }

    return res;
}

//設定選單初始值取回json格式
static esp_err_t status_handler(httpd_req_t *req){
    static char json_response[1024];

    sensor_t * s = esp_camera_sensor_get();
    char * p = json_response;
    *p++ = '{';

    p+=sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p+=sprintf(p, "\"quality\":%u,", s->status.quality);
    p+=sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p+=sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p+=sprintf(p, "\"saturation\":%d,", s->status.saturation);
    p+=sprintf(p, "\"sharpness\":%d,", s->status.sharpness);
    p+=sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
    p+=sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
    p+=sprintf(p, "\"awb\":%u,", s->status.awb);
    p+=sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
    p+=sprintf(p, "\"aec\":%u,", s->status.aec);
    p+=sprintf(p, "\"aec2\":%u,", s->status.aec2);
    p+=sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
    p+=sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
    p+=sprintf(p, "\"agc\":%u,", s->status.agc);
    p+=sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
    p+=sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
    p+=sprintf(p, "\"bpc\":%u,", s->status.bpc);
    p+=sprintf(p, "\"wpc\":%u,", s->status.wpc);
    p+=sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
    p+=sprintf(p, "\"lenc\":%u,", s->status.lenc);
    p+=sprintf(p, "\"vflip\":%u,", s->status.vflip);
    p+=sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
    p+=sprintf(p, "\"dcw\":%u,", s->status.dcw);
    p+=sprintf(p, "\"colorbar\":%u", s->status.colorbar);
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

void startCameraServer(){
    //https://github.com/espressif/esp-idf/blob/master/components/esp_http_server/include/esp_http_server.h
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();  //可在HTTPD_DEFAULT_CONFIG()中設定Server Port

    httpd_uri_t index_uri = {
        .uri       = "/",      //http://192.168.xxx.xxx/
        .method    = HTTP_GET,
        .handler   = index_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t status_uri = {
        .uri       = "/status",       //http://192.168.xxx.xxx/status
        .method    = HTTP_GET,
        .handler   = status_handler,
        .user_ctx  = NULL
    };    
    
    httpd_uri_t cmd_uri = {
        .uri       = "/control",      //http://192.168.xxx.xxx/control
        .method    = HTTP_GET,
        .handler   = cmd_handler,
        .user_ctx  = NULL
    };

    httpd_uri_t capture_uri = {
        .uri       = "/capture",      //http://192.168.xxx.xxx/capture
        .method    = HTTP_GET,
        .handler   = capture_handler,
        .user_ctx  = NULL
    };

   httpd_uri_t stream_uri = {
        .uri       = "/stream",       //http://192.168.xxx.xxx:81/stream
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };
    
   //自訂WI-FI設定網頁路徑
   httpd_uri_t wifi_uri = {
        .uri       = "/wifi",       //http://192.168.xxx.xxx/wifi
        .method    = HTTP_GET,
        .handler   = wifi_handler,
        .user_ctx  = NULL
    };

   //自訂網路資訊動態查詢網頁路徑
   httpd_uri_t info_uri = {
        .uri       = "/info",       //http://192.168.xxx.xxx/info
        .method    = HTTP_GET,
        .handler   = info_handler,
        .user_ctx  = NULL
    };

   //自訂網路資訊動態查詢網頁路徑
   httpd_uri_t test_uri = {
        .uri       = "/test",       //http://192.168.xxx.xxx/test
        .method    = HTTP_GET,
        .handler   = test_handler,
        .user_ctx  = NULL
    };                 

    Serial.printf("Starting web server on port: '%d'\n", config.server_port);  //TCP Port
    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        //註冊自訂網址路徑對應執行的函式
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
        httpd_register_uri_handler(camera_httpd, &status_uri);        
        httpd_register_uri_handler(camera_httpd, &capture_uri);        
        httpd_register_uri_handler(camera_httpd, &wifi_uri);   //註冊WI-FI設定網頁
        httpd_register_uri_handler(camera_httpd, &info_uri);   //註冊網路資訊動態查詢網頁
        httpd_register_uri_handler(camera_httpd, &test_uri);   //test               
    }

    config.server_port += 1;  //Stream Port
    config.ctrl_port += 1;  //UDP Port
    Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }    
}

//自訂指令拆解參數字串置入變數
void getCommand(char c)
{
  if (c=='?') ReceiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
  
  if (ReceiveState==1)
  {
    Command=Command+String(c);
    
    if (c=='=') cmdState=0;
    if (c==';') strState++;
  
    if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) P1=P1+String(c);
    if ((cmdState==0)&&(strState==2)&&(c!=';')) P2=P2+String(c);
    if ((cmdState==0)&&(strState==3)&&(c!=';')) P3=P3+String(c);
    if ((cmdState==0)&&(strState==4)&&(c!=';')) P4=P4+String(c);
    if ((cmdState==0)&&(strState==5)&&(c!=';')) P5=P5+String(c);
    if ((cmdState==0)&&(strState==6)&&(c!=';')) P6=P6+String(c);
    if ((cmdState==0)&&(strState==7)&&(c!=';')) P7=P7+String(c);
    if ((cmdState==0)&&(strState==8)&&(c!=';')) P8=P8+String(c);
    if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) P9=P9+String(c);
    
    if (c=='?') questionstate=1;
    if (c=='=') equalstate=1;
    if ((strState>=9)&&(c==';')) semicolonstate=1;
  }
}
