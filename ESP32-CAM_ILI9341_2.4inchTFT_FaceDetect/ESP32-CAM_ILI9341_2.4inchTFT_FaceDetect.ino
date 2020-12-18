/*
ESP32-CAM 2.4 inch TFT LCD Display Module (ILI9341, SPI, 240x320)
Face detect
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-12-19 02:00
https://www.facebook.com/francefu

Circuit
TFT_MOSI --> IO13
TFT_MISO --> IO12
TFT_SCLK --> IO14
TFT_CS   --> IO15
TFT_DC   --> IO2
TFT_RST  --> IO16 (Don't use IO4)
TFT_VCC  --> 3.3V
TFT_LED  --> 3.3V
TFT_GND  --> GND

Introduction
http://www.lcdwiki.com/2.8inch_SPI_Module_ILI9341_SKU:MSP2807

Refer to
http://fabacademy.org/2020/labs/seoulinnovation/students/seokmin-park/week9.html

Libraries：
https://www.arduinolibraries.info/libraries/adafruit-gfx-library
https://www.arduinolibraries.info/libraries/adafruit-ili9341
*/

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "fd_forward.h"          //人臉偵測函式
#include "SPI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

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

#define FACE_COLOR_WHITE  0x00FFFFFF
#define FACE_COLOR_BLACK  0x00000000
#define FACE_COLOR_RED    0x000000FF
#define FACE_COLOR_GREEN  0x0000FF00
#define FACE_COLOR_BLUE   0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN   (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)

#define TFT_WHITE   ILI9341_WHITE
#define TFT_BLACK   ILI9341_BLACK
#define TFT_RED     ILI9341_RED
#define TFT_ORANGE  ILI9341_ORANGE
#define TFT_YELLOW  ILI9341_YELLOW
#define TFT_GREEN   ILI9341_GREEN
#define TFT_CYAN    ILI9341_CYAN
#define TFT_BLUE    ILI9341_BLUE
#define TFT_MAGENTA ILI9341_MAGENTA

#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  16

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST, TFT_MISO);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialise the TFT
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);

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
  config.pixel_format = PIXFORMAT_RGB565;
  config.frame_size = FRAMESIZE_QQVGA;   //160x120
  config.jpeg_quality = 12;
  config.fb_count = 1; 

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void loop() {
  dl_matrix3du_t *image_matrix = NULL;
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();

  if (!fb) {
      Serial.println("Camera capture failed");
  } else {
      image_matrix = dl_matrix3du_alloc(1, fb->width, fb->height, 3);  //分配內部記憶體
      if (!image_matrix) {
          Serial.println("dl_matrix3du_alloc failed");
      } else {
          //臉部偵測參數設定  https://github.com/espressif/esp-face/blob/master/face_detection/README.md
          static mtmn_config_t mtmn_config = {0};
          mtmn_config.type = FAST;
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
          
          fmt2rgb888(fb->buf, fb->len, fb->format, image_matrix->item);  //影像格式轉換RGB格式
          box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);  //偵測人臉取得臉框數據

          uint8_t buffer;
          for( int i = 0; i < (160*120); i++) {   //160x120
            buffer = fb->buf[i*2];
            fb->buf[i*2] = fb->buf[i*2+1];
            fb->buf[i*2+1] = buffer;
          }    
                  
          tft.startWrite();
          drawRGBBitmap_fb(0,0,(uint16_t*)fb->buf, 160,120);   //160x120
            
          if (net_boxes){
            Serial.println("faces = " + String(net_boxes->len));  //偵測到的人臉數
            Serial.println();

            int16_t x_ = 0;
            int16_t y_ = 0;
            int16_t w_ = 0;
            int16_t h_ = 0;
            for (int i = 0; i < net_boxes->len; i++){  //列舉人臉位置與大小
                Serial.println("index = " + String(i));
                x_ = (int16_t)net_boxes->box[i].box_p[0];
                Serial.println("x = " + String(x_));
                y_ = (int16_t)net_boxes->box[i].box_p[1];
                Serial.println("y = " + String(y_));
                w_ = (int16_t)net_boxes->box[i].box_p[2] - x_ + 1;
                Serial.println("width = " + String(w_));
                h_ = (int16_t)net_boxes->box[i].box_p[3] - y_ + 1;
                Serial.println("height = " + String(h_));
                Serial.println();

                drawRGBBitmap_rect(x_, y_, w_, h_);                
            }

            free(net_boxes->score);
            free(net_boxes->box);
            free(net_boxes->landmark);
            free(net_boxes);
          }
          else {
            Serial.println("No Face");    //未偵測到的人臉
            Serial.println();          
          }

          tft.endWrite();
                      
          dl_matrix3du_free(image_matrix);
      }
 
      esp_camera_fb_return(fb); 
  }
  
  delay(1000);  
}

void drawRGBBitmap_fb(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h) {
  for (int16_t j = 0; j < h; j++, y++) {
    for (int16_t i = 0; i < w; i++) {
      tft.writePixel(x + i, y, pgm_read_word(&bitmap[j * w + i]));
    }
  }
}

void drawRGBBitmap_rect(int16_t x_, int16_t y_, int16_t w_, int16_t h_) {
  if (w_!=0) {
    tft.drawFastVLine((int16_t)x_, (int16_t)y_, (int16_t)h_, (int16_t)FACE_COLOR_GREEN);
    tft.drawFastVLine((int16_t)(x_+w_), (int16_t)y_, (int16_t)h_, (int16_t)FACE_COLOR_GREEN);
    tft.drawFastHLine((int16_t)x_, (int16_t)y_, (int16_t)w_, (int16_t)FACE_COLOR_GREEN);
    tft.drawFastHLine((int16_t)x_, (int16_t)(y_+h_), (int16_t)w_, (int16_t)FACE_COLOR_GREEN);
  }
}
