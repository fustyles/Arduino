/*
ESP32-CAM Load images from SD card to enroll faces and recognize faces automatically without opening the page.
Author : ChungYi Fu (Kaohsiung, Taiwan)  2021-6-29 21:30
https://www.facebook.com/francefu

Arduino ESP32 version 1.0.6


http://192.168.xxx.xxx             //網頁首頁管理介面
http://192.168.xxx.xxx:81/stream   //取得串流影像      嵌入網頁語法 <img src="http://192.168.xxx.xxx:81/stream">
http://192.168.xxx.xxx/capture     //取得影像         嵌入網頁語法 <img src="http://192.168.xxx.xxx/capture">
http://192.168.xxx.xxx/status      //取得影像狀態值

自訂指令格式  http://192.168.xxx.xxx/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://192.168.xxx.xxx/control?facename=matched_id;name  //設定姓名
http://192.168.xxx.xxx/control?constrolstate=state       //state=0 or 1 設定是否執行函式 void FaceMatched(), void FaceNoMatched()
官方指令格式  http://192.168.xxx.xxx/control?var=xxx&val=xxx
http://192.168.xxx.xxx/control?var=flash&val=value          //閃光燈 value= 0~255
http://192.168.xxx.xxx/control?var=framesize&val=value      //解析度 value = 10->UXGA(1600x1200), 9->SXGA(1280x1024), 8->XGA(1024x768) ,7->SVGA(800x600), 6->VGA(640x480), 5 selected=selected->CIF(400x296), 4->QVGA(320x240), 3->HQVGA(240x176), 0->QQVGA(160x120), 11->QXGA(2048x1564 for OV3660)
http://192.168.xxx.xxx/control?var=quality&val=value        //畫質 value = 10 ~ 63
http://192.168.xxx.xxx/control?var=brightness&val=value     //亮度 value = -2 ~ 2
http://192.168.xxx.xxx/control?var=contrast&val=value       //對比 value = -2 ~ 2
*/

//輸入WIFI連線帳號密碼
const char* ssid = "teacher";
const char* password = "87654321";

String LineToken = "";  //傳送區域網路IP至Line通知(用不到可不填)

//輸入AP端連線帳號密碼  http://192.168.4.1
const char* apssid = "esp32-cam";
const char* appassword = "12345678";         //AP密碼至少要8個字元以上  

boolean streamState = false;

//人臉辨識同一人人臉註冊影像數
#define ENROLL_CONFIRM_TIMES 5

//可由網頁get-still按鈕取得解析度CIF影像另存於SD卡 http://192.168.xxx.xxx/capture  (FRAMESIZE_CIF)
String filepath[5] = {"/1.jpg", "/2.jpg", "/3.jpg", "/4.jpg", "/5.jpg"};  //1.jpg, 2.jpg, ...., 35.jpg
int image_width = 400;  
int image_height = 296;

//人臉辨識同一人人臉註冊影像數
#define ENROLL_CONFIRM_TIMES 5
//人臉辨識註冊人數
#define FACE_ID_SAVE_NUMBER 7

//設定人臉辨識顯示的人名
String recognize_face_matched_name[7] = {"Name0","Name1","Name2","Name3","Name4","Name5","Name6"};

boolean controlState = false;  //是否執行函式 void FaceMatched(), void FaceNoMatched

#include <WiFi.h>
#include <HTTPClient.h>
HTTPClient http;
#include "soc/soc.h"             //用於電源不穩不重開機 
#include "soc/rtc_cntl_reg.h"    //用於電源不穩不重開機 
#include "esp_camera.h"          //視訊函式
#include "img_converters.h"      //影像格式轉換函式
#include "fb_gfx.h"              //影像繪圖函式
#include "fd_forward.h"          //人臉偵測函式
#include "fr_forward.h"          //人臉辨識函式
#include "FS.h"                  //檔案系統函式
#include "SD_MMC.h"              //SD卡存取函式
#include "esp_http_server.h"     //HTTP Server函式
#include "esp_timer.h"           //計時器函式

String Feedback="";   //回傳客戶端訊息
//指令參數值
String Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
//指令拆解狀態值
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

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

//設定人臉偵測、辨識框線、文字顏色變數
#define FACE_COLOR_WHITE  0x00FFFFFF
#define FACE_COLOR_BLACK  0x00000000
#define FACE_COLOR_RED    0x000000FF
#define FACE_COLOR_GREEN  0x0000FF00
#define FACE_COLOR_BLUE   0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN   (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)

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

//初始值
static mtmn_config_t mtmn_config = {0};
static int8_t detection_enabled = 1;     //人臉偵測狀態 0 or 1
static int8_t recognition_enabled = 1;   //人臉辨識狀態 0 or 1
static int8_t is_enrolling = 0;
static face_id_list id_list = {0};
static int8_t flash_value = 0;
int8_t enroll_id = 0;

//https://github.com/espressif/esp-dl/blob/master/face_detection/README.md
box_array_t *net_boxes = NULL;

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
  s->set_framesize(s, FRAMESIZE_CIF);    //解析度 UXGA(1600x1200), SXGA(1280x1024), XGA(1024x768), SVGA(800x600), VGA(640x480), CIF(400x296), QVGA(320x240), HQVGA(240x176), QQVGA(160x120), QXGA(2048x1564 for OV3660)

  //s->set_vflip(s, 1);  //垂直翻轉
  //s->set_hmirror(s, 1);  //水平鏡像
          
  //閃光燈(GPIO4)
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);

  encrollImageSD();  //讀取SD卡圖檔註冊人臉   
  
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
  
      if (LineToken != "") 
        LineNotify_http_get(LineToken, WiFi.localIP().toString());
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
  
  startCameraServer();    //啟動視訊服務器 

  //設定閃光燈為低電位
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);              
}

void loop() {
  if (streamState == false&&recognition_enabled==1) {
    faceRecognition();
  }
  delay(100);
}


void FaceMatched(int faceid) {  //辨識到註冊人臉執行指令控制
  if (faceid==0) {  
  } 
  else if (faceid==1) { 
  } 
  else if (faceid==2) { 
  } 
  else if (faceid==3) { 
  } 
  else if (faceid==4) { 
  } 
  else if (faceid==5) { 
  } 
  else if (faceid==6) {
  } 
  else {
  }   
}

void FaceNoMatched() {  //辨識為陌生人臉執行指令控制
  
}

void encrollImageSD() {
  //臉部偵測參數設定  https://github.com/espressif/esp-face/blob/master/face_detection/README.md
  mtmn_config.type = FAST;  //FAST or NORMAL
  mtmn_config.min_face = 80;
  mtmn_config.pyramid = 0.707;
  mtmn_config.pyramid_times = 4;
  mtmn_config.p_threshold.score = 0.6;
  mtmn_config.p_threshold.nms = 0.7;
  mtmn_config.p_threshold.candidate_number = 20;
  mtmn_config.r_threshold.score = 0.7;
  mtmn_config.r_threshold.nms = 0.7;
  mtmn_config.r_threshold.candidate_number = 10;
  mtmn_config.o_threshold.score = 0.7;
  mtmn_config.o_threshold.nms = 0.7;
  mtmn_config.o_threshold.candidate_number = 1;
    
  //SD卡初始化
  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    ESP.restart();
  }  
  
  fs::FS &fs = SD_MMC;
  
  face_id_init(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
  dl_matrix3du_t *aligned_face = NULL;
  int8_t left_sample_face = NULL;
  dl_matrix3du_t *image_matrix = NULL;
  
  for (int j=0;j<sizeof(filepath)/sizeof(*filepath);j++) {
    File file = fs.open(filepath[j]);
    Serial.println("detect file: "+filepath[j]);
    if(!file){
      Serial.println("Failed to open file for reading");
      SD_MMC.end();    
    } else {
      Serial.println("file size: "+String(file.size())); 
      char *buf;
      buf = (char*) malloc (sizeof(char)*file.size());
      long i = 0;
      while (file.available()) {
        buf[i] = file.read(); 
        i++;  
      }
  
      image_matrix = dl_matrix3du_alloc(1, image_width, image_height, 3);  //分配內部記憶體
      if (!image_matrix) {
          Serial.println("dl_matrix3du_alloc failed");
      } else {          
          fmt2rgb888((uint8_t*)buf, file.size(), PIXFORMAT_JPEG, image_matrix->item);  //影像格式轉換RGB格式
          box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);  //執行人臉偵測取得臉框數據
          if (net_boxes){
            Serial.println("faces = " + String(net_boxes->len));  //偵測到的人臉數
            Serial.println();
            for (int i = 0; i < net_boxes->len; i++){  //列舉人臉位置與大小
                Serial.println("index = " + String(i));
                int x = (int)net_boxes->box[i].box_p[0];
                Serial.println("x = " + String(x));
                int y = (int)net_boxes->box[i].box_p[1];
                Serial.println("y = " + String(y));
                int w = (int)net_boxes->box[i].box_p[2] - x + 1;
                Serial.println("width = " + String(w));
                int h = (int)net_boxes->box[i].box_p[3] - y + 1;
                Serial.println("height = " + String(h));
                Serial.println();

                //註冊人臉
                if (i==0) {
                  aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
                  if (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK){
                    if(!aligned_face){
                        Serial.println("Could not allocate face recognition buffer");
                    } 
                    else {
                      int8_t left_sample_face = enroll_face(&id_list, aligned_face);
          
                      if(left_sample_face == (ENROLL_CONFIRM_TIMES - 1)){
                          enroll_id = id_list.tail;
                          Serial.printf("Enrolling Face ID: %d\n", enroll_id);
                      }
                      Serial.printf("Enrolling Face ID: %d sample %d\n", enroll_id, ENROLL_CONFIRM_TIMES - left_sample_face);
                      if (left_sample_face == 0){
                          enroll_id = id_list.tail;
                          //Serial.printf("Enrolled Face ID: %d\n", enroll_id);
                      }
                      Serial.println();
                    }
                    dl_matrix3du_free(aligned_face);
                  }
                }
            } 
            dl_lib_free(net_boxes->score);
            dl_lib_free(net_boxes->box);
            dl_lib_free(net_boxes->landmark);
            dl_lib_free(net_boxes);                                
            net_boxes = NULL;
          }
          else {
            Serial.println("No Face");    //未偵測到人臉
            Serial.println();
          }
          dl_matrix3du_free(image_matrix);
      }
      free(buf);
    }
    file.close();
  } 
  
  SD_MMC.end();
  Serial.println();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);   
}

//影像輸出人臉辨識文字
static void rgb_print(dl_matrix3du_t *image_matrix, uint32_t color, const char * str){
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_BGR888;
    fb_gfx_print(&fb, (fb.width - (strlen(str) * 14)) / 2, 10, color, str);
}

//影像輸出人臉辨識文字定位處理
static int rgb_printf(dl_matrix3du_t *image_matrix, uint32_t color, const char *format, ...){
    char loc_buf[64];
    char * temp = loc_buf;
    int len;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    len = vsnprintf(loc_buf, sizeof(loc_buf), format, arg);
    va_end(copy);
    if(len >= sizeof(loc_buf)){
        temp = (char*)malloc(len+1);
        if(temp == NULL) {
            return 0;
        }
    }
    vsnprintf(temp, len+1, format, arg);
    va_end(arg);
    rgb_print(image_matrix, color, temp);
    if(len > 64){
        free(temp);
    }
    return len;
}

//繪製臉部框架
static void draw_face_boxes(dl_matrix3du_t *image_matrix, box_array_t *boxes, int face_id){
    int x, y, w, h, i;
    uint32_t color = FACE_COLOR_YELLOW;
    if(face_id < 0){
        color = FACE_COLOR_RED;
    } else if(face_id > 0){
        color = FACE_COLOR_GREEN;
    }
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_BGR888;
    for (i = 0; i < boxes->len; i++){
        // rectangle box
        x = (int)boxes->box[i].box_p[0];
        y = (int)boxes->box[i].box_p[1];
        w = (int)boxes->box[i].box_p[2] - x + 1;
        h = (int)boxes->box[i].box_p[3] - y + 1;
        fb_gfx_drawFastHLine(&fb, x, y, w, color);
        fb_gfx_drawFastHLine(&fb, x, y+h-1, w, color);
        fb_gfx_drawFastVLine(&fb, x, y, h, color);
        fb_gfx_drawFastVLine(&fb, x+w-1, y, h, color);
#if 0
        // landmark
        int x0, y0, j;
        for (j = 0; j < 10; j+=2) {
            x0 = (int)boxes->landmark[i].landmark_p[j];
            y0 = (int)boxes->landmark[i].landmark_p[j+1];
            fb_gfx_fillRect(&fb, x0, y0, 3, 3, color);
        }
#endif
    }
}

void faceRecognition() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
      Serial.println("Camera capture failed");
      ESP.restart();
  }
  size_t out_len, out_width, out_height;
  uint8_t * out_buf;
  bool s;
  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);
  if (!image_matrix) {
      esp_camera_fb_return(fb);
      Serial.println("dl_matrix3du_alloc failed");
      return;
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
      return;
  }
  box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);  //執行人臉偵測
  if (net_boxes){
      run_face_recognition(image_matrix, net_boxes);  //執行人臉辨識
      dl_lib_free(net_boxes->score);
      dl_lib_free(net_boxes->box);
      dl_lib_free(net_boxes->landmark);
      dl_lib_free(net_boxes);                                
      net_boxes = NULL;
  }
  dl_matrix3du_free(image_matrix);
}

//人臉辨識函式
static int run_face_recognition(dl_matrix3du_t *image_matrix, box_array_t *net_boxes){
    dl_matrix3du_t *aligned_face = NULL;
    int matched_id = 0;

    aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
    if(!aligned_face){
        Serial.println("Could not allocate face recognition buffer");
        return matched_id;
    }
    if (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK){
        if (is_enrolling == 1){  //註冊人臉
            int8_t left_sample_face = enroll_face(&id_list, aligned_face);

            if(left_sample_face == (ENROLL_CONFIRM_TIMES - 1)){
                enroll_id = id_list.tail;
                Serial.printf("Enrolling Face ID: %d\n", enroll_id);
            }
            Serial.printf("Enrolling Face ID: %d sample %d\n", enroll_id, ENROLL_CONFIRM_TIMES - left_sample_face);
            rgb_printf(image_matrix, FACE_COLOR_CYAN, "ID[%u] Sample[%u]", id_list.tail, ENROLL_CONFIRM_TIMES - left_sample_face);
            if (left_sample_face == 0){
                is_enrolling = 0;
                enroll_id = id_list.tail;
                //Serial.printf("Enrolled Face ID: %d\n", enroll_id);
            }
        } else {  //人臉辨識
            matched_id = recognize_face(&id_list, aligned_face);
            if (matched_id >= 0) {  //若辨識為已註冊之人臉
                Serial.printf("Match Face ID: %u\n", matched_id);
                int name_length = sizeof(recognize_face_matched_name) / sizeof(recognize_face_matched_name[0]);
                if (matched_id<name_length) {
                  Serial.printf("Match Face Name: %s\n", recognize_face_matched_name[matched_id]);
                  rgb_printf(image_matrix, FACE_COLOR_GREEN, "[%u] %s", matched_id, recognize_face_matched_name[matched_id]);
                } else {
                  Serial.printf("Match Face Name: No name");
                  rgb_printf(image_matrix, FACE_COLOR_GREEN, "[%u] No Name", matched_id);
                }
                FaceMatched(matched_id);  //辨識到註冊人臉執行指令控制
            } else {  //若辨識為未註冊之人臉
                Serial.println("No Match Found");
                rgb_print(image_matrix, FACE_COLOR_RED, "Intruder Alert!");
                matched_id = -1;
                FaceNoMatched();  //辨識為陌生人臉執行指令控制
            }
        }
    } else {  //若偵測出人臉，但無法進行識別
        Serial.println("Face Not Aligned");
        rgb_print(image_matrix, FACE_COLOR_YELLOW, "Human Detected");
    }
    Serial.println();
    
    dl_matrix3du_free(aligned_face);
    return matched_id;
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
    bool detected = false;
    int face_id = 0;
    if(!detection_enabled || fb->width > 400){
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

    box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);  //執行人臉偵測取得臉框數據

    if (net_boxes){
        //Serial.println("faces = " + String(net_boxes->len));  //偵測到的人臉數
        detected = true;
        if(recognition_enabled){
            face_id = run_face_recognition(image_matrix, net_boxes);  //執行人臉辨識
        }
        draw_face_boxes(image_matrix, net_boxes, face_id);  //繪製人臉方框
        dl_lib_free(net_boxes->score);
        dl_lib_free(net_boxes->box);
        dl_lib_free(net_boxes->landmark);
        dl_lib_free(net_boxes);                                
        net_boxes = NULL;
    }

    jpg_chunking_t jchunk = {req, 0};
    s = fmt2jpg_cb(out_buf, out_len, out_width, out_height, PIXFORMAT_RGB888, 90, jpg_encode_stream, &jchunk);
    dl_matrix3du_free(image_matrix);
    if(!s){
        Serial.println("JPEG compression failed");
        return ESP_FAIL;
    }

    int64_t fr_end = esp_timer_get_time();
    Serial.printf("FACE: %uB %ums %s%d\n", (uint32_t)(jchunk.len), (uint32_t)((fr_end - fr_start)/1000), detected?"DETECTED ":"", face_id);
    return res;
}

//影像串流
static esp_err_t stream_handler(httpd_req_t *req){
    streamState = true;
  
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];
    dl_matrix3du_t *image_matrix = NULL;
    bool detected = false;
    int face_id = 0;
    int64_t fr_start = 0;
    int64_t fr_ready = 0;
    int64_t fr_face = 0;
    int64_t fr_recognize = 0;
    int64_t fr_encode = 0;

    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        streamState = false;
        return res;
    }
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    while(true){
        detected = false;
        face_id = 0;
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            fr_start = esp_timer_get_time();
            fr_ready = fr_start;
            fr_face = fr_start;
            fr_encode = fr_start;
            fr_recognize = fr_start;
            if(!detection_enabled || fb->width > 400){
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
                        box_array_t *net_boxes = NULL;
                        if(detection_enabled){
                            net_boxes = face_detect(image_matrix, &mtmn_config);  //執行人臉偵測取得臉框數據
                        }
                        fr_face = esp_timer_get_time();
                        fr_recognize = fr_face;
                        if (net_boxes || fb->format != PIXFORMAT_JPEG){
                            if(net_boxes){
                                //Serial.println("faces = " + String(net_boxes->len));  //偵測到的人臉數
                                detected = true;
                                if(recognition_enabled){
                                    face_id = run_face_recognition(image_matrix, net_boxes);  //執行人臉辨識
                                }
                                fr_recognize = esp_timer_get_time();
                                draw_face_boxes(image_matrix, net_boxes, face_id);  //繪製人臉方框
                                dl_lib_free(net_boxes->score);
                                dl_lib_free(net_boxes->box);
                                dl_lib_free(net_boxes->landmark);
                                dl_lib_free(net_boxes);                                
                                net_boxes = NULL;
                            }
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
                        fr_encode = esp_timer_get_time();
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
            streamState = false;
            break;
        }
        int64_t fr_end = esp_timer_get_time();

        int64_t ready_time = (fr_ready - fr_start)/1000;
        int64_t face_time = (fr_face - fr_ready)/1000;
        int64_t recognize_time = (fr_recognize - fr_face)/1000;
        int64_t encode_time = (fr_encode - fr_recognize)/1000;
        int64_t process_time = (fr_encode - fr_start)/1000;
        
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        /*
        Serial.printf("MJPG: %uB %ums (%.1ffps), %u+%u+%u+%u=%u %s%d\n",
            (uint32_t)(_jpg_buf_len),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
            (uint32_t)ready_time, (uint32_t)face_time, (uint32_t)recognize_time, (uint32_t)encode_time, (uint32_t)process_time,
            (detected)?"DETECTED ":"", face_id
        );
        */
    }

    last_frame = 0;
    streamState = false;
    return res;
}

//指令參數控制
static esp_err_t cmd_handler(httpd_req_t *req){
    char*  buf;  //存取網址後帶的參數字串
    size_t buf_len;
    char variable[128] = {0,};  //存取參數var值，可修改陣列長度。
    char value[128] = {0,};  //存取參數val值，可修改陣列長度。
    String myCmd = "";
    
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {  //拆解網址後帶的參數字串
          if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
            httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
          } 
          else {
            myCmd = String(buf);
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
      else if (cmd=="facename") {  //設定人名
        recognize_face_matched_name[P1.toInt()] = P2;
        
        int name_length = sizeof(recognize_face_matched_name) / sizeof(recognize_face_matched_name[0]);
        Feedback="<table style=\"color:white\"><tr><td>matched_id</td><td>name</td></tr>";
        for (int i=0;i<name_length;i++) {
          Feedback+="<tr><td>"+String(i)+"</td><td>"+recognize_face_matched_name[i]+"</td></tr>";
        }
        Feedback+="</table>";
      }  
      else if (cmd=="clearface") {  //刪除註冊人臉
        delete_face(&id_list);
        face_id_init(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
        
        int name_length = sizeof(recognize_face_matched_name) / sizeof(recognize_face_matched_name[0]);
        Feedback="<table style=\"color:white\"><tr><td>matched_id</td><td>name</td></tr>";
        for (int i=0;i<name_length;i++) {
          Feedback+="<tr><td>"+String(i)+"</td><td>"+recognize_face_matched_name[i]+"</td></tr>";
        }
        Feedback+="</table>";
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
      int val = atoi(value);  //將參數val值轉換為整數，原為char格式
      sensor_t * s = esp_camera_sensor_get();
      int res = 0;
  
      //官方指令區塊，也可在此自訂指令  http://192.168.xxx.xxx/control?var=xxx&val=xxx
      if(!strcmp(variable, "framesize")) {  //解析度
          if(s->pixformat == PIXFORMAT_JPEG) res = s->set_framesize(s, (framesize_t)val);
      }
      else if(!strcmp(variable, "quality")) res = s->set_quality(s, val);  //畫質
      else if(!strcmp(variable, "contrast")) res = s->set_contrast(s, val);  //對比
      else if(!strcmp(variable, "brightness")) res = s->set_brightness(s, val);  //亮度
      else if(!strcmp(variable, "face_detect")) {  //人臉偵測
          detection_enabled = val;
          if(!detection_enabled) {
              recognition_enabled = 0;
          }
      }
      else if(!strcmp(variable, "face_enroll")) is_enrolling = val;  //人臉註冊
      else if(!strcmp(variable, "face_recognize")) {  //人臉辨識
          recognition_enabled = val;
          if(recognition_enabled){
              detection_enabled = val;
          }
      }
      else if(!strcmp(variable, "flash")) {  //閃光燈
        ledcWrite(4,val);
        flash_value = val;
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
    p+=sprintf(p, "\"face_detect\":%u,", detection_enabled);
    p+=sprintf(p, "\"face_enroll\":%u,", is_enrolling);
    p+=sprintf(p, "\"face_recognize\":%u,", recognition_enabled);
    p+=sprintf(p, "\"flash\":%u", flash_value);
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
                display: block
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
    </head>
    <body>
        <figure>
            <div id="stream-container" class="image-container hidden">
                <div class="close" id="close-stream">×</div>
                <img id="stream" src="" crossorigin="anonymous">
            </div>
        </figure>    
        <section class="main">
            <div id="logo">
                <label for="nav-toggle-cb" id="nav-toggle">&#9776;&nbsp;&nbsp;Toggle OV2640 settings</label>
            </div>
            <div id="content">
                <div id="sidebar">
                    <input type="checkbox" id="nav-toggle-cb" checked="checked">
                    <nav id="menu">
                        <section id="buttons">
                            <button id="get-still">Get Still</button>
                            <button id="toggle-stream">Start Stream</button>
                            <button id="face_enroll" class="disabled" disabled="disabled">Enroll Face</button>
                        </section> 
                        <div class="input-group" id="face_detect-group">
                            <label for="face_detect">Face Detection</label>
                            <div class="switch">
                                <input id="face_detect" type="checkbox" class="default-action">
                                <label class="slider" for="face_detect"></label>
                            </div>
                        </div>
                        <div class="input-group" id="face_recognize-group">
                            <label for="face_recognize">Face Recognition</label>
                            <div class="switch">
                                <input id="face_recognize" type="checkbox" class="default-action">
                                <label class="slider" for="face_recognize"></label>
                            </div>
                        </div>                        
                        <div class="input-group" id="flash-group">
                            <label for="flash">Flash</label>
                            <div class="range-min">0</div>
                            <input type="range" id="flash" min="0" max="255" value="0" onchange="try{fetch(document.location.origin+'/control?var=flash&val='+this.value);}catch(e){}" class="default-action">
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
                                <option value="5" selected="selected">CIF(400x296)</option>
                                <option value="4">QVGA(320x240)</option>
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
                        <div class="input-group" id="facename-group">
                            <label for="facename">Face Name</label>
                            <select id="faceid" class="default-action">
                              <option value="0">0</option>
                              <option value="1">1</option>
                              <option value="2">2</option>
                              <option value="3">3</option>
                              <option value="4">4</option>
                              <option value="5">5</option>
                              <option value="6">6</option>
                            </select>
                            <input type="text" id="facename" size="6" style="height:16px">
                            <button onclick="var ifr = document.getElementById('ifr');var ifrlab = document.getElementById('ifrlab');ifr.style.display='block';ifrlab.style.display='block';ifr.src=document.location.origin+'/control?facename='+document.getElementById('faceid').value+';'+document.getElementById('facename').value;">Set</button>
                        </div>    
                        <div class="input-group" id="facename-group">
                            <label for="facename"></label>
                            <button onclick="var ifr = document.getElementById('ifr');ifr.src=document.location.origin+'/control?clearface'">Clear Face</button>
                        </div> 
                        <div class="input-group" id="contrast-group">
                            <label id="ifrlab" style="display:none;" for="ifr">Name List</label>
                            <iframe id="ifr" style="display:none;width:170px"></iframe>
                        </div>                        
                    </nav>
                </div>
            </div>
        </section>
        
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
                } else if(el.id === "face_recognize"){
                  value ? enable(enrollButton) : disable(enrollButton)
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
            const enrollButton = document.getElementById('face_enroll')
            const closeButton = document.getElementById('close-stream')
          
            const stopStream = () => {
              window.stop();
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
          
            enrollButton.onclick = () => {
              updateConfig(enrollButton)
            }
          
            // Attach default on change action
            document
              .querySelectorAll('.default-action')
              .forEach(el => {
                el.onchange = () => updateConfig(el)
              })
          
            // Custom actions
          
            // Detection and framesize
            const detect = document.getElementById('face_detect')
            const recognize = document.getElementById('face_recognize')
            const framesize = document.getElementById('framesize')
          
            framesize.onchange = () => {
              updateConfig(framesize)
              if (framesize.value > 5) {
                updateValue(detect, false)
                updateValue(recognize, false)
              }
            }
          
            detect.onchange = () => {
              if (framesize.value > 5) {
                alert("Please select CIF or lower resolution before enabling this feature!");
                updateValue(detect, false)
                return;
              }
              updateConfig(detect)
              if (!detect.checked) {
                disable(enrollButton)
                updateValue(recognize, false)
              }
            }
          
            recognize.onchange = () => {
              if (framesize.value > 5) {
                alert("Please select CIF or lower resolution before enabling this feature!");
                updateValue(recognize, false)
                return;
              }
              updateConfig(recognize)
              if (recognize.checked) {
                enable(enrollButton)
                updateValue(detect, true)
              } else {
                disable(enrollButton)
              }
            }
          })
        </script>
    </body>
</html>)rawliteral";

//網頁首頁  http://192.168.xxx.xxx
static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");     
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

void startCameraServer(){
    //https://github.com/espressif/esp-idf/blob/master/components/esp_http_server/include/esp_http_server.h
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();  //可在HTTPD_DEFAULT_CONFIG()中設定Server Port

    //可自訂網址路徑對應執行的函式
    httpd_uri_t index_uri = {
        .uri       = "/",             //http://192.168.xxx.xxx/
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
    
    Serial.printf("Starting web server on port: '%d'\n", config.server_port);  //TCP Port
    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        //註冊自訂網址路徑對應執行的函式
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
        httpd_register_uri_handler(camera_httpd, &status_uri);
        httpd_register_uri_handler(camera_httpd, &capture_uri);
    }

    config.server_port += 1;  //Stream Port
    config.ctrl_port += 1;  //UDP Port
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

String LineNotify_http_get(String token, String message) {
  message.replace("%","%25");  
  message.replace(" ","%20");
  message.replace("&","%20");
  message.replace("#","%20");
  //message.replace("\'","%27");
  message.replace("\"","%22");
  message.replace("\n","%0D%0A");
  
  http.begin("http://linenotify.com/notify.php?token="+token+"&message="+message);
  int httpCode = http.GET();
  /*
  if(httpCode > 0) {
      if(httpCode == 200) 
        Serial.println(http.getString());
  } else 
      Serial.println("Connection Error!");
  */
}
