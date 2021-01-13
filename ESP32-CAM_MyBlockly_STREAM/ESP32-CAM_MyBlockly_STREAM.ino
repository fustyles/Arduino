/*
ESP32-CAM 模組 (暫時無法跨網域連線與使用HTTPS)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-1-18 20:00
https://www.facebook.com/francefu

Arduino IDE settings
Partition Scheme : Huge APP (3MB No OTA/1MB SPIFFS)

預設AP端IP： 192.168.4.1 
http://APIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

預設AP端IP： 192.168.4.1
http://192.168.xxx.xxx/control?ip
http://192.168.xxx.xxx/control?mac
http://192.168.xxx.xxx/control?restart
http://192.168.xxx.xxx/control?inputpullup=pin
http://192.168.xxx.xxx/control?pinmode=pin;value
http://192.168.xxx.xxx/control?digitalwrite=pin;value
http://192.168.xxx.xxx/control?analogwrite=pin;value
http://192.168.xxx.xxx/control?digitalread=pin
http://192.168.xxx.xxx/control?analogread=pin
http://192.168.xxx.xxx/control?touchread=pin
http://192.168.xxx.xxx/control?tcp=domain;port;request;wait
--> wait = 0 or 1  (waiting for response)
--> request = /xxxx/xxxx
http://192.168.xxx.xxx/control?ifttt=event;key;value1;value2;value3
http://192.168.xxx.xxx/control?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8
http://192.168.xxx.xxx/control?thingspeakread=request
--> request = /channels/xxxxx/fields/1.jsoncontrol?results=1
http://192.168.xxx.xxx/control?linenotify=token;request
--> request = message=xxxxx
http://192.168.xxx.xxx/control?flash=value          //vale= 0~255
http://192.168.xxx.xxx/control?servo=pin;value;channel  //vale= 1700~8000, channel=9, 10, 11
http://192.168.xxx.xxx/control?speedL=value         //vale= 0~255
http://192.168.xxx.xxx/control?speedR=value         //vale= 0~255
http://192.168.xxx.xxx/control?decelerate=value     //vale= 0~100  (%)
http://192.168.xxx.xxx/control?car=state            //state= 1(Front),2(Left),3(Stop),4(Right),5(Back),6(FrontLeft),7(FrontRight),8(LeftAfter),9(RightAfter)
http://192.168.xxx.xxx/control?framesize=size       //size= UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

查詢Client端IP：
查詢IP：http://192.168.4.1/?ip
重設網路：http://192.168.4.1/?resetwifi=ssid;password

http://192.168.xxx.xxx             //網頁首頁管理介面
http://192.168.xxx.xxx:81/stream   //取得串流影像        <img src="http://192.168.xxx.xxx:81/stream">
http://192.168.xxx.xxx/capture     //取得影像            <img src="http://192.168.xxx.xxx/capture">
http://192.168.xxx.xxx/status      //取得視訊參數值

設定視訊參數(官方指令格式)
http://192.168.xxx.xxx/control?var=framesize&val=value    // value = 10->UXGA(1600x1200), 9->SXGA(1280x1024), 8->XGA(1024x768) ,7->SVGA(800x600), 6->VGA(640x480), 5 selected=selected->CIF(400x296), 4->QVGA(320x240), 3->HQVGA(240x176), 0->QQVGA(160x120)
http://192.168.xxx.xxx/control?var=quality&val=value    // value = 10 ~ 63
http://192.168.xxx.xxx/control?var=brightness&val=value    // value = -2 ~ 2
http://192.168.xxx.xxx/control?var=contrast&val=value    // value = -2 ~ 2
http://192.168.xxx.xxx/control?var=saturation&val=value    // value = -2 ~ 2 
http://192.168.xxx.xxx/control?var=gainceiling&val=value    // value = 0 ~ 6
http://192.168.xxx.xxx/control?var=colorbar&val=value    // value = 0 or 1
http://192.168.xxx.xxx/control?var=awb&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=agc&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=aec&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=hmirror&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=vflip&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=awb_gain&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=agc_gain&val=value    // value = 0 ~ 30
http://192.168.xxx.xxx/control?var=aec_value&val=value    // value = 0 ~ 1200
http://192.168.xxx.xxx/control?var=aec2&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=dcw&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=bpc&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=wpc&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=raw_gma&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=lenc&val=value    // value = 0 or 1 
http://192.168.xxx.xxx/control?var=special_effect&val=value    // value = 0 ~ 6
http://192.168.xxx.xxx/control?var=wb_mode&val=value    // value = 0 ~ 4
http://192.168.xxx.xxx/control?var=ae_level&val=value    // value = -2 ~ 2 
http://192.168.xxx.xxx/control?var=flash&val=value    // value = 0 ~ 255  
*/

//輸入WIFI連線帳號密碼
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

//輸入AP端連線帳號密碼
const char* apssid = "ESP32-CAM";
const char* appassword = "12345678";         //AP password require at least 8 characters.

int speedR = 255;  //You can adjust the speed of the wheel. (gpio12, gpio13)
int speedL = 255;  //You can adjust the speed of the wheel. (gpio14, gpio15)
double decelerate = 60;

#include <WiFi.h>
#include <HTTPClient.h>
HTTPClient http;
#include "esp_camera.h"          //視訊
#include "Base64.h"              //用於轉換視訊影像格式為base64格式，易於上傳google雲端硬碟或資料庫
#include "soc/soc.h"             //用於電源不穩不重開機 
#include "soc/rtc_cntl_reg.h"    //用於電源不穩不重開機 

//官方函式庫
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"

#include "fb_gfx.h"
#include "fd_forward.h"
#include "fr_forward.h"

String Feedback="";   //回傳客戶端訊息

//指令參數值
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

//指令拆解狀態值
byte ReceiveState=0;
byte cmdState=1;
byte strState=1;
byte questionstate=0;
byte equalstate=0;
byte semicolonstate=0;

typedef struct {
        size_t size; //number of values used for filtering
        size_t index; //current value index
        size_t count; //value count
        int sum;
        int * values; //array to be filled with values
} ra_filter_t;

typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static ra_filter_t ra_filter;
httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

static mtmn_config_t mtmn_config = {0};

static ra_filter_t * ra_filter_init(ra_filter_t * filter, size_t sample_size){
    memset(filter, 0, sizeof(ra_filter_t));

    filter->values = (int *)malloc(sample_size * sizeof(int));
    if(!filter->values){
        return NULL;
    }
    memset(filter->values, 0, sample_size * sizeof(int));

    filter->size = sample_size;
    return filter;
}

static int ra_filter_run(ra_filter_t * filter, int value){
    if(!filter->values){
        return value;
    }
    filter->sum -= filter->values[filter->index];
    filter->values[filter->index] = value;
    filter->sum += filter->values[filter->index];
    filter->index++;
    filter->index = filter->index % filter->size;
    if (filter->count < filter->size) {
        filter->count++;
    }
    return filter->sum / filter->count;
}

// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled

//安可信ESP32-CAM模組腳位設定
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

  //視訊組態設定
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
  
  //視訊初始化
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  //可動態改變視訊框架大小(解析度大小)
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_CIF);  //UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

  //閃光燈(GPIO4)
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);
  
  //馬達驅動IC
  ledcAttachPin(12, 5);
  ledcSetup(5, 2000, 8);      
  ledcAttachPin(13, 6);
  ledcSetup(6, 2000, 8);
  ledcWrite(6, 0);  
  ledcAttachPin(15, 7);
  ledcSetup(7, 2000, 8);      
  ledcAttachPin(14, 8);
  ledcSetup(8, 2000, 8);   
  
  WiFi.mode(WIFI_AP_STA);

  //指定Client端靜態IP
  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  WiFi.begin(ssid, password);    //執行網路連線

  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+10000) < millis()) break;    //等待10秒連線
  } 

  if (WiFi.status() == WL_CONNECTED) {    //若連線成功
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);   //設定SSID顯示客戶端IP         
    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP()); 

    for (int i=0;i<5;i++) {   //若連上WIFI設定閃光燈快速閃爍
      ledcWrite(4,10);
      delay(200);
      ledcWrite(4,0);
      delay(200);    
    }    
  }
  else {
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
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    size_t out_len, out_width, out_height;
    uint8_t * out_buf;
    bool s;
    if(fb->width > 400){
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
        int64_t fr_end = esp_timer_get_time();
        Serial.printf("JPG: %uB %ums\n", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start)/1000));
        return res;
    }

    dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
    if (!image_matrix) {
        esp_camera_fb_return(fb);
        Serial.println("dl_matrix3du_alloc failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    out_buf = image_matrix->item;
    out_len = fb->width * fb->height * 3;
    out_width = fb->width;
    out_height = fb->height;

    s = fmt2rgb888(fb->buf, fb->len, fb->format, out_buf);
    esp_camera_fb_return(fb);
    if(!s){
        dl_matrix3du_free(image_matrix);
        Serial.println("to rgb888 failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    jpg_chunking_t jchunk = {req, 0};
    s = fmt2jpg_cb(out_buf, out_len, out_width, out_height, PIXFORMAT_RGB888, 90, jpg_encode_stream, &jchunk);
    dl_matrix3du_free(image_matrix);
    if(!s){
        Serial.println("JPEG compression failed");
        return ESP_FAIL;
    }

    int64_t fr_end = esp_timer_get_time();
    return res;
}

//影像串流
static esp_err_t stream_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];
    dl_matrix3du_t *image_matrix = NULL;
    int64_t fr_start = 0;
    int64_t fr_ready = 0;

    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

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
            fr_start = esp_timer_get_time();
            fr_ready = fr_start;
            if(fb->width > 400){
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
            } else {

                image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);

                if (!image_matrix) {
                    Serial.println("dl_matrix3du_alloc failed");
                    res = ESP_FAIL;
                } else {
                    if(!fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item)){
                        Serial.println("fmt2rgb888 failed");
                        res = ESP_FAIL;
                    } else {
                        fr_ready = esp_timer_get_time();
                        if (fb->format != PIXFORMAT_JPEG){
                            if(!fmt2jpg(image_matrix->item, fb->width*fb->height*3, fb->width, fb->height, PIXFORMAT_RGB888, 90, &_jpg_buf, &_jpg_buf_len)){
                                Serial.println("fmt2jpg failed");
                                res = ESP_FAIL;
                            }
                            esp_camera_fb_return(fb);
                            fb = NULL;
                        } else {
                            _jpg_buf = fb->buf;
                            _jpg_buf_len = fb->len;
                        }
                    }
                    dl_matrix3du_free(image_matrix);
                }
            }
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
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
        int64_t fr_end = esp_timer_get_time();

        int64_t ready_time = (fr_ready - fr_start)/1000;
        
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
        Serial.printf("MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps), %u+%u+%u+%u=%u %s%d\n",
            (uint32_t)(_jpg_buf_len),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
            avg_frame_time, 1000.0 / avg_frame_time
        );
    }

    last_frame = 0;
    return res;
}

//指令參數控制
static esp_err_t cmd_handler(httpd_req_t *req){
    char*  buf;  //存取網址後帶的參數字串
    size_t buf_len;
    char variable[128] = {0,};  //存取參數var值
    char value[128] = {0,};  //存取參數val值
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
            myCmd = String(buf);
          }
        }
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    Feedback="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
    ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;     
    if (myCmd.length()>0) {
      myCmd = "?"+myCmd;  //網址後帶的參數字串轉換成自訂參數格式
      for (int i=0;i<myCmd.length();i++) {
        getCommand(char(myCmd.charAt(i)));  //拆解參數字串
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
      else if (cmd=="ip") {  //查詢IP
        Feedback="AP IP: "+WiFi.softAPIP().toString();    
        Feedback+=", ";
        Feedback+="STA IP: "+WiFi.localIP().toString();
      }  
      else if (cmd=="mac") {  //查詢MAC
        Feedback="STA MAC: "+WiFi.macAddress();
      }  
      else if (cmd=="restart") {  //重設WIFI連線
        ESP.restart();
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
        Feedback=String(digitalRead(P1.toInt()));
      }
      else if (cmd=="analogwrite") {
        if (P1=="2") {
          ledcAttachPin(2, 3);  
          ledcSetup(3, 5000, 8);
          ledcWrite(3,P2.toInt());     
        }    
        else if (P1=="4") {
          ledcAttachPin(4, 4);  
          ledcSetup(4, 5000, 8);
          ledcWrite(4,P2.toInt());     
        }
        else if (P1=="12") {
          ledcAttachPin(12, 5);  
          ledcSetup(5, 5000, 8);
          ledcWrite(5,P2.toInt());     
        }
        else if (P1=="13") {
          ledcAttachPin(13, 6);  
          ledcSetup(6, 5000, 8);
          ledcWrite(6,P2.toInt());     
        }
        else if (P1=="14") {
          ledcAttachPin(14, 8);  
          ledcSetup(8, 5000, 8);
          ledcWrite(8,P2.toInt());     
        }
        else if (P1=="15") {
          ledcAttachPin(15, 7);  
          ledcSetup(7, 5000, 8);
          ledcWrite(7,P2.toInt());     
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
      else if (cmd=="tcp") {
        String domain=P1;
        int port=P2.toInt();
        String request=P3;
        int wait=P4.toInt();      // wait = 0 or 1
    
        Feedback=tcp_http(domain,request,port,wait);  
      }
      else if (cmd=="ifttt") {
        String domain="maker.ifttt.com";
        String request = "/trigger/" + P1 + "/with/key/" + P2;
        request += "?value1="+P3+"&value2="+P4+"&value3="+P5;
        Feedback=tcp_http(domain,request,80,0);
      }
      else if (cmd=="thingspeakupdate") {
        String domain="api.thingspeak.com";
        String request = "/update?api_key=" + P1;
        request += "&field1="+P2+"&field2="+P3+"&field3="+P4+"&field4="+P5+"&field5="+P6+"&field6="+P7+"&field7="+P8+"&field8="+P9;
        Feedback=tcp_http(domain,request,80,0);
      }    
      else if (cmd=="thingspeakread") {
        String domain="api.thingspeak.com";
        String request = P1;
        Feedback=tcp_http(domain,request,80,1);
        int s=Feedback.indexOf("feeds");
        Feedback=Feedback.substring(s+8);
        int e=Feedback.indexOf("]");
        Feedback=Feedback.substring(0,e);
        Feedback.replace("},{",";");
        Feedback.replace("\":\"",",");
        Feedback.replace("\":",",");
        Feedback.replace("\",\"",","); 
        Feedback.replace("\"","");
        Feedback.replace("{","");
        Feedback.replace("}","");
        Feedback.replace("[","");
        Feedback.replace("]","");
        Feedback.replace(",\"",",");
        Feedback.replace("\":",",");
      } 
      else if (cmd=="linenotify") {
        String token = P1;
        String request = P2;
        Feedback=LineNotify(token,request,1);
        //Serial.println(Feedback);
        if (Feedback.indexOf("status")!=-1) {
          int s=Feedback.indexOf("<body>");
          Feedback=Feedback.substring(s+6);
          int e=Feedback.indexOf("<");
          Feedback=Feedback.substring(0,e);
        }
      } 
      else if (cmd=="flash") {  //控制內建閃光燈
        ledcAttachPin(4, 4);  
        ledcSetup(4, 5000, 8);   
         
        int val = P1.toInt();
        ledcWrite(4,val);  
      }
      else if (cmd=="servo") {
        int pin = P1.toInt();
        int channel = P3.toInt();
        ledcAttachPin(pin, channel);  
        ledcSetup(channel, 50, 16); 
        
        int val = P2.toInt();     
        if (val > 8000)
           val = 8000;
        else if (val < 1700)
          val = 1700;   
        val = 1700 + (8000 - val);   
        ledcWrite(channel, val); 
      }  
      else if (cmd=="speedL") {
        int val = P1.toInt();
        if (val > 255)
           val = 255;
        else if (val < 0)
          val = 0;       
        speedL = val;
        Serial.println("LeftSpeed = " + String(val)); 
      }  
      else if (cmd=="speedR") {
        int val = P1.toInt();
        if (val > 255)
           val = 255;
        else if (val < 0)
          val = 0;       
        speedR = val;
        Serial.println("RightSpeed = " + String(val)); 
      }  
      else if (cmd=="decelerate") { 
        int val = P1.toInt();      
        decelerate = val;
        Serial.println("Decelerate = " + String(val)); 
      }   
      else if (cmd=="car") {
        ledcAttachPin(12, 5);
        ledcSetup(5, 2000, 8);      
        ledcAttachPin(13, 6);
        ledcSetup(6, 2000, 8);
        ledcWrite(6, 0);  
        ledcAttachPin(15, 7);
        ledcSetup(7, 2000, 8);      
        ledcAttachPin(14, 8);
        ledcSetup(8, 2000, 8); 
          
        int val = P1.toInt();
        if (val==1) {
          Serial.println("Front");     
          ledcWrite(5,speedR);
          ledcWrite(6,0);
          ledcWrite(7,speedL);
          ledcWrite(8,0);   
        }
        else if (val==2) {
          Serial.println("Left");     
          ledcWrite(5,speedR*decelerate/100);
          ledcWrite(6,0);
          ledcWrite(7,0);
          ledcWrite(8,speedL*decelerate/100);  
        }
        else if (val==3) {
          Serial.println("Stop");      
          ledcWrite(5,0);
          ledcWrite(6,0);
          ledcWrite(7,0);
          ledcWrite(8,0);    
        }
        else if (val==4) {
          Serial.println("Right");
          ledcWrite(5,0);
          ledcWrite(6,speedR*decelerate/100);
          ledcWrite(7,speedL*decelerate/100);
          ledcWrite(8,0);          
        }
        else if (val==5) {
          Serial.println("Back");      
          ledcWrite(5,0);
          ledcWrite(6,speedR);
          ledcWrite(7,0);
          ledcWrite(8,speedL);
        }  
        else if (val==6) {
          Serial.println("FrontLeft");     
          ledcWrite(5,speedR);
          ledcWrite(6,0);
          ledcWrite(7,speedL*decelerate/100);
          ledcWrite(8,0);   
        }
        else if (val==7) {
          Serial.println("FrontRight");     
          ledcWrite(5,speedR*decelerate/100);
          ledcWrite(6,0);
          ledcWrite(7,speedL);
          ledcWrite(8,0);   
        }  
        else if (val==8) {
          Serial.println("LeftAfter");      
          ledcWrite(5,0);
          ledcWrite(6,speedR);
          ledcWrite(7,0);
          ledcWrite(8,speedL*decelerate/100);
        } 
        else if (val==9) {
          Serial.println("RightAfter");      
          ledcWrite(5,0);
          ledcWrite(6,speedR*decelerate/100);
          ledcWrite(7,0);
          ledcWrite(8,speedL);
        }       
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
          s->set_framesize(s, FRAMESIZE_CIF);    
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
      int val = atoi(value);
      sensor_t * s = esp_camera_sensor_get();
      int res = 0;

      //官方指令區塊，也可在此自訂指令  http://192.168.xxx.xxx/control?var=xxx&val=xxx
      if(!strcmp(variable, "framesize")) {
          if(s->pixformat == PIXFORMAT_JPEG) res = s->set_framesize(s, (framesize_t)val);
      }
      else if(!strcmp(variable, "quality")) res = s->set_quality(s, val);
      else if(!strcmp(variable, "contrast")) res = s->set_contrast(s, val);
      else if(!strcmp(variable, "brightness")) res = s->set_brightness(s, val);
      else if(!strcmp(variable, "saturation")) res = s->set_saturation(s, val);
      else if(!strcmp(variable, "gainceiling")) res = s->set_gainceiling(s, (gainceiling_t)val);
      else if(!strcmp(variable, "colorbar")) res = s->set_colorbar(s, val);
      else if(!strcmp(variable, "awb")) res = s->set_whitebal(s, val);
      else if(!strcmp(variable, "agc")) res = s->set_gain_ctrl(s, val);
      else if(!strcmp(variable, "aec")) res = s->set_exposure_ctrl(s, val);
      else if(!strcmp(variable, "hmirror")) res = s->set_hmirror(s, val);
      else if(!strcmp(variable, "vflip")) res = s->set_vflip(s, val);
      else if(!strcmp(variable, "awb_gain")) res = s->set_awb_gain(s, val);
      else if(!strcmp(variable, "agc_gain")) res = s->set_agc_gain(s, val);
      else if(!strcmp(variable, "aec_value")) res = s->set_aec_value(s, val);
      else if(!strcmp(variable, "aec2")) res = s->set_aec2(s, val);
      else if(!strcmp(variable, "dcw")) res = s->set_dcw(s, val);
      else if(!strcmp(variable, "bpc")) res = s->set_bpc(s, val);
      else if(!strcmp(variable, "wpc")) res = s->set_wpc(s, val);
      else if(!strcmp(variable, "raw_gma")) res = s->set_raw_gma(s, val);
      else if(!strcmp(variable, "lenc")) res = s->set_lenc(s, val);
      else if(!strcmp(variable, "special_effect")) res = s->set_special_effect(s, val);
      else if(!strcmp(variable, "wb_mode")) res = s->set_wb_mode(s, val);
      else if(!strcmp(variable, "ae_level")) res = s->set_ae_level(s, val);   
      else if(!strcmp(variable, "flash")) {
        ledcWrite(4,val);
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

//顯示視訊參數狀態
static esp_err_t status_handler(httpd_req_t *req){
    static char response[1024];

    sensor_t * s = esp_camera_sensor_get();
    char * p = response;
    p+=sprintf(p, "framesize:%u<br>", s->status.framesize);
    p+=sprintf(p, "quality:%u<br>", s->status.quality);
    p+=sprintf(p, "brightness:%d<br>", s->status.brightness);
    p+=sprintf(p, "contrast:%d<br>", s->status.contrast);
    p+=sprintf(p, "saturation:%d<br>", s->status.saturation);
    p+=sprintf(p, "special_effect:%u<br>", s->status.special_effect);
    p+=sprintf(p, "wb_mode:%u<br>", s->status.wb_mode);
    p+=sprintf(p, "awb:%u<br>", s->status.awb);
    p+=sprintf(p, "awb_gain:%u<br>", s->status.awb_gain);
    p+=sprintf(p, "aec:%u<br>", s->status.aec);
    p+=sprintf(p, "aec2:%u<br>", s->status.aec2);
    p+=sprintf(p, "ae_level:%d<br>", s->status.ae_level);
    p+=sprintf(p, "aec_value:%u<br>", s->status.aec_value);
    p+=sprintf(p, "agc:%u<br>", s->status.agc);
    p+=sprintf(p, "agc_gain:%u<br>", s->status.agc_gain);
    p+=sprintf(p, "gainceiling:%u<br>", s->status.gainceiling);
    p+=sprintf(p, "bpc:%u<br>", s->status.bpc);
    p+=sprintf(p, "wpc:%u<br>", s->status.wpc);
    p+=sprintf(p, "raw_gma:%u<br>", s->status.raw_gma);
    p+=sprintf(p, "lenc:%u<br>", s->status.lenc);
    p+=sprintf(p, "vflip:%u<br>", s->status.vflip);
    p+=sprintf(p, "hmirror:%u<br>", s->status.hmirror);
    p+=sprintf(p, "dcw:%u<br>", s->status.dcw);
    p+=sprintf(p, "colorbar:%u<br>", s->status.colorbar);
    *p++ = 0;
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, response, strlen(response));
}

//自訂網頁首頁管理介面
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!doctype html>
  <html>
  <head>
      <meta charset="utf-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <meta http-equiv="Expires" Content="0">
  </head>
  <script>
  function SendCommand() {
    var P1=document.getElementById("P1").value;
    var P2=document.getElementById("P2").value;
    var P3=document.getElementById("P3").value;
    var P4=document.getElementById("P4").value;
    var P5=document.getElementById("P5").value;
        
    document.getElementById("Send").disabled=true;
    if (document.getElementById("choice1").checked) {
      var cmd=document.getElementById("cmd1").value;
      var link=location.origin+"/control?"+cmd+"="+P1+";"+P2+";"+P3+";"+P4+";"+P5;
    }
    else if (document.getElementById("choice2").checked) {
      var cmd=document.getElementById("cmd2").value;
      var link=location.origin+"/control?var="+cmd+"&val="+P1;
    }    
    else if (document.getElementById("choice3").checked) {
      var cmd=document.getElementById("cmd3").value;
      var link=location.origin+"/control?"+cmd+"="+P1+";"+P2+";"+P3+";"+P4+";"+P5;
    }

    document.getElementById("ShowUrl").innerHTML="<font color=red>URL："+link+"</font>";
    document.getElementById("MyFirmata").src=link;
    setTimeout(wait,1000);
  }
  
  function wait() {
    document.getElementById("Send").disabled=false;
  }
  </script>
  <body>
  <button onclick="document.getElementById('stream').src=location.origin+':81/stream';">Start Stream</button><button onclick="document.getElementById('stream').src='';">Stop Stream</button><button onclick="document.getElementById('stream').src=window.location.origin+'/capture?'+Math.floor(Math.random()*1000000);">Get Still</button></td>
  <br>
  <img id="stream" src="" crossorigin="anonymous">
  <br>  
  <form onreset="document.getElementById('MyFirmata').src='about:blank';">
      <table width="350px">
      <tr>
        <td>
        <input type="radio" name="choice" id="choice1" checked>
        <select name="cmd1" id="cmd1">
        <option value="ip">ip</option>
        <option value="mac">mac</option>
        <option value="restart">restart</option>
        <option value="inputpullup">inputpullup</option>
        <option value="pinmode">pinmode</option>
        <option value="digitalwrite">digitalwrite(gpio,value)</option>
        <option value="analogwrite">analogwrite(gpio,value)</option>
        <option value="digitalread">digitalread(gpio)</option>
        <option value="analogread">analogread(gpio)</option>
        <option value="touchread">touchread(gpio)</option>
        <option value="tcp">tcp(domain,port,request,wait)</option>
        <option value="ifttt">ifttt(event,key,value1,value2,value3)</option>
        <option value="thingspeakupdate">thingspeakupdate(key,field1~field8)</option>
        <option value="thingspeakread">thingspeakread(request)</option>
        <option value="linenotify">linenotify(token,request)</option>
        <option value="flash">flash(value)</option>
        <option value="servo">servo(value)</option>
        <option value="servo1">servo1(value)</option>
        <option value="servo2">servo2(value)</option>
        <option value="speedL">speedL(value)</option>
        <option value="speedR">speedR(value)</option>
        <option value="decelerate">decelerate(value)</option>
        <option value="car">car(state)</option>
        <option value="framesize">framesize(size)</option>     
        </select>
        <br>
      <tr>
        <td>
        <input type="radio" name="choice" id="choice2">
        <select name="cmd2" id="cmd2">
          <option value="framesize">framesize(value)</option>  
          <option value="quality">quality(value)</option>  
          <option value="brightness">brightness(value)</option>  
          <option value="contrast">contrast(value)</option>  
          <option value="saturation">saturation(value)</option>  
          <option value="gainceiling">gainceiling(value)</option>  
          <option value="colorbar">colorbar(value)</option>  
          <option value="awb">awb(value)</option>  
          <option value="agc">agc(value)</option>  
          <option value="aec">aec(value)</option>  
          <option value="hmirror">hmirror(value)</option>  
          <option value="vflip">vflip(value)</option>  
          <option value="awb_gain">awb_gain(value)</option>  
          <option value="agc_gain">agc_gain(value)</option>  
          <option value="aec_value">aec_value(value)</option>  
          <option value="aec2">aec2(value)</option>  
          <option value="dcw">dcw(value)</option>  
          <option value="bpc">bpc(value)</option>  
          <option value="wpc">wpc(value)</option>  
          <option value="raw_gma">raw_gma(value)</option>  
          <option value="lenc">lenc(value)</option>  
          <option value="special_effect">special_effect(value)</option>  
          <option value="wb_mode">wb_mode(value)</option>  
          <option value="ae_level">ae_level(value)</option>  
          <option value="flash">flash(value)</option>  
        </select>
        <br>        
        <input type="radio" name="choice" id="choice3">
        <input type="text" name="cmd3" id="cmd3" size="23">
        </td>
      </tr>
      <tr>
      <td>P1：<input type="text" name="P1" id="P1" size="30" onclick="this.select()"></td>
      </tr>
      <tr>
      <td>P2：<input type="text" name="P2" id="P2" size="30" onclick="this.select()"></td>
      </tr>
      <tr>
      <td>P3：<input type="text" name="P3" id="P3" size="30" onclick="this.select()"></td>
      </tr>
      <tr>
      <td>P4：<input type="text" name="P4" id="P4" size="30" onclick="this.select()"></td>
      </tr>
      <tr>
      <td>P5：<input type="text" name="P5" id="P5" size="30" onclick="this.select()"></td>
      </tr>
      <tr>
      <td align="center"><input type="reset" value="Clear" height="50">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type="button" id="Send" value="Send" onclick="SendCommand();">&nbsp;&nbsp;&nbsp;</td>
      </tr>
    </table>
  </form>
  <iframe id="MyFirmata" width="270" height="150"></iframe><br>
  Command Format：<br>
  http://APIP/control?cmd=P1;P2;P3;P4;P5<br>
  http://STAIP/?cmd=P1;P2;P3;P4;P5<br>
  <div id="ShowUrl"></div>
  </body>
  </html>
)rawliteral";

//網頁首頁   http://192.168.xxx.xxx
static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

void startCameraServer(){
/*
  #define HTTPD_DEFAULT_CONFIG() {                   \
     .task_priority      = tskIDLE_PRIORITY+5,       \
     .stack_size         = 4096,                     \    
     .server_port        = 80,                       \  
     .ctrl_port          = 32768,                    \
     .max_open_sockets   = 7,                        \
     .max_uri_handlers   = 8,                        \
     .max_resp_headers   = 8,                        \
     .backlog_conn       = 5,                        \
     .lru_purge_enable   = false,                    \
     .recv_wait_timeout  = 5,                        \
     .send_wait_timeout  = 5,                        \
     .global_user_ctx = NULL,                        \
     .global_user_ctx_free_fn = NULL,                \
     .global_transport_ctx = NULL,                   \
     .global_transport_ctx_free_fn = NULL,           \
     .open_fn = NULL,                                \
     .close_fn = NULL,                               \
  } 
*/  

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();  //可在HTTPD_DEFAULT_CONFIG()中設定Server Port

  //可自訂網址路徑
  //http://192.168.xxx.xxx/
  httpd_uri_t index_uri = {
      .uri       = "/",
      .method    = HTTP_GET,
      .handler   = index_handler,
      .user_ctx  = NULL
  };

  httpd_uri_t status_uri = {
      .uri       = "/status",
      .method    = HTTP_GET,
      .handler   = status_handler,
      .user_ctx  = NULL
  };

  httpd_uri_t cmd_uri = {
      .uri       = "/control",
      .method    = HTTP_GET,
      .handler   = cmd_handler,
      .user_ctx  = NULL
  }; 

  httpd_uri_t capture_uri = {
      .uri       = "/capture",
      .method    = HTTP_GET,
      .handler   = capture_handler,
      .user_ctx  = NULL
  };

  httpd_uri_t stream_uri = {
      .uri       = "/stream",
      .method    = HTTP_GET,
      .handler   = stream_handler,
      .user_ctx  = NULL
  };
  
  ra_filter_init(&ra_filter, 20);
  
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

//拆解命令字串置入變數
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
          
String tcp_http(String domain,String request,int port,byte wait)
{
  String getAll="", getBody="";

  WiFiClient client_tcp;
  if (client_tcp.connect(domain.c_str(), port)) {
    Serial.println("GET " + request);
    client_tcp.println("GET " + request + " HTTP/1.1");
    client_tcp.println("Host: " + domain);
    client_tcp.println("Connection: close");
    client_tcp.println();

    boolean state = false;
    int waitTime = 10000;   // timeout 10 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis()) {
      while (client_tcp.available()) {
        char c = client_tcp.read();
        if (c == '\n') {
          if (getAll.length()==0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        if (state==true) getBody += String(c);
        if (wait==1)
          startTime = millis();
       }
       if (wait==0)
        if (getBody.length()!= 0) break;
    }
    client_tcp.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to "+domain+" failed.";
    Serial.println("Connected to "+domain+" failed.");
  }
  
  return getBody; 
}

String LineNotify(String token, String message, byte wait)
{
  message.replace("%","%25");         
  message.replace(" ","%20");
  message.replace("&","%20");
  message.replace("#","%20");
  //message.replace("\'","%27");
  message.replace("\"","%22");
  message.replace("\n","%0D%0A");
  message.replace("%3Cbr%3E","%0D%0A");
  message.replace("%3Cbr/%3E","%0D%0A");
  message.replace("%3Cbr%20/%3E","%0D%0A");
  message.replace("%3CBR%3E","%0D%0A");
  message.replace("%3CBR/%3E","%0D%0A");
  message.replace("%3CBR%20/%3E","%0D%0A");     
  message.replace("%20stickerPackageId","&stickerPackageId");
  message.replace("%20stickerId","&stickerId");    
  
  http.begin("http://linenotify.com/notify.php?token="+token+"&"+message);
  int httpCode = http.GET();
  if(httpCode > 0) {
      return http.getString();
  } else 
      return "Connection Error!";
}
