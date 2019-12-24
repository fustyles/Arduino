/*
ESP32-CAM BodyPix
Open the page in Chrome.
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-12-24 01:00
https://www.facebook.com/francefu

自訂指令格式 :  
http://APIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

預設AP端IP： 192.168.4.1
http://192.168.xxx.xxx/control?ip
http://192.168.xxx.xxx/control?mac
http://192.168.xxx.xxx/control?restart
http://192.168.xxx.xxx/control?flash=value        //value= 0~255

查詢Client端IP：
查詢IP：http://192.168.4.1/?ip
重設網路：http://192.168.4.1/?resetwifi=ssid;password

http://192.168.xxx.xxx             //網頁首頁管理介面
http://192.168.xxx.xxx:81/stream   //取得串流影像
http://192.168.xxx.xxx/capture     //取得影像
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
      
查詢Client端IP：
查詢IP：http://192.168.xxx.xxx/?ip
重設網路：http://192.168.xxx.xxx/?resetwifi=ssid;password
*/

//輸入WIFI連線帳號密碼
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

//輸入AP端連線帳號密碼
const char* apssid = "ESP32-CAM";
const char* appassword = "12345678";         //AP密碼至少要8個字元以上

#include <WiFi.h>
#include <esp32-hal-ledc.h>      //用於控制伺服馬達
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
  //https://github.com/espressif/esp32-camera/blob/master/driver/include/sensor.h
  config.pixel_format = PIXFORMAT_JPEG;    //影像格式：RGB565|YUV422|GRAYSCALE|JPEG|RGB888|RAW|RGB444|RGB555
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
  s->set_framesize(s, FRAMESIZE_QVGA);  //96x96|QQVGA|QQVGA2|QCIF|HQVGA|240x240|QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA|QXGA|INVALID

  //閃光燈(GPIO4)
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);
  
  WiFi.mode(WIFI_AP_STA);  //其他模式 WiFi.mode(WIFI_AP); WiFi.mode(WIFI_STA);

  //指定Client端靜態IP
  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  WiFi.begin(ssid, password);    //執行網路連線

  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) {
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
      else if (cmd=="flash") {  //控制內建閃光燈
        ledcAttachPin(4, 4);  
        ledcSetup(4, 5000, 8);   
         
        int val = P1.toInt();
        ledcWrite(4,val);  
      }  
      else {
        Feedback="Command is not defined";
      }

      if (Feedback=="") Feedback=Command;  //若沒有設定回傳資料就回傳Command值
    
      static char response[15000];
      char * p = response;
      p+=sprintf(p, "%s", Feedback.c_str());   
      *p++ = 0;
      httpd_resp_set_type(req, "text/html");  //設定回傳資料格式
      httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");  //允許跨網域讀取
      return httpd_resp_send(req, response, strlen(response));
    } 
    else {
      int val = atoi(value);
      sensor_t * s = esp_camera_sensor_get();
      int res = 0;

      //官方指令區塊，也可在此自訂指令  http://192.168.xxx.xxx/control?var=xxx&val=xxx
      if(!strcmp(variable, "framesize")) {
        if(s->pixformat == PIXFORMAT_JPEG) 
          res = s->set_framesize(s, (framesize_t)val);
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
      else {
          res = -1;
      }
  
      if(res){
          return httpd_resp_send_500(req);
      }

      if (buf) {
        Feedback = String(buf);
        static char response[128];
        char * p = response;
        p+=sprintf(p, "%s", Feedback.c_str());   
        *p++ = 0;
        httpd_resp_set_type(req, "text/html");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        return httpd_resp_send(req, response, strlen(response));  //回傳參數字串
      }
      else {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        return httpd_resp_send(req, NULL, 0);
      }
    }
}

//顯示視訊參數狀態(須回傳json格式)
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

//自訂網頁首頁
static const char PROGMEM INDEX_HTML[] = R"rawliteral(<!doctype html>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <meta http-equiv="Access-Control-Allow-Headers" content="Origin, X-Requested-With, Content-Type, Accept">
        <meta http-equiv="Access-Control-Allow-Methods" content="GET,POST,PUT,DELETE,OPTIONS">
        <meta http-equiv="Access-Control-Allow-Origin" content="*">
        <title>ESP32 OV2460</title>
        <style>
          body{font-family:Arial,Helvetica,sans-serif;background:#181818;color:#EFEFEF;font-size:16px}h2{font-size:18px}section.main{display:flex}#menu,section.main{flex-direction:column}#menu{display:none;flex-wrap:nowrap;min-width:340px;background:#363636;padding:8px;border-radius:4px;margin-top:-10px;margin-right:10px}#content{display:flex;flex-wrap:wrap;align-items:stretch}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}figure img{display:block;width:100%;height:auto;border-radius:4px;margin-top:8px}@media (min-width: 800px) and (orientation:landscape){#content{display:flex;flex-wrap:nowrap;align-items:stretch}figure img{display:block;max-width:100%;max-height:calc(100vh - 40px);width:auto;height:auto}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}}section#buttons{display:flex;flex-wrap:nowrap;justify-content:space-between}#nav-toggle{cursor:pointer;display:block}#nav-toggle-cb{outline:0;opacity:0;width:0;height:0}#nav-toggle-cb:checked+#menu{display:flex}.input-group{display:flex;flex-wrap:nowrap;line-height:22px;margin:5px 0}.input-group>label{display:inline-block;padding-right:10px;min-width:47%}.input-group input,.input-group select{flex-grow:1}.range-max,.range-min{display:inline-block;padding:0 5px}button{display:block;margin:5px;padding:0 12px;border:0;line-height:28px;cursor:pointer;color:#fff;background:#ff3034;border-radius:5px;font-size:16px;outline:0}button:hover{background:#ff494d}button:active{background:#f21c21}button.disabled{cursor:default;background:#a0a0a0}input[type=range]{-webkit-appearance:none;width:100%;height:22px;background:#363636;cursor:pointer;margin:0}input[type=range]:focus{outline:0}input[type=range]::-webkit-slider-runnable-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-webkit-slider-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;-webkit-appearance:none;margin-top:-11.5px}input[type=range]:focus::-webkit-slider-runnable-track{background:#EFEFEF}input[type=range]::-moz-range-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-moz-range-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer}input[type=range]::-ms-track{width:100%;height:2px;cursor:pointer;background:0 0;border-color:transparent;color:transparent}input[type=range]::-ms-fill-lower{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-fill-upper{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;height:2px}input[type=range]:focus::-ms-fill-lower{background:#EFEFEF}input[type=range]:focus::-ms-fill-upper{background:#363636}.switch{display:block;position:relative;line-height:22px;font-size:16px;height:22px}.switch input{outline:0;opacity:0;width:0;height:0}.slider{width:50px;height:22px;border-radius:22px;cursor:pointer;background-color:grey}.slider,.slider:before{display:inline-block;transition:.4s}.slider:before{position:relative;content:"";border-radius:50%;height:16px;width:16px;left:4px;top:3px;background-color:#fff}input:checked+.slider{background-color:#ff3034}input:checked+.slider:before{-webkit-transform:translateX(26px);transform:translateX(26px)}select{border:1px solid #363636;font-size:14px;height:22px;outline:0;border-radius:5px}.image-container{position:relative;min-width:160px}.close{position:absolute;right:5px;top:5px;background:#ff3034;width:16px;height:16px;border-radius:100px;color:#fff;text-align:center;line-height:18px;cursor:pointer}.hidden{display:none}
        </style>
        <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow/tfjs@1.0.0"> </script>
        <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow-models/body-pix@1.0.0"> </script>          
    </head>
    <body>
        <section class="main">
            <section id="buttons">
                <table>
                <tr><td><button id="get-still" style="display:none">Restart</button></td><td>Style<select id="kind"><option value="1">backgroundDarkeningMask</option><option value="2">segmentation</option><option value="3">coloredPartImageData</option><option value="4">coloredPartImage</option></select></td><td style="display:none"><button id="toggle-stream"></button></td></tr>
                <tr><td>Flash</td><td align="center" colspan="2"><input type="range" id="flash" min="0" max="255" value="0" onchange="try{fetch(document.location.origin+'/control?flash='+this.value);}catch(e){}"></td></tr>
                </table>
            </section>
            <figure>
              <div id="stream-container" class="image-container hidden">
                <div class="close" id="close-stream">×</div>
                <img id="stream" src="" style="display:none"><br>
                <canvas id="canvas"></canvas><br>
                <canvas id="canvas1"></canvas><br>
                <canvas id="canvas2"></canvas><br>
                <canvas id="canvas3"></canvas><br>
                <canvas id="canvas4"></canvas>                 
              </div>
            </figure>         
            <div id="logo">
                <label for="nav-toggle-cb" id="nav-toggle">&#9776;&nbsp;&nbsp;Toggle settings</label>
            </div>
            <div id="content">
                <div id="sidebar">
                    <input type="checkbox" id="nav-toggle-cb">
                    <nav id="menu">  
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
                        <div class="input-group" id="saturation-group">
                            <label for="saturation">Saturation</label>
                            <div class="range-min">-2</div>
                            <input type="range" id="saturation" min="-2" max="2" value="0" class="default-action">
                            <div class="range-max">2</div>
                        </div>
                        <div class="input-group" id="special_effect-group">
                            <label for="special_effect">Special Effect</label>
                            <select id="special_effect" class="default-action">
                                <option value="0" selected="selected">No Effect</option>
                                <option value="1">Negative</option>
                                <option value="2">Grayscale</option>
                                <option value="3">Red Tint</option>
                                <option value="4">Green Tint</option>
                                <option value="5">Blue Tint</option>
                                <option value="6">Sepia</option>
                            </select>
                        </div>
                        <div class="input-group" id="awb-group">
                            <label for="awb">AWB</label>
                            <div class="switch">
                                <input id="awb" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="awb"></label>
                            </div>
                        </div>
                        <div class="input-group" id="awb_gain-group">
                            <label for="awb_gain">AWB Gain</label>
                            <div class="switch">
                                <input id="awb_gain" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="awb_gain"></label>
                            </div>
                        </div>
                        <div class="input-group" id="wb_mode-group">
                            <label for="wb_mode">WB Mode</label>
                            <select id="wb_mode" class="default-action">
                                <option value="0" selected="selected">Auto</option>
                                <option value="1">Sunny</option>
                                <option value="2">Cloudy</option>
                                <option value="3">Office</option>
                                <option value="4">Home</option>
                            </select>
                        </div>
                        <div class="input-group" id="aec-group">
                            <label for="aec">AEC SENSOR</label>
                            <div class="switch">
                                <input id="aec" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="aec"></label>
                            </div>
                        </div>
                        <div class="input-group" id="aec2-group">
                            <label for="aec2">AEC DSP</label>
                            <div class="switch">
                                <input id="aec2" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="aec2"></label>
                            </div>
                        </div>
                        <div class="input-group" id="ae_level-group">
                            <label for="ae_level">AE Level</label>
                            <div class="range-min">-2</div>
                            <input type="range" id="ae_level" min="-2" max="2" value="0" class="default-action">
                            <div class="range-max">2</div>
                        </div>
                        <div class="input-group" id="aec_value-group">
                            <label for="aec_value">Exposure</label>
                            <div class="range-min">0</div>
                            <input type="range" id="aec_value" min="0" max="1200" value="204" class="default-action">
                            <div class="range-max">1200</div>
                        </div>
                        <div class="input-group" id="agc-group">
                            <label for="agc">AGC</label>
                            <div class="switch">
                                <input id="agc" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="agc"></label>
                            </div>
                        </div>
                        <div class="input-group hidden" id="agc_gain-group">
                            <label for="agc_gain">Gain</label>
                            <div class="range-min">1x</div>
                            <input type="range" id="agc_gain" min="0" max="30" value="5" class="default-action">
                            <div class="range-max">31x</div>
                        </div>
                        <div class="input-group" id="gainceiling-group">
                            <label for="gainceiling">Gain Ceiling</label>
                            <div class="range-min">2x</div>
                            <input type="range" id="gainceiling" min="0" max="6" value="0" class="default-action">
                            <div class="range-max">128x</div>
                        </div>
                        <div class="input-group" id="bpc-group">
                            <label for="bpc">BPC</label>
                            <div class="switch">
                                <input id="bpc" type="checkbox" class="default-action">
                                <label class="slider" for="bpc"></label>
                            </div>
                        </div>
                        <div class="input-group" id="wpc-group">
                            <label for="wpc">WPC</label>
                            <div class="switch">
                                <input id="wpc" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="wpc"></label>
                            </div>
                        </div>
                        <div class="input-group" id="raw_gma-group">
                            <label for="raw_gma">Raw GMA</label>
                            <div class="switch">
                                <input id="raw_gma" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="raw_gma"></label>
                            </div>
                        </div>
                        <div class="input-group" id="lenc-group">
                            <label for="lenc">Lens Correction</label>
                            <div class="switch">
                                <input id="lenc" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="lenc"></label>
                            </div>
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
                        <div class="input-group" id="dcw-group">
                            <label for="dcw">DCW (Downsize EN)</label>
                            <div class="switch">
                                <input id="dcw" type="checkbox" class="default-action" checked="checked">
                                <label class="slider" for="dcw"></label>
                            </div>
                        </div>
                        <div class="input-group" id="colorbar-group">
                            <label for="colorbar">Color Bar</label>
                            <div class="switch">
                                <input id="colorbar" type="checkbox" class="default-action">
                                <label class="slider" for="colorbar"></label>
                            </div>
                        </div>
                    </nav>
                </div>
            </div>
        </section>
        <div id="result"><div>  
             
        <script>
          document.addEventListener('DOMContentLoaded', function (event) {
            var baseHost = document.location.origin
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
              if (updateRemote && initialValue !== value) {
                updateConfig(el);
              } else if(!updateRemote){
                if(el.id === "aec"){
                  value ? hide(exposure) : show(exposure)
                } else if(el.id === "agc"){
                  if (value) {
                    show(gainCeiling)
                    hide(agcGain)
                  } else {
                    hide(gainCeiling)
                    show(agcGain)
                  }
                } else if(el.id === "awb_gain"){
                  value ? show(wb) : hide(wb)
                }
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
              })
            const view = document.getElementById('stream')
            const viewContainer = document.getElementById('stream-container')
            const stillButton = document.getElementById('get-still')
            const streamButton = document.getElementById('toggle-stream')
            const closeButton = document.getElementById('close-stream')
            const stopStream = () => {
              //window.stop();
              view.src="";
              streamButton.innerHTML = 'Start Stream'
            }
            const startStream = () => {
              view.src = `${streamUrl}/stream`
              show(viewContainer)
              streamButton.innerHTML = 'Stop Stream'
            }
            // Attach actions to buttons
            stillButton.onclick = () => {
              stopStream()
              try{
                view.src = `${baseHost}/capture?_cb=${Date.now()}`
              }
              catch(e) {
                view.src = `${baseHost}/capture?_cb=${Date.now()}`  
              }
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
            // Custom actions
            // Gain
            const agc = document.getElementById('agc')
            const agcGain = document.getElementById('agc_gain-group')
            const gainCeiling = document.getElementById('gainceiling-group')
            agc.onchange = () => {
              updateConfig(agc)
              if (agc.checked) {
                show(gainCeiling)
                hide(agcGain)
              } else {
                hide(gainCeiling)
                show(agcGain)
              }
            }
            // Exposure
            const aec = document.getElementById('aec')
            const exposure = document.getElementById('aec_value-group')
            aec.onchange = () => {
              updateConfig(aec)
              aec.checked ? hide(exposure) : show(exposure)
            }
            // AWB
            const awb = document.getElementById('awb_gain')
            const wb = document.getElementById('wb_mode-group')
            awb.onchange = () => {
              updateConfig(awb)
              awb.checked ? show(wb) : hide(wb)
            }
          })
        </script>
        
    <!-- Object Detection -->    
    <script>
    var getStill = document.getElementById('get-still');
    var ShowImage = document.getElementById('stream');
    var kind = document.getElementById('kind')
    var canvas = document.getElementById('canvas');
    var context = canvas.getContext('2d');
    var canvas1 = document.getElementById('canvas1'); 
    var canvas2 = document.getElementById('canvas2');
    var canvas3 = document.getElementById('canvas3');
    var canvas4 = document.getElementById('canvas4');     
    var Model;
    var outputStride = 16;
    var segmentationThreshold = 0.5;
    
    function ObjectDetect() {
      result.innerHTML = "Please wait for loading model.";
      bodyPix.load().then(function(net) {
        Model = net;
        result.innerHTML = ""; 
        getStill.style.display = "block";
        getStill.click();
      }); 
    }
    
    function DetectImage() {
      canvas.setAttribute("width", ShowImage.width);
      canvas.setAttribute("height", ShowImage.height);       
      canvas1.setAttribute("width", ShowImage.width);
      canvas1.setAttribute("height", ShowImage.height);  
      canvas2.setAttribute("width", ShowImage.width);
      canvas2.setAttribute("height", ShowImage.height);  
      canvas3.setAttribute("width", ShowImage.width);
      canvas3.setAttribute("height", ShowImage.height);     
      canvas4.setAttribute("width", ShowImage.width);
      canvas4.setAttribute("height", ShowImage.height);
      context.drawImage(ShowImage, 0, 0, ShowImage.width, ShowImage.height); 
      
      Model.estimatePersonSegmentation(canvas, outputStride, segmentationThreshold).then(segmentation => {
        
        const maskBackground = true;
        const backgroundDarkeningMask = bodyPix.toMaskImageData(segmentation, maskBackground);
        const opacity = 0.7;
        const maskBlurAmount = 3;
        const flipHorizontal = false;
        const backgroundBlurAmount = 3;
        const edgeBlurAmount = 3;

        if (kind.value=="1") {
          canvas1.style.display="block";
          bodyPix.drawMask(canvas1, canvas, backgroundDarkeningMask, opacity, maskBlurAmount, flipHorizontal);
        }
        else
          canvas1.style.display="none";
          
        if (kind.value=="2") {
          canvas2.style.display="block";
          bodyPix.drawBokehEffect(canvas2, canvas, segmentation, backgroundBlurAmount, edgeBlurAmount, flipHorizontal);
        }
        else
          canvas2.style.display="none";   
               
        if (kind.value=="3") {
          canvas3.style.display="block";
          const rainbow = [
            [110, 64, 170], [143, 61, 178], [178, 60, 178], [210, 62, 167],
            [238, 67, 149], [255, 78, 125], [255, 94, 99],  [255, 115, 75],
            [255, 140, 56], [239, 167, 47], [217, 194, 49], [194, 219, 64],
            [175, 240, 91], [135, 245, 87], [96, 247, 96],  [64, 243, 115],
            [40, 234, 141], [28, 219, 169], [26, 199, 194], [33, 176, 213],
            [47, 150, 224], [65, 125, 224], [84, 101, 214], [99, 81, 195]
          ];
          const coloredPartImageData = bodyPix.toColoredPartImageData(segmentation, rainbow);
          const pixelCellWidth = 10.0;
          bodyPix.drawPixelatedMask(canvas3, canvas, coloredPartImageData, opacity, maskBlurAmount, flipHorizontal, pixelCellWidth);
        }
        else
          canvas3.style.display="none";   
               
        if (kind.value=="4") {
          canvas4.style.display="block";
          const warm = [
            [110, 64, 170], [106, 72, 183], [100, 81, 196], [92, 91, 206],
            [84, 101, 214], [75, 113, 221], [66, 125, 224], [56, 138, 226],
            [48, 150, 224], [40, 163, 220], [33, 176, 214], [29, 188, 205],
            [26, 199, 194], [26, 210, 182], [28, 219, 169], [33, 227, 155],
            [41, 234, 141], [51, 240, 128], [64, 243, 116], [79, 246, 105],
            [96, 247, 97],  [115, 246, 91], [134, 245, 88], [155, 243, 88]
          ];
          const coloredPartImage = bodyPix.toColoredPartImageData(segmentation, warm);
          bodyPix.drawMask(canvas4, canvas, coloredPartImage, 1.0, maskBlurAmount, flipHorizontal);
        }
        else
          canvas4.style.display="none";
          
        getStill.click();
      });   
    }
    
    ShowImage.onload = function (event) {
      if (Model) {
        try { 
          document.createEvent("TouchEvent");
          setTimeout(function(){DetectImage();},250);
        }
        catch(e) { 
          DetectImage();
        } 
      }
    }
    window.onload = function () { ObjectDetect(); }
    </script>
    </body>
</html>
)rawliteral";

//網頁首頁   http://192.168.xxx.xxx
static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

void startCameraServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();  //可在HTTPD_DEFAULT_CONFIG()中設定Server Port 

  //可自訂網址路徑
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
