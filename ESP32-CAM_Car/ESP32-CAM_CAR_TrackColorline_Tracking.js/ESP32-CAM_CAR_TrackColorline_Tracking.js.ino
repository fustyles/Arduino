/*
ESP32-CAM Line Following Car with Color Recognition (Tracking.js)

Author : ChungYi Fu (Kaohsiung, Taiwan)  2021-7-11 16:00
https://www.facebook.com/francefu

Motor Driver IC -> PWM1(IO12, IO13), PWM2(IO14, IO15)
Don't use L9110S.

Servo -> IO2 (伺服馬達與ESP32-CAM共地外接電源)

物件類別
https://github.com/tensorflow/tfjs-models/blob/master/coco-ssd/src/classes.ts

http://192.168.xxx.xxx             //網頁首頁管理介面
http://192.168.xxx.xxx:81/stream   //取得串流影像       <img src="http://192.168.xxx.xxx:81/stream">
http://192.168.xxx.xxx/capture     //取得影像          <img src="http://192.168.xxx.xxx/capture">
http://192.168.xxx.xxx/status      //取得視訊參數值

自訂指令格式 :  
http://APIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

預設AP端IP： 192.168.4.1

自訂指令格式 http://192.168.xxx.xxx/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://192.168.xxx.xxx/control?ip                      //取得APIP, STAIP
http://192.168.xxx.xxx/control?mac                     //取得MAC位址
http://192.168.xxx.xxx/control?restart                 //重啟ESP32-CAM
http://192.168.xxx.xxx/control?digitalwrite=pin;value  //數位輸出
http://192.168.xxx.xxx/control?analogwrite=pin;value   //類比輸出
http://192.168.xxx.xxx/control?digitalread=pin         //數位讀取
http://192.168.xxx.xxx/control?analogread=pin          //類比讀取
http://192.168.xxx.xxx/control?touchread=pin           //觸碰讀取
http://192.168.xxx.xxx/control?resetwifi=ssid;password   //重設Wi-Fi網路
http://192.168.xxx.xxx/control?flash=value               //內建閃光燈 value= 0-255
http://192.168.xxx.xxx/control?servo=value               //伺服馬達 value = 0-180

官方指令格式 http://192.168.xxx.xxx/control?var=***&val=***
http://192.168.xxx.xxx/control?var=framesize&val=value    // value = 10->UXGA(1600x1200), 9->SXGA(1280x1024), 8->XGA(1024x768) ,7->SVGA(800x600), 6->VGA(640x480), 5 selected=selected->CIF(400x296), 4->QVGA(320x240), 3->HQVGA(240x176), 0->QQVGA(160x120)
http://192.168.xxx.xxx/control?var=quality&val=value      // value = 10 ~ 63
http://192.168.xxx.xxx/control?var=brightness&val=value   // value = -2 ~ 2
http://192.168.xxx.xxx/control?var=contrast&val=value     // value = -2 ~ 2
http://192.168.xxx.xxx/control?var=hmirror&val=value      // value = 0 or 1 
http://192.168.xxx.xxx/control?var=vflip&val=value        // value = 0 or 1 
http://192.168.xxx.xxx/control?var=flash&val=value        // value = 0 ~ 255 
*/

//輸入WIFI連線帳號密碼
const char* ssid = "teacher";
const char* password = "87654321";

//輸入AP端連線帳號密碼  http://192.168.4.1
const char* apssid = "esp32-cam";
const char* appassword = "12345678";         //AP密碼至少要8個字元以上 

int pinServo = 2;      //伺服馬達腳位
int servoAngle = 30;   //伺服馬達初始角度
int speedR = 255;  //You can adjust the speed of the wheel. (IO12, IO13)
int speedL = 255;  //You can adjust the speed of the wheel. (IO14, IO15)
float decelerate = 0.4;   // value = 0-1

#include <WiFi.h>
#include "soc/soc.h"             //用於電源不穩不重開機 
#include "soc/rtc_cntl_reg.h"    //用於電源不穩不重開機
#include <esp32-hal-ledc.h>      //用於控制伺服馬達 

//官方函式庫
#include "esp_http_server.h"
#include "esp_camera.h"
#include "img_converters.h"

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

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

//ESP32-CAM模組腳位設定
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
  
  //可動態改變視訊框架大小(解析度大小)
  s->set_framesize(s, FRAMESIZE_QVGA);

  //鏡像
  //s->set_hmirror(s, 1);
  //s->set_vflip(s, 1);  //垂直翻轉

  //伺服馬達
  ledcAttachPin(pinServo, 3);  
  ledcSetup(3, 50, 16);  
  servo_rotate(3, servoAngle);    
  
  //閃光燈(GPIO4)
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);

  //馬達驅動IC
  ledcAttachPin(12, 5);
  ledcSetup(5, 2000, 8);      
  ledcAttachPin(13, 6);
  ledcSetup(6, 2000, 8);
  ledcWrite(6, 0);  //gpio13初始化呈高電位，改設定為低電位
  ledcAttachPin(15, 7);
  ledcSetup(7, 2000, 8);      
  ledcAttachPin(14, 8);
  ledcSetup(8, 2000, 8); 
        
  WiFi.mode(WIFI_AP_STA);  //其他模式 WiFi.mode(WIFI_AP); WiFi.mode(WIFI_STA);

  //指定Client端靜態IP
  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

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
  
  startCameraServer(); 

  //設定閃光燈為低電位
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); 
}

void loop() {

}

void servo_rotate(int channel, int angle) {
    int val = 7864-angle*34.59; 
    if (val > 7864)
       val = 7864;
    else if (val < 1638)
      val = 1638; 
    ledcWrite(channel, val);
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
      else if (cmd=="restart") {
        ESP.restart();
      }  
      else if (cmd=="digitalwrite") {
        ledcDetachPin(P1.toInt());
        pinMode(P1.toInt(), OUTPUT);
        digitalWrite(P1.toInt(), P2.toInt());
      }   
      else if (cmd=="digitalread") {
        Feedback=String(digitalRead(P1.toInt()));
      }
      else if (cmd=="analogwrite") {   
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
      else if (cmd=="analogread") {
        Feedback=String(analogRead(P1.toInt()));
      }
      else if (cmd=="touchread") {
        Feedback=String(touchRead(P1.toInt()));
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
      else if (cmd=="flash") {  //控制內建閃光燈
        ledcAttachPin(4, 4);  
        ledcSetup(4, 5000, 8);   
        int val = P1.toInt();
        ledcWrite(4,val);  
      }
      else if (cmd=="serial") { 
        if (P1!=""&P1!="stop") Serial.println(P1);
        if (P2!=""&P2!="stop") Serial.println(P2);
        Serial.println();
      }
      else if (cmd=="car") {  //自走車運動狀態
        int val = P1.toInt(); 
        if (val==1) {  //前進 http://192.168.xxx.xxx/control?car=1
          Serial.println("Front");     
          ledcWrite(5,speedR);
          ledcWrite(6,0);
          ledcWrite(7,speedL);
          ledcWrite(8,0);   
        }
        else if (val==2) {  //左轉 http://192.168.xxx.xxx/control?car=2
          Serial.println("Left");     
          ledcWrite(5,speedR*decelerate);
          ledcWrite(6,0);
          ledcWrite(7,0);
          ledcWrite(8,speedL*decelerate);  
        }
        else if (val==3) {  //停止 http://192.168.xxx.xxx/control?car=3
          Serial.println("Stop");      
          ledcWrite(5,0);
          ledcWrite(6,0);
          ledcWrite(7,0);
          ledcWrite(8,0);    
        }
        else if (val==4) {  //右轉 http://192.168.xxx.xxx/control?car=4
          Serial.println("Right");
          ledcWrite(5,0);
          ledcWrite(6,speedR*decelerate);
          ledcWrite(7,speedL*decelerate);
          ledcWrite(8,0);          
        }
        else if (val==5) {  //後退 http://192.168.xxx.xxx/control?car=5
          Serial.println("Back");      
          ledcWrite(5,0);
          ledcWrite(6,speedR);
          ledcWrite(7,0);
          ledcWrite(8,speedL);
        }  
        else if (val==6) {  //左前進 http://192.168.xxx.xxx/control?car=6
          Serial.println("FrontLeft");     
          ledcWrite(5,speedR);
          ledcWrite(6,0);
          ledcWrite(7,speedL*decelerate);
          ledcWrite(8,0);   
        }
        else if (val==7) {  //右前進 http://192.168.xxx.xxx/control?car=7
          Serial.println("FrontRight");     
          ledcWrite(5,speedR*decelerate);
          ledcWrite(6,0);
          ledcWrite(7,speedL);
          ledcWrite(8,0);   
        }  
        else if (val==8) {  //左後退 http://192.168.xxx.xxx/control?car=8
          Serial.println("LeftAfter");      
          ledcWrite(5,0);
          ledcWrite(6,speedR);
          ledcWrite(7,0);
          ledcWrite(8,speedL*decelerate);
        } 
        else if (val==9) {  //右後退 http://192.168.xxx.xxx/control?car=9
          Serial.println("RightAfter");      
          ledcWrite(5,0);
          ledcWrite(6,speedR*decelerate);
          ledcWrite(7,0);
          ledcWrite(8,speedL);
        }  
        if (P2!="") {
          //Serial.println("delay "+P2+" ms"); 
          delay(P2.toInt()); 
          Serial.println("Stop");     
          ledcWrite(5,0);
          ledcWrite(6,0);
          ledcWrite(7,0);
          ledcWrite(8,0);         
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
      //官方指令區塊，也可在此自訂指令  http://192.168.xxx.xxx/control?var=xxx&val=xxx
      int val = atoi(value);
      sensor_t * s = esp_camera_sensor_get();
      int res = 0;

      if(!strcmp(variable, "framesize")) {
        if(s->pixformat == PIXFORMAT_JPEG) 
          res = s->set_framesize(s, (framesize_t)val);
      }
      else if(!strcmp(variable, "quality")) res = s->set_quality(s, val);
      else if(!strcmp(variable, "contrast")) res = s->set_contrast(s, val);
      else if(!strcmp(variable, "brightness")) res = s->set_brightness(s, val);
      else if(!strcmp(variable, "hmirror")) res = s->set_hmirror(s, val);
      else if(!strcmp(variable, "vflip")) res = s->set_vflip(s, val);
      else if(!strcmp(variable, "flash")) {
        ledcAttachPin(4, 4);  
        ledcSetup(4, 5000, 8);        
        ledcWrite(4,val);
      } 
      else if(!strcmp(variable, "speedL")) {
        if (val > 255)
           val = 255;
        else if (val < 0)
          val = 0;       
        speedL = val;
        Serial.println("LeftSpeed = " + String(val)); 
      }  
      else if(!strcmp(variable, "speedR")) {
        if (val > 255)
           val = 255;
        else if (val < 0)
          val = 0;       
        speedR = val;
        Serial.println("RightSpeed = " + String(val)); 
      }  
      else if(!strcmp(variable, "decelerate")) {       
        decelerate = String(value).toFloat();
        Serial.println("Decelerate = " + String(decelerate));  
      }
      else if(!strcmp(variable, "servo")) {  //伺服馬達
        servoAngle = val;
        ledcAttachPin(pinServo, 3);
        ledcSetup(3, 50, 16);
        servo_rotate(3, servoAngle);
        delay(100);
        
        Serial.println("servo = "+String(servoAngle));
      }
      else if(!strcmp(variable, "car")) {  //自走車運動狀態
        if (val==1) {  //前進 http://192.168.xxx.xxx/control?car=1
          Serial.println("Front");     
          ledcWrite(5,speedR);
          ledcWrite(6,0);
          ledcWrite(7,speedL);
          ledcWrite(8,0);   
        }
        else if (val==2) {  //左轉 http://192.168.xxx.xxx/control?car=2
          Serial.println("Left");     
          ledcWrite(5,speedR);
          ledcWrite(6,0);
          ledcWrite(7,0);
          ledcWrite(8,speedL);  
        }
        else if (val==3) {  //停止 http://192.168.xxx.xxx/control?car=3
          Serial.println("Stop");      
          ledcWrite(5,0);
          ledcWrite(6,0);
          ledcWrite(7,0);
          ledcWrite(8,0);    
        }
        else if (val==4) {  //右轉 http://192.168.xxx.xxx/control?car=4
          Serial.println("Right");
          ledcWrite(5,0);
          ledcWrite(6,speedR);
          ledcWrite(7,speedL);
          ledcWrite(8,0);          
        }
        else if (val==5) {  //後退 http://192.168.xxx.xxx/control?car=5
          Serial.println("Back");      
          ledcWrite(5,0);
          ledcWrite(6,speedR);
          ledcWrite(7,0);
          ledcWrite(8,speedL);
        }  
        else if (val==6) {  //左前進 http://192.168.xxx.xxx/control?car=6
          Serial.println("FrontLeft");     
          ledcWrite(5,speedR);
          ledcWrite(6,0);
          ledcWrite(7,speedL*decelerate);
          ledcWrite(8,0);   
        }
        else if (val==7) {  //右前進 http://192.168.xxx.xxx/control?car=7
          Serial.println("FrontRight");     
          ledcWrite(5,speedR*decelerate);
          ledcWrite(6,0);
          ledcWrite(7,speedL);
          ledcWrite(8,0);   
        }  
        else if (val==8) {  //左後退 http://192.168.xxx.xxx/control?car=8
          Serial.println("LeftAfter");      
          ledcWrite(5,0);
          ledcWrite(6,speedR);
          ledcWrite(7,0);
          ledcWrite(8,speedL*decelerate);
        } 
        else if (val==9) {  //右後退 http://192.168.xxx.xxx/control?car=9
          Serial.println("RightAfter");      
          ledcWrite(5,0);
          ledcWrite(6,speedR*decelerate);
          ledcWrite(7,0);
          ledcWrite(8,speedL);
        }    
      }                        
      else {
          res = -1;
      }
  
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

//顯示視訊參數狀態(須回傳json格式載入初始設定)
static esp_err_t status_handler(httpd_req_t *req){
    static char json_response[1024];

    sensor_t * s = esp_camera_sensor_get();
    char * p = json_response;
    *p++ = '{';
    p+=sprintf(p, "\"flash\":%d,", 0);
    p+=sprintf(p, "\"speedL\":%d,", speedL);
    p+=sprintf(p, "\"speedR\":%d,", speedR);
    p+=sprintf(p, "\"decelerate\":%.1f,", decelerate);
    p+=sprintf(p, "\"servo\":%d,", servoAngle);        
    p+=sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p+=sprintf(p, "\"quality\":%u,", s->status.quality);
    p+=sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p+=sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p+=sprintf(p, "\"hmirror\":%u,", s->status.hmirror); 
    p+=sprintf(p, "\"vflip\":%u", s->status.vflip);
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

//自訂網頁首頁
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <title>ESP32 OV2460</title>
        <style>
            body {
                font-family: Arial,Helvetica,sans-serif;
                background: #181818;
                color: #EFEFEF;
                font-size: 16px
            }
            h2 {
                font-size: 18px
            }
            section.main {
                display: flex
            }
            #menu,section.main {
                flex-direction: column
            }
            #menu {
                display: none;
                flex-wrap: nowrap;
                min-width: 340px;
                background: #363636;
                padding: 8px;
                border-radius: 4px;
                margin-top: -10px;
                margin-right: 10px;
            }
            #content {
                display: flex;
                flex-wrap: wrap;
                align-items: stretch
            }
            figure {
                padding: 0px;
                margin: 0;
                -webkit-margin-before: 0;
                margin-block-start: 0;
                -webkit-margin-after: 0;
                margin-block-end: 0;
                -webkit-margin-start: 0;
                margin-inline-start: 0;
                -webkit-margin-end: 0;
                margin-inline-end: 0
            }
            figure img {
                display: block;
                width: 100%;
                height: auto;
                border-radius: 4px;
                margin-top: 8px;
            }
            @media (min-width: 800px) and (orientation:landscape) {
                #content {
                    display:flex;
                    flex-wrap: nowrap;
                    align-items: stretch
                }
                figure img {
                    display: block;
                    max-width: 100%;
                    max-height: calc(100vh - 40px);
                    width: auto;
                    height: auto
                }
                figure {
                    padding: 0 0 0 0px;
                    margin: 0;
                    -webkit-margin-before: 0;
                    margin-block-start: 0;
                    -webkit-margin-after: 0;
                    margin-block-end: 0;
                    -webkit-margin-start: 0;
                    margin-inline-start: 0;
                    -webkit-margin-end: 0;
                    margin-inline-end: 0
                }
            }
            section#buttons {
                display: flex;
                flex-wrap: nowrap;
                justify-content: space-between
            }
            #nav-toggle {
                cursor: pointer;
                display: block
            }
            #nav-toggle-cb {
                outline: 0;
                opacity: 0;
                width: 0;
                height: 0
            }
            #nav-toggle-cb:checked+#menu {
                display: flex
            }
            .input-group {
                display: flex;
                flex-wrap: nowrap;
                line-height: 22px;
                margin: 5px 0
            }
            .input-group>label {
                display: inline-block;
                padding-right: 10px;
                min-width: 47%
            }
            .input-group input,.input-group select {
                flex-grow: 1
            }
            .range-max,.range-min {
                display: inline-block;
                padding: 0 5px
            }
            button {
                display: block;
                margin: 5px;
                padding: 0 12px;
                border: 0;
                line-height: 28px;
                cursor: pointer;
                color: #fff;
                background: #ff3034;
                border-radius: 5px;
                font-size: 16px;
                outline: 0
            }
            button:hover {
                background: #ff494d
            }
            button:active {
                background: #f21c21
            }
            button.disabled {
                cursor: default;
                background: #a0a0a0
            }
            input[type=range] {
                -webkit-appearance: none;
                width: 100%;
                height: 22px;
                background: #363636;
                cursor: pointer;
                margin: 0
            }
            input[type=range]:focus {
                outline: 0
            }
            input[type=range]::-webkit-slider-runnable-track {
                width: 100%;
                height: 2px;
                cursor: pointer;
                background: #EFEFEF;
                border-radius: 0;
                border: 0 solid #EFEFEF
            }
            input[type=range]::-webkit-slider-thumb {
                border: 1px solid rgba(0,0,30,0);
                height: 22px;
                width: 22px;
                border-radius: 50px;
                background: #ff3034;
                cursor: pointer;
                -webkit-appearance: none;
                margin-top: -11.5px
            }
            input[type=range]:focus::-webkit-slider-runnable-track {
                background: #EFEFEF
            }
            input[type=range]::-moz-range-track {
                width: 100%;
                height: 2px;
                cursor: pointer;
                background: #EFEFEF;
                border-radius: 0;
                border: 0 solid #EFEFEF
            }
            input[type=range]::-moz-range-thumb {
                border: 1px solid rgba(0,0,30,0);
                height: 22px;
                width: 22px;
                border-radius: 50px;
                background: #ff3034;
                cursor: pointer
            }
            input[type=range]::-ms-track {
                width: 100%;
                height: 2px;
                cursor: pointer;
                background: 0 0;
                border-color: transparent;
                color: transparent
            }
            input[type=range]::-ms-fill-lower {
                background: #EFEFEF;
                border: 0 solid #EFEFEF;
                border-radius: 0
            }
            input[type=range]::-ms-fill-upper {
                background: #EFEFEF;
                border: 0 solid #EFEFEF;
                border-radius: 0
            }
            input[type=range]::-ms-thumb {
                border: 1px solid rgba(0,0,30,0);
                height: 22px;
                width: 22px;
                border-radius: 50px;
                background: #ff3034;
                cursor: pointer;
                height: 2px
            }
            input[type=range]:focus::-ms-fill-lower {
                background: #EFEFEF
            }
            input[type=range]:focus::-ms-fill-upper {
                background: #363636
            }
            .switch {
                display: block;
                position: relative;
                line-height: 22px;
                font-size: 16px;
                height: 22px
            }
            .switch input {
                outline: 0;
                opacity: 0;
                width: 0;
                height: 0
            }
            .slider {
                width: 50px;
                height: 22px;
                border-radius: 22px;
                cursor: pointer;
                background-color: grey
            }
            .slider,.slider:before {
                display: inline-block;
                transition: .4s
            }
            .slider:before {
                position: relative;
                content: "";
                border-radius: 50%;
                height: 16px;
                width: 16px;
                left: 4px;
                top: 3px;
                background-color: #fff
            }
            input:checked+.slider {
                background-color: #ff3034
            }
            input:checked+.slider:before {
                -webkit-transform: translateX(26px);
                transform: translateX(26px)
            }
            select {
                border: 1px solid #363636;
                font-size: 14px;
                height: 22px;
                outline: 0;
                border-radius: 5px
            }
            .image-container {
                position: relative;
                min-width: 160px
            }
            .close {
                position: absolute;
                right: 5px;
                top: 5px;
                background: #ff3034;
                width: 16px;
                height: 16px;
                border-radius: 100px;
                color: #fff;
                text-align: center;
                line-height: 18px;
                cursor: pointer
            }
            .hidden {
                display: none
            }
        </style>   
        <script src="https:\/\/fustyles.github.io/webduino/Tracking_20190917/tracking-min.js"></script>     
    </head>
    <figure>
      <div id="stream-container" class="image-container hidden">
        <div class="close" id="close-stream">×</div>
        <img id="stream" src="" crossorigin="anonymous">
        <table>
          <tr>
          <td align="left"><input type="range" id="leftpoint" min="0" max="1" value="0.8" step="0.05" required></td>
          <td align="right"><input type="range" id="rightpoint" min="0" max="1" value="0.2" step="0.05" required></td>
          </tr>
          <tr>
          <td colspan="2"><canvas id="canvas" width="0" height="0"></canvas><canvas id="canvas_pixel" style="display:none;"></canvas></td>
          </tr>
        </table>
      </div>
    </figure>
      <section id="buttons">
        <table>
          <tr><td colspan="3">IP: <input type="text" id="ip" value=""><input type="button" id="setip" value="Set IP" onclick="start();"></td></tr>
          <tr>
          <td align="left"><button id="restartButton">Restart</button></td>
          <td align="center"><button id="get-still">get-still</button></td>
          <td align="right"><button id="toggle-stream">Start Stream</button></td>
          </tr>
        </table>                  
      </section>    
        <section class="main">
            <section id="buttons">
                <table id="buttonPanel" style="display:none">
                  <tr><td colspan="3"><input type="checkbox" id="nostop" onclick="noStop();">No Stop</td></tr> 
                  <tr bgcolor="#363636">
                  <td align="center"><button onmousedown="stopDetection();car('/control?var=car&val=6');" onmouseup="noStop();" ontouchstart="event.preventDefault();car('/control?var=car&val=6');" ontouchend="noStop();">FrontLeft</button></td>
                  <td align="center"><button onmousedown="stopDetection();car('/control?var=car&val=1');" onmouseup="noStop();" ontouchstart="event.preventDefault();car('/control?var=car&val=1');" ontouchend="noStop();">Front</button></td>
                  <td align="center"><button onmousedown="stopDetection();car('/control?var=car&val=7');" onmouseup="noStop();" ontouchstart="event.preventDefault();car('/control?var=car&val=7');" ontouchend="noStop();">FrontRight</button></td>
                  </tr>
                  <tr bgcolor="#363636">
                  <td align="center"><button onmousedown="stopDetection();car('/control?var=car&val=2');" onmouseup="noStop();" ontouchstart="event.preventDefault();car('/control?var=car&val=2');" ontouchend="noStop();">Left</button></td>
                  <td align="center"><button onclick="stopDetection();car('/control?var=car&val=3');">Stop</button></td>
                  <td align="center"><button onmousedown="stopDetection();car('/control?var=car&val=4');" onmouseup="noStop();" ontouchstart="event.preventDefault();car('/control?var=car&val=4');" ontouchend="noStop();">Right</button></td>
                  </tr>
                  <tr bgcolor="#363636"><td align="center"><button onmousedown="stopDetection();car('/control?var=car&val=8');" onmouseup="noStop();" ontouchstart="event.preventDefault();car('/control?var=car&val=8');" ontouchend="noStop();">LeftAfter</button></td>
                  <td align="center"><button onmousedown="stopDetection();car('/control?var=car&val=5');" onmouseup="noStop();" ontouchstart="event.preventDefault();car('/control?var=car&val=5');" ontouchend="noStop();">Back</button></td>
                  <td align="center"><button onmousedown="stopDetection();car('/control?var=car&val=9');" onmouseup="noStop();" ontouchstart="event.preventDefault();car('/control?var=car&val=9');" ontouchend="noStop();">RightAfter</button></td>
                  </tr>            
                </table>
            </section>         
            <div id="logo">
                <label for="nav-toggle-cb" id="nav-toggle">&#9776;&nbsp;&nbsp;Toggle settings</label>
            </div>
            <div id="content">
                <div id="sidebar">
                    <input type="checkbox" id="nav-toggle-cb">
                    <nav id="menu">
                        <div class="input-group" id="rectheight-group">
                            <label for="rectheight">Rect Height</label>
                            <div class="range-min">20</div>
                            <input type="range" id="rectheight" min="20" max="400" step="10" value="30" class="my-action">
                            <div class="range-max">320</div>
                        </div>
                        <div class="input-group" id="Rmin-group">
                            <label for="Rmin">R min</label>
                            <div class="range-min">0</div>
                            <input type="range" id="Rmin" min="0" max="255" value="0" step="1" class="my-action">
                            <div class="range-max">255</div>
                        </div>
                        <div class="input-group" id="Rmax-group">
                            <label for="Rmax">R max</label>
                            <div class="range-min">0</div>
                            <input type="range" id="Rmax" min="0" max="255" value="0" step="1" class="my-action">
                            <div class="range-max">255</div>
                        </div>
                        <div class="input-group" id="Gmin-group">
                            <label for="Gmin">G min</label>
                            <div class="range-min">0</div>
                            <input type="range" id="Gmin" min="0" max="255" value="0" step="1" class="my-action">
                            <div class="range-max">255</div>
                        </div>
                        <div class="input-group" id="Gmax-group">
                            <label for="Gmax">G max</label>
                            <div class="range-min">0</div>
                            <input type="range" id="Gmax" min="0" max="255" value="0" step="1" class="my-action">
                            <div class="range-max">255</div>
                        </div>            
                        <div class="input-group" id="Bmin-group">
                            <label for="Bmin">B min</label>
                            <div class="range-min">0</div>
                            <input type="range" id="Bmin" min="0" max="255" value="0" step="1" class="my-action">
                            <div class="range-max">255</div>
                        </div>            
                        <div class="input-group" id="Bmax-group">
                            <label for="Bmax">B max</label>
                            <div class="range-min">0</div>
                            <input type="range" id="Bmax" min="0" max="255" value="0" step="1" class="my-action">
                            <div class="range-max">255</div>
                        </div>
                        <div class="input-group" id="pixel-group">
                            <label for="pixel">Pixel Map</label>
                            <div class="switch">
                                <input id="pixel" type="checkbox">
                                <label class="slider" for="pixel"></label>
                            </div>
                        </div>  
                        <div class="input-group" id="detectState-group">
                            <label for="detectState">Start Detect</label>
                            <div class="switch">
                                <input id="detectState" type="checkbox">
                                <label class="slider" for="detectState"></label>
                            </div>
                        </div>
                        <div class="input-group" id="motorState-group">
                            <label for="motorState">Control Motor</label>
                            <div class="switch">
                                <input id="motorState" type="checkbox">
                                <label class="slider" for="motorState"></label>
                            </div>
                        </div>            
                        <div class="input-group" id="speedR-group">
                            <label for="speedR">speed R</label>
                            <div class="range-min">0</div>
                            <input type="range" id="speedR" min="0" max="255" value="255" class="default-action">
                            <div class="range-max">255</div>
                        </div>
                        <div class="input-group" id="speedL-group">
                            <label for="speedL">speed L</label>
                            <div class="range-min">0</div>
                            <input type="range" id="speedL" min="0" max="255" value="255" class="default-action">
                            <div class="range-max">255</div>
                        </div>                        
                        <div class="input-group" id="decelerate-group">
                            <label for="decelerate">Turn Decelerate</label>
                            <div class="range-min">0</div>
                            <input type="range" id="decelerate" min="0" max="1" value="0.6" step="0.1" class="default-action">
                            <div class="range-max">1</div>
                        </div>
                        <div class="input-group" id="turnDelayMax-group">
                            <label for="turnDelayMax">Turn Delay Max</label>
                            <div class="range-min">10</div>
                            <input type="range" id="turnDelayMax" min="10" max="1000" value="60" step="10" class="my-action">
                            <div class="range-max">1000</div>
                        </div>
                        <div class="input-group" id="turnDelayMin-group">
                            <label for="turnDelayMin">Turn Delay Min</label>
                            <div class="range-min">10</div>
                            <input type="range" id="turnDelayMin" min="10" max="1000" value="30" step="10" class="my-action">
                            <div class="range-max">1000</div>
                        </div>
                        <div class="input-group" id="forwardDelay-group">
                            <label for="forwardDelay">Forward Delay</label>
                            <div class="range-min">10</div>
                            <input type="range" id="forwardDelay" min="10" max="1000" value="30" step="10" class="my-action">
                            <div class="range-max">1000</div>
                        </div>
                        <div class="input-group" id="panel-group">
                            <label for="panel">Button Panel</label>
                            <div class="switch">
                                <input id="panel" type="checkbox">
                                <label class="slider" for="panel"></label>
                            </div>
                        </div>            
                        <div class="input-group" id="servo-group">
                            <label for="servo">Servo</label>
                            <div class="range-min">0</div>
                            <input type="range" id="servo" min="0" max="180" value="90" class="default-action">
                            <div class="range-max">180</div>
                        </div>                       
                        <div class="input-group" id="flash-group">
                            <label for="flash">Flash</label>
                            <div class="range-min">0</div>
                            <input type="range" id="flash" min="0" max="255" value="0" class="default-action">
                            <div class="range-max">255</div>
                        </div>
                        <div class="input-group" id="framesize-group">
                            <label for="framesize">Resolution</label>
                            <select id="framesize" class="default-action">
                                <option value="10">UXGA(1600x1200)</option>
                                <option value="9">SXGA(1280x1024)</option>
                                <option value="8">XGA(1024x768)</option>
                                <option value="7">SVGA(800x600)</option>
                                <option value="6">VGA(640x480)</option>
                                <option value="5">CIF(400x296)</option>
                                <option value="4" selected="selected">QVGA(320x240)</option>
                                <option value="3">HQVGA(240x176)</option>
                                <option value="0">QQVGA(160x120)</option>
                            </select>
                        </div>
                        <div class="input-group" id="quality-group">
                            <label for="quality">Quality</label>
                            <div class="range-min">10</div>
                            <input type="range" id="quality" min="10" max="63" value="10" class="default-action">
                            <div class="range-max">63</div>
                        </div>
                        <div class="input-group" id="brightness-group">
                            <label for="brightness">Brightness</label>
                            <div class="range-min">-2</div>
                            <input type="range" id="brightness" min="-2" max="2" value="0" class="default-action">
                            <div class="range-max">2</div>
                        </div>
                        <div class="input-group" id="contrast-group">
                            <label for="contrast">Contrast</label>
                            <div class="range-min">-2</div>
                            <input type="range" id="contrast" min="-2" max="2" value="0" class="default-action">
                            <div class="range-max">2</div>
                        </div>
                        <div class="input-group" id="hmirror-group">
                            <label for="hmirror">H-Mirror</label>
                            <div class="switch">
                                <input id="hmirror" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="hmirror"></label>
                            </div>
                        </div>
                        <div class="input-group" id="vflip-group">
                            <label for="vflip">V-Flip</label>
                            <div class="switch">
                                <input id="vflip" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="vflip"></label>
                            </div>
                        </div>
                    </nav>
                </div>
            </div>
        </section>
        <div id="result" style="color:yellow"></div> 
      </body>
  </html>
  
        <script> 
          //  網址/?192.168.1.38  可自動帶入?後參數IP值
          var href=location.href;
          if (href.indexOf("?")!=-1) {
            document.getElementById("ip").value = location.search.split("?")[1].replace(/http:\/\//g,"");
          }
          else if (href.indexOf("http")!=-1) {
            document.getElementById("ip").value = location.host;
          } 

          function start() {
            window.stop();
            
            var baseHost = 'http://'+document.getElementById("ip").value;  //var baseHost = document.location.origin
            var streamUrl = baseHost + ':81'
          
            const hide = el => {
              el.classList.add('hidden')
            }
            const show = el => {
              el.classList.remove('hidden')
            }
          
            const disable = el => {
              el.classList.add('disabled')
              el.disabled = true
            }
          
            const enable = el => {
              el.classList.remove('disabled')
              el.disabled = false
            }
          
            const updateValue = (el, value, updateRemote) => {
              updateRemote = updateRemote == null ? true : updateRemote
              let initialValue
              if (el.type === 'checkbox') {
                initialValue = el.checked
                value = !!value
                el.checked = value
              } else {
                initialValue = el.value
                el.value = value
              }
              el.title = value;
          
              if (updateRemote && initialValue !== value) {
                updateConfig(el);
              }
            }
          
            function updateConfig (el) {
              let value
              switch (el.type) {
                case 'checkbox':
                  value = el.checked ? 1 : 0
                  break
                case 'range':
                case 'select-one':
                  value = el.value
                  break
                case 'button':
                case 'submit':
                  value = '1'
                  break
                default:
                  return
              }
          
              const query = `${baseHost}/control?var=${el.id}&val=${value}`
              el.title = value;
        
              fetch(query)
                .then(response => {
                  console.log(`request to ${query} finished, status: ${response.status}`)
                })
            }
          
            document
              .querySelectorAll('.close')
              .forEach(el => {
                el.onclick = () => {
                  hide(el.parentNode)
                }
              })
          
            // read initial values
            fetch(`${baseHost}/status`)
              .then(function (response) {
                return response.json()
              })
              .then(function (state) {
                document
                  .querySelectorAll('.default-action')
                  .forEach(el => {
                      updateValue(el, state[el.id], false)
                  })
                  result.innerHTML = "Connection successful";
              })
          
            const view = document.getElementById('stream')
            const viewContainer = document.getElementById('stream-container')
            const stillButton = document.getElementById('get-still')
            const streamButton = document.getElementById('toggle-stream')
            const closeButton = document.getElementById('close-stream')
            const restartButton = document.getElementById('restartButton')
          
            const stopStream = () => {
              //window.stop();
              streamButton.innerHTML = 'Start Stream';
              hide(viewContainer)
            }
          
            const startStream = () => {
              view.src = `${streamUrl}/stream`
              show(viewContainer)
              streamButton.innerHTML = 'Stop Stream'
            }

            //新增重啟電源按鈕點選事件 (自訂指令格式：http://192.168.xxx.xxx/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9)
            restartButton.onclick = () => {
              fetch(baseHost+"/control?restart");
            }            
          
            // Attach actions to buttons
            stillButton.onclick = () => {
              stopStream()
              view.src = `${baseHost}/capture?_cb=${Date.now()}`
              show(viewContainer)
            }
          
            closeButton.onclick = () => {
              stopStream()
              hide(viewContainer)
            }
          
            streamButton.onclick = () => {
              const streamEnabled = streamButton.innerHTML === 'Stop Stream'
              if (streamEnabled) {
                stopStream()
              } else {
                startStream()
              }
            }
          
            // Attach default on change action
            document
              .querySelectorAll('.default-action')
              .forEach(el => {
                el.onchange = () => updateConfig(el)
              })
        
            // 自訂類別my-action, title屬性顯示數值
            document
              .querySelectorAll('.my-action')
              .forEach(el => {
                el.title = el.value;
                el.onchange = () => el.title = el.value;
              })        
          
            // Custom actions
          
            const framesize = document.getElementById('framesize')
          
            framesize.onchange = () => {
              updateConfig(framesize)
            }                                 
          }


          //法蘭斯影像辨識
          const aiView = document.getElementById('stream');
          const aiStill = document.getElementById('get-still')
          const canvas = document.getElementById('canvas');     
          var context = canvas.getContext("2d");
          const canvas_pixel = document.getElementById('canvas_pixel');
          var context_pixel = canvas_pixel.getContext('2d');      
          const nostop = document.getElementById('nostop');
          const detectState = document.getElementById('detectState');
          const autodetect = document.getElementById('autodetect');
          const motorState = document.getElementById('motorState');
          const servo = document.getElementById('servo');
          const panel = document.getElementById('panel');
          const result = document.getElementById('result');
          const ip = document.getElementById('ip');
      
          const pixelState = document.getElementById('pixel');
          const rectheight = document.getElementById('rectheight');
          const Rmin = document.getElementById('Rmin');
          const Rmax = document.getElementById('Rmax');
          const Gmin = document.getElementById('Gmin');
          const Gmax = document.getElementById('Gmax');
          const Bmin = document.getElementById('Bmin');
          const Bmax = document.getElementById('Bmax');
          const turnDelayMax = document.getElementById('turnDelayMax');  //遠離時迴轉時間
          const turnDelayMin = document.getElementById('turnDelayMin');  //偏離時迴轉時間
          const forwardDelay = document.getElementById('forwardDelay');  //前進時持續時間
          var servoAngle = 30;  //伺服馬達預設位置
          var lastDirection = "";  //記錄前一動作行進方向
      
          const tracker = new tracking.ColorTracker();

          panel.onchange = function(e){  
            if (!panel.checked)
              buttonPanel.style.display = "none";
            else
              buttonPanel.style.display = "block";
          }                       
          
          function car(query) {
             query = "http:\/\/" + ip.value + query;
             fetch(query)
                .then(response => {
                  console.log(`request to ${query} finished, status: ${response.status}`)
                })
          }
                
          function noStop() {
            if (!nostop.checked) {
              car('/control?var=car&val=3');
            }
          }

          detectState.onclick = function() {
            if (detectState.checked == true) {
              aiView.style.display = "none";
              canvas.style.display = "block";
              aiStill.click();
            } else {
              aiView.style.display = "block";
              canvas.style.display = "none";
            }
          }
      
          pixel.onclick = function() {
            if (pixelState.checked == true) {
              canvas_pixel.style.display = "block";
            } else {
              canvas_pixel.style.display = "none";
            }
          }     
            
          function stopDetection() {
            detectState.checked = false;
            aiView.style.display = "block";
            canvas.style.display = "none";  
          }

          aiView.onload = function (event) {
            if (detectState.checked == false) return;
            
            canvas.setAttribute("width", aiView.width);
            canvas.setAttribute("height", aiView.height);
            canvas_pixel.setAttribute("width", aiView.width);
            canvas_pixel.setAttribute("height", aiView.height);  
            context.drawImage(aiView,0,0,aiView.width,aiView.height);
               
            var imgData=context.getImageData(0,0,canvas.width,canvas.height);
            var y1 = aiView.height/2-Number(rectheight.value)/2;
            var y2 = aiView.height/2+Number(rectheight.value)/2;

            for (var i=0;i<imgData.data.length;i+=4) {
              var r=0;
              var g=0;
              var b=0;
              if ((imgData.data[i]>=Rmin.value&&imgData.data[i]<=Rmax.value)&&(imgData.data[i+1]>=Gmin.value&&imgData.data[i+1]<=Gmax.value)&&(imgData.data[i+2]>=Bmin.value&&imgData.data[i+2]<=Bmax.value)) {
                if (aiView.width*4*y1<=i&&aiView.width*4*y2>=i)
                r=255;
                else
                r=0;
              }
      
              imgData.data[i]=r;
              imgData.data[i+1]=g;
              imgData.data[i+2]=b;
              imgData.data[i+3]=255;
            }
            context_pixel.putImageData(imgData,0,0);
      
            tracking.track('#canvas_pixel', tracker);
        
            context.strokeStyle = "cyan";
            context.fillStyle = "cyan";
            context.lineWidth = 5;      
            context.beginPath();
            context.arc(leftpoint.value*aiView.width/2,aiView.height/2,3, 6.284, false, false, '#ff0000', 0, 1);
            context.fill();
            context.beginPath();
            context.arc(aiView.width/2+rightpoint.value*aiView.width/2,aiView.height/2,3, 6.284, false, false, '#ff0000', 0, 1);
            context.fill();
          }  
    
        tracking.ColorTracker.registerColor('red', function(r, g, b) {
          if ((r==255)&&(g==0)&&(b==0)) {
          return true;
          }     
          return false;
        });
       
        var trackedColors = {
          custom: true
        };
        
        Object.keys(tracking.ColorTracker.knownColors_).forEach(function(color) {
          trackedColors[color] = true;
        });
        
        var colors = [];
        for (var color in trackedColors) {
          if (trackedColors[color]) {
          colors.push(color);
          }
        }
        tracker.setColors(colors);
        
        tracker.on('track', function(event) {
        
          event.data.forEach(function(rect) {
          context.strokeStyle = rect.color;
          context.strokeRect(rect.x, rect.y, rect.width, rect.height);
          //context.font = '11px Helvetica';
          //context.fillStyle = "#fff";
          //context.fillText('x: ' + rect.x + 'px', rect.x + rect.width + 5, rect.y + 11);
          //context.fillText('y: ' + rect.y + 'px', rect.x + rect.width + 5, rect.y + 22);
    
          //result.innerHTML+= rect.color+","+rect.x+","+rect.y+","+rect.width+","+rect.height+"<br>";
          // 1-Front, 2-Left, 3-Stop, 4-Right, 5-Back, 6-FrontLeft, 7-FrontRight, 8-LeftAfter, 9-RightAfter
          
          if (rect.color=="red"&&motorState.checked) {
            var xl = leftpoint.value*aiView.width/2;
            var yl = aiView.height/2;
            var xr = aiView.width/2+rightpoint.value*aiView.width/2;
            var yr = aiView.height/2;
            var xls = -1;
            var xrs = -1;
            if (xl<rect.x)
              var xls = 0;
            else if (xl>=rect.x&&xl<=(rect.x+rect.width))
              var xls = 1;
            else if (xl>(rect.x+rect.width))
              var xls = 2;
              
            if (xr<rect.x)
              var xrs = 0;
            else if (xr>=rect.x&&xr<=(rect.x+rect.width))
              var xrs = 1;
            else if (xr>(rect.x+rect.width))
              var xrs = 2;
            
            var position = "";
            if (xls==0&&xrs==0) {
              result.innerHTML= "偏左遠離";
              position = "leftfar";
            } else if (xls==0&&xrs==2) {
              result.innerHTML= "正常";
              position = "normal";
            } else if (xls==0&&xrs==1) {
              result.innerHTML= "偏左";
              position = "left";
            } else if (xls==1&&xrs==1) {
              result.innerHTML= "正常";
              position = "normal";
            } else if (xls==1&&xrs==2) {
              result.innerHTML= "偏右";
              position = "right";
            } else if (xls==2&&xrs==2) {
              result.innerHTML= "偏右遠離";
              position = "rightfar";
            } else {
              result.innerHTML= "無法判別";
              position = "";
            }
              
            context.font = '30px Helvetica';
            context.fillStyle = "#fff";
            context.fillText(result.innerHTML, 0, 30);        
          
            var delay=0;
            if (position.indexOf("right")!=-1) {
              if (position=="right") {  //物件中心點偏左
                delay = turnDelayMin.value;
              } else {  //物件中心點偏左遠離
                delay = turnDelayMax.value;
              } 
              if (!hmirror.checked) {  //鏡像左右位置相反
                car('/control?car=6;'+delay);  //左前進
                lastDirection = "left";
              } else {                
                car('/control?car=7;'+delay);  //右前進
                lastDirection = "right";
              }
            } else if (position.indexOf("left")!=-1) {
              if (position=="left") { //物件中心點偏右
                delay = turnDelayMin.value;
              } else { //物件中心點偏右遠離
                delay = turnDelayMax.value;
              }
              if (!hmirror.checked) {  //鏡像左右位置相反
                car('/control?car=7;'+delay);  //右前進
                lastDirection = "right";
              }
              else {                           
                car('/control?car=6;'+delay);  //左前進
                lastDirection = "left";
              }
            } else if (position=="normal") {  //位於線上位置則前進
              car('/control?car=1;' + forwardDelay.value);
            }
            return;
          }
        });
    
        if (event.data.length==0&&motorState.checked) {  //若畫面中完全偵測不到線，則依前一動作反向轉動。
          if (lastDirection = "right")
            car('/control?car=6;' + turnDelayMin.value);
          else if (lastDirection = "left")
            car('/control?car=7;' + turnDelayMin.value);
        }   
    
        try { 
          document.createEvent("TouchEvent");
          setTimeout(function(){aiStill.click();},250);
        }
        catch(e) { 
          setTimeout(function(){aiStill.click();},150);
        }     
      });            
  </script>
)rawliteral";

//網頁首頁   http://192.168.xxx.xxx
static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

//自訂網址路徑要執行的函式
void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();  //可在HTTPD_DEFAULT_CONFIG()中設定Server Port 

  //http://192.168.xxx.xxx/
  httpd_uri_t index_uri = {
      .uri       = "/",
      .method    = HTTP_GET,
      .handler   = index_handler,
      .user_ctx  = NULL
  };

  //http://192.168.xxx.xxx/status
  httpd_uri_t status_uri = {
      .uri       = "/status",
      .method    = HTTP_GET,
      .handler   = status_handler,
      .user_ctx  = NULL
  };

  //http://192.168.xxx.xxx/control
  httpd_uri_t cmd_uri = {
      .uri       = "/control",
      .method    = HTTP_GET,
      .handler   = cmd_handler,
      .user_ctx  = NULL
  }; 

  //http://192.168.xxx.xxx/capture
  httpd_uri_t capture_uri = {
      .uri       = "/capture",
      .method    = HTTP_GET,
      .handler   = capture_handler,
      .user_ctx  = NULL
  };

  //http://192.168.xxx.xxx:81/stream
  httpd_uri_t stream_uri = {
      .uri       = "/stream",
      .method    = HTTP_GET,
      .handler   = stream_handler,
      .user_ctx  = NULL
  };
  
  Serial.printf("Starting web server on port: '%d'\n", config.server_port);  //Server Port
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
      //註冊自訂網址路徑對應執行的函式
      httpd_register_uri_handler(camera_httpd, &index_uri);
      httpd_register_uri_handler(camera_httpd, &cmd_uri);
      httpd_register_uri_handler(camera_httpd, &status_uri);
      httpd_register_uri_handler(camera_httpd, &capture_uri);
  }
  
  config.server_port += 1;  //Stream Port
  config.ctrl_port += 1;    //UDP Port
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
