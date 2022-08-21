/*
ESP32-CAM CameraWebServer (No face detection)

Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-8-20 11:00
https://www.facebook.com/francefu

Face recognition works well in v1.0.6.

AP IP: 192.168.4.1
http://192.168.xxx.xxx             //網頁首頁管理介面
http://192.168.xxx.xxx:81/stream   //取得串流影像        網頁語法 <img src="http://192.168.xxx.xxx:81/stream">
http://192.168.xxx.xxx/status      //取得視訊參數值      網頁語法 <img src="http://192.168.xxx.xxx/status">
http://192.168.xxx.xxx/capture     //取得影像           網頁語法 <img src="http://192.168.xxx.xxx/capture">
http://192.168.xxx.xxx/bmp         //取得BMP影像        網頁語法 <img src="http://192.168.xxx.xxx/bmp">

http://192.168.xxx.xxx/reg?reg=reg&mask=mask&val=value
http://192.168.xxx.xxx/greg?reg=reg&mask=mask
http://192.168.xxx.xxx/xclk?xclk=xclk
http://192.168.xxx.xxx/pll?bypass=bypass&mul=mul&sys=sys&root=root&pre=pre&seld5=seld5&pclken=pclken&pclk=pclk
http://192.168.xxx.xxx/resolution?sx=start_x&sy=start_y&ex=end_x&ey=end_y&offx=offset_x&offy=offset_y&tx=total_x&ty=total_y&ox=output_x&oy=output_y&scale=scaling&binning=binning


官方指令格式  http://192.168.xxx.xxx/control?var=*****&val=*****
自訂指令格式  http://192.168.xxx.xxx/control?cmd=p1;p2;p3;p4;p5;p6;p7;p8;p9

http://192.168.xxx.xxx/control?var=flash&val=value          //閃光燈 value= 0~255
http://192.168.xxx.xxx/control?var=framesize&val=value      //解析度 value = 21->QSXGA(2560x1920), 20->P FHD(1080x1920), 19->WQXGA(2560x1600), 18->QHD(2560x1440), 17->QXGA(2048x1564), 16->P 3MP(864x1564), 15->P HD(720x1280), 14->FHD(1920x1080), 13->UXGA(1600x1200), 12->SXGA(1280x1024), 11->HD(1280x720), 10->XGA(1024x768), 9->SVGA(800x600), 8->VGA(640x480), 7->HVGA(480x320), 6->CIF(400x296), 5->QVGA(320x240), 4->240x240, 3->HQVGA(240x176), 2->QCIF(176x144), 1->QQVGA(160x120), 0->96x96
http://192.168.xxx.xxx/control?var=quality&val=value        //畫質 value = 4 ~ 63
http://192.168.xxx.xxx/control?var=brightness&val=value     //亮度 value = -3 ~ 3
http://192.168.xxx.xxx/control?var=contrast&val=value       //對比 value = -3 ~ 3
http://192.168.xxx.xxx/control?var=saturation&val=value     //飽和度 value = -4 ~ 4
http://192.168.xxx.xxx/control?var=sharpness&val=value      //清晰度 value = -3 ~ 3
http://192.168.xxx.xxx/control?var=denoise&val=value        //降噪 0 ~ 8
http://192.168.xxx.xxx/control?var=ae_level&val=value       //自動曝光層級 value = -5 ~ 5 
http://192.168.xxx.xxx/control?var=gainceiling&val=value    //自動增益上限(開啟時) value = 0 ~ 511
http://192.168.xxx.xxx/control?var=special_effect&val=value //特效 value = 0 ~ 6
http://192.168.xxx.xxx/control?var=awb&val=value            //白平衡 value = 0 or 1 
http://192.168.xxx.xxx/control?var=dcw&val=value            //使用自訂影像尺寸 value = 0 or 1 
http://192.168.xxx.xxx/control?var=awb_gain&val=value       //自動白平衡增益 value = 0 or 1 
http://192.168.xxx.xxx/control?var=wb_mode&val=value        //白平衡模式 value = 0自動，1晴天，2陰天，3日光燈，4鎢絲燈
http://192.168.xxx.xxx/control?var=aec&val=value            //自動曝光感測器 value = 0 or 1 
http://192.168.xxx.xxx/control?var=aec_value&val=value      //曝光值 value = 0 ~ 1920
http://192.168.xxx.xxx/control?var=aec2&val=value           //自動曝光控制 value = 0 or 1 
http://192.168.xxx.xxx/control?var=agc&val=value            //自動增益控制 value = 0 or 1 
http://192.168.xxx.xxx/control?var=agc_gain&val=value       //自動增益(關閉時) value = 0 ~ 30
http://192.168.xxx.xxx/control?var=raw_gma&val=value        //原始伽瑪 value = 0 or 1 
http://192.168.xxx.xxx/control?var=lenc&val=value           //鏡頭校正 value = 0 or 1 
http://192.168.xxx.xxx/control?var=hmirror&val=value        //水平鏡像 value = 0 or 1 
http://192.168.xxx.xxx/control?var=vflip&val=value          //垂直翻轉 value = 0 or 1 
http://192.168.xxx.xxx/control?var=bpc&val=value            //黑色像素校正 value = 0 or 1
http://192.168.xxx.xxx/control?var=wpc&val=value            //白色像素校正 value = 0 or 1 
http://192.168.xxx.xxx/control?var=colorbar&val=value       //顏色條畫面 value = 0 or 1

視訊參數說明
https://heyrick.eu/blog/index.php?diary=20210418
*/

const char* ssid = "teacher";
const char* password = "87654321";
const char* apssid = "esp32-cam";
const char* appassword = "12345678";

#include <WiFi.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
//#define CAMERA_MODEL_M5STACK_UNITCAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

typedef struct
{
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

void startCameraServer();

String Feedback="",Command="",cmd="",p1="",p2="",p3="",p4="",p5="",p6="",p7="",p8="",p9="";
byte receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;

void executeCommand() {
  if (cmd=="ip") {
    Feedback="AP IP: "+WiFi.softAPIP().toString();
    Feedback+="<br>";
    Feedback+="STA IP: "+WiFi.localIP().toString();
  } else if (cmd=="mac") {
    Feedback="STA MAC: "+WiFi.macAddress();
  } else if (cmd=="restart") {
    ESP.restart();
  } else if (cmd=="resetwifi") {
    for (int i=0;i<2;i++) {
      WiFi.begin(p1.c_str(), p2.c_str());
      Serial.print("Connecting to ");
      Serial.println(p1);
      long int StartTime=millis();
      while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          if ((StartTime+5000) < millis()) break;
      }
      Serial.println("");
      Serial.println("STAIP: "+WiFi.localIP().toString());
      Feedback="STAIP: "+WiFi.localIP().toString();
      if (WiFi.status() == WL_CONNECTED) {
        WiFi.softAP((WiFi.localIP().toString()+"_"+p1).c_str(), p2.c_str());
        break;
      }
    }
  } else if (cmd=="print") {
    Serial.print(p1);
  } else if (cmd=="println") {
    Serial.println(p1);
  } else {
    Feedback = String("Command is not defined.");
  }
}

void getCommand(char c) {
  if (c=='?') receiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) receiveState=0;

  if (receiveState==1) {
    Command=Command+String(c);

    if (c=='=') cmdState=0;
    if (c==';') pState++;

    if ((cmdState==1)&&((c!='?')||(questionState==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(pState==1)&&((c!='=')||(equalState==1))) p1=p1+String(c);
    if ((cmdState==0)&&(pState==2)&&(c!=';')) p2=p2+String(c);
    if ((cmdState==0)&&(pState==3)&&(c!=';')) p3=p3+String(c);
    if ((cmdState==0)&&(pState==4)&&(c!=';')) p4=p4+String(c);
    if ((cmdState==0)&&(pState==5)&&(c!=';')) p5=p5+String(c);
    if ((cmdState==0)&&(pState==6)&&(c!=';')) p6=p6+String(c);
    if ((cmdState==0)&&(pState==7)&&(c!=';')) p7=p7+String(c);
    if ((cmdState==0)&&(pState==8)&&(c!=';')) p8=p8+String(c);
    if ((cmdState==0)&&(pState>=9)&&((c!=';')||(semicolonState==1))) p9=p9+String(c);

    if (c=='?') questionState=1;
    if (c=='=') equalState=1;
    if ((pState>=9)&&(c==';')) semicolonState=1;
  }
}
  
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
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
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);

  WiFi.mode(WIFI_AP_STA);
  
  for (int i=0;i<2;i++) {
    WiFi.begin(ssid, password);    //執行網路連線
  
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
      WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
      Serial.println("");
      Serial.println("STAIP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");
      break;
    }
  }
  
  startCameraServer();
}

void loop() {
  delay(10000);
}

static esp_err_t bmp_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    uint64_t fr_start = esp_timer_get_time();
    fb = esp_camera_fb_get();
    if (!fb)
    {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/x-windows-bmp");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.bmp");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char ts[32];
    snprintf(ts, 32, "%ld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);
    httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);

    uint8_t * buf = NULL;
    size_t buf_len = 0;
    bool converted = frame2bmp(fb, &buf, &buf_len);
    esp_camera_fb_return(fb);
    if(!converted){
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    res = httpd_resp_send(req, (const char *)buf, buf_len);
    free(buf);
    return res;
}

static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len)
{
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if (!index)
    {
        j->len = 0;
    }
    if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK)
    {
        return 0;
    }
    j->len += len;
    return len;
}

static esp_err_t capture_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;

    fb = esp_camera_fb_get();

    if (!fb)
    {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

      size_t fb_len = 0;
      if (fb->format == PIXFORMAT_JPEG)
      {
          fb_len = fb->len;
          res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
      }
      else
      {
          jpg_chunking_t jchunk = {req, 0};
          res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
          httpd_resp_send_chunk(req, NULL, 0);
          fb_len = jchunk.len;
      }
      esp_camera_fb_return(fb);
      return res;
}

static esp_err_t stream_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    struct timeval _timestamp;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[128];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
    {
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");
    
    while (true)
    {
        fb = esp_camera_fb_get();
        if (!fb)
        {
            res = ESP_FAIL;
        }
        else
        {
            if (fb->format != PIXFORMAT_JPEG)
            {
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if (!jpeg_converted)
                {
                    res = ESP_FAIL;
                }
            }
            else
            {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if (res == ESP_OK)
        {
            size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK)
        {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (fb)
        {
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        }
        else if (_jpg_buf)
        {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK)
        {
            break;
        }
    }
    return res;
}

static esp_err_t parse_get(httpd_req_t *req, char **obuf)
{
    char *buf = NULL;
    size_t buf_len = 0;

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char *)malloc(buf_len);
        if (!buf) {
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            *obuf = buf;
            return ESP_OK;
        }
        free(buf);
    }
    httpd_resp_send_404(req);
    return ESP_FAIL;
}

static esp_err_t cmd_handler(httpd_req_t *req)
{
    char *buf = NULL;
    char variable[128];
    char value[128];
    String myCmd = "";
    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }
    if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) != ESP_OK || httpd_query_key_value(buf, "val", value, sizeof(value)) != ESP_OK) {
        myCmd = String(buf);
        free(buf);
        if (myCmd!="") {
          Feedback="";Command="";cmd="";p1="";p2="";p3="";p4="";p5="";p6="";p7="";p8="";p9="";
          receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;
          sensor_t * s = esp_camera_sensor_get();
          myCmd = "?"+myCmd;
          for (int i=0;i<myCmd.length();i++) {
            getCommand(char(myCmd.charAt(i)));
          }
          if (cmd=="ip") {
            Feedback="AP IP: "+WiFi.softAPIP().toString();
            Feedback+="<br>";
            Feedback+="STA IP: "+WiFi.localIP().toString();
          }
          else if (cmd=="mac")
            Feedback="STA MAC: "+WiFi.macAddress();
          else if (cmd=="restart")
            ESP.restart();
          else if (cmd=="resetwifi") {
            for (int i=0;i<2;i++) {
              WiFi.begin(p1.c_str(), p2.c_str());
              Serial.print("Connecting to ");
              Serial.println(p1);
              long int StartTime=millis();
              while (WiFi.status() != WL_CONNECTED) {
                  delay(500);
                  if ((StartTime+5000) < millis()) break;
              }
              Serial.println("");
              Serial.println("STAIP: "+WiFi.localIP().toString());
              Feedback=WiFi.localIP().toString();
              if (WiFi.status() == WL_CONNECTED) {
                WiFi.softAP((WiFi.localIP().toString()+"_"+p1).c_str(), p2.c_str());
                break;
              }
            }
          }
          else if (cmd=="print")
            Serial.print(p1);
          else if (cmd=="println")
            Serial.println(p1);
          else if (cmd=="delay")
            delay(p1.toInt());
          else if (cmd=="framesize") {
              if (s->pixformat == PIXFORMAT_JPEG) {
                  s->set_framesize(s, (framesize_t)p1.toInt());
              }
          } 
          else if (cmd=="quality")
              s->set_quality(s, p1.toInt());
          else if (cmd=="contrast")
              s->set_contrast(s, p1.toInt());
          else if (cmd=="brightness")
              s->set_brightness(s, p1.toInt());
          else if (cmd=="saturation")
              s->set_saturation(s, p1.toInt());
          else if (cmd=="gainceiling")
              s->set_gainceiling(s, (gainceiling_t)p1.toInt());
          else if (cmd=="colorbar")
              s->set_colorbar(s, p1.toInt());
          else if (cmd=="awb")
              s->set_whitebal(s, p1.toInt());
          else if (cmd=="agc")
              s->set_gain_ctrl(s, p1.toInt());
          else if (cmd=="aec")
              s->set_exposure_ctrl(s, p1.toInt());
          else if (cmd=="hmirror")
              s->set_hmirror(s, p1.toInt());
          else if (cmd=="vflip")
              s->set_vflip(s, p1.toInt());
          else if (cmd=="awb_gain")
              s->set_awb_gain(s, p1.toInt());
          else if (cmd=="agc_gain")
              s->set_agc_gain(s, p1.toInt());
          else if (cmd=="aec_p1.toInt()ue")
              s->set_aec_value(s, p1.toInt());
          else if (cmd=="aec2")
              s->set_aec2(s, p1.toInt());
          else if (cmd=="dcw")
              s->set_dcw(s, p1.toInt());
          else if (cmd=="bpc")
              s->set_bpc(s, p1.toInt());
          else if (cmd=="wpc")
              s->set_wpc(s, p1.toInt());
          else if (cmd=="raw_gma")
              s->set_raw_gma(s, p1.toInt());
          else if (cmd=="lenc")
              s->set_lenc(s, p1.toInt());
          else if (cmd=="special_effect")
              s->set_special_effect(s, p1.toInt());
          else if (cmd=="wb_mode")
              s->set_wb_mode(s, p1.toInt());
          else if (cmd=="ae_level")
              s->set_ae_level(s, p1.toInt());
          else {
              Feedback = String("Command is not defined.");
          }
          const char *resp = Feedback.c_str();
          httpd_resp_set_type(req, "text/html");
          httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
          return httpd_resp_send(req, resp, strlen(resp));
        } else {
          httpd_resp_send_404(req);
          return ESP_FAIL;
        }
    }

    free(buf);

    int val = atoi(value);
    sensor_t *s = esp_camera_sensor_get();
    int res = 0;

    if (!strcmp(variable, "framesize")) {
        if (s->pixformat == PIXFORMAT_JPEG) {
            res = s->set_framesize(s, (framesize_t)val);
        }
    }
    else if (!strcmp(variable, "quality"))
        res = s->set_quality(s, val);
    else if (!strcmp(variable, "contrast"))
        res = s->set_contrast(s, val);
    else if (!strcmp(variable, "brightness"))
        res = s->set_brightness(s, val);
    else if (!strcmp(variable, "saturation"))
        res = s->set_saturation(s, val);
    else if (!strcmp(variable, "gainceiling"))
        res = s->set_gainceiling(s, (gainceiling_t)val);
    else if (!strcmp(variable, "colorbar"))
        res = s->set_colorbar(s, val);
    else if (!strcmp(variable, "awb"))
        res = s->set_whitebal(s, val);
    else if (!strcmp(variable, "agc"))
        res = s->set_gain_ctrl(s, val);
    else if (!strcmp(variable, "aec"))
        res = s->set_exposure_ctrl(s, val);
    else if (!strcmp(variable, "hmirror"))
        res = s->set_hmirror(s, val);
    else if (!strcmp(variable, "vflip"))
        res = s->set_vflip(s, val);
    else if (!strcmp(variable, "awb_gain"))
        res = s->set_awb_gain(s, val);
    else if (!strcmp(variable, "agc_gain"))
        res = s->set_agc_gain(s, val);
    else if (!strcmp(variable, "aec_value"))
        res = s->set_aec_value(s, val);
    else if (!strcmp(variable, "aec2"))
        res = s->set_aec2(s, val);
    else if (!strcmp(variable, "dcw"))
        res = s->set_dcw(s, val);
    else if (!strcmp(variable, "bpc"))
        res = s->set_bpc(s, val);
    else if (!strcmp(variable, "wpc"))
        res = s->set_wpc(s, val);
    else if (!strcmp(variable, "raw_gma"))
        res = s->set_raw_gma(s, val);
    else if (!strcmp(variable, "lenc"))
        res = s->set_lenc(s, val);
    else if (!strcmp(variable, "special_effect"))
        res = s->set_special_effect(s, val);
    else if (!strcmp(variable, "wb_mode"))
        res = s->set_wb_mode(s, val);
    else if (!strcmp(variable, "ae_level"))
        res = s->set_ae_level(s, val);
    else if (!strcmp(variable, "print"))
        Serial.print(val);
    else if (!strcmp(variable, "println"))
        Serial.println(val);           
    else {
        res = -1;
    }

    if (res < 0) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static int print_reg(char * p, sensor_t * s, uint16_t reg, uint32_t mask){
    return sprintf(p, "\"0x%x\":%u,", reg, s->get_reg(s, reg, mask));
}

static esp_err_t status_handler(httpd_req_t *req)
{
    static char json_response[1024];

    sensor_t *s = esp_camera_sensor_get();
    char *p = json_response;
    *p++ = '{';

    if(s->id.PID == OV5640_PID || s->id.PID == OV3660_PID){
        for(int reg = 0x3400; reg < 0x3406; reg+=2){
            p+=print_reg(p, s, reg, 0xFFF);//12 bit
        }
        p+=print_reg(p, s, 0x3406, 0xFF);

        p+=print_reg(p, s, 0x3500, 0xFFFF0);//16 bit
        p+=print_reg(p, s, 0x3503, 0xFF);
        p+=print_reg(p, s, 0x350a, 0x3FF);//10 bit
        p+=print_reg(p, s, 0x350c, 0xFFFF);//16 bit

        for(int reg = 0x5480; reg <= 0x5490; reg++){
            p+=print_reg(p, s, reg, 0xFF);
        }

        for(int reg = 0x5380; reg <= 0x538b; reg++){
            p+=print_reg(p, s, reg, 0xFF);
        }

        for(int reg = 0x5580; reg < 0x558a; reg++){
            p+=print_reg(p, s, reg, 0xFF);
        }
        p+=print_reg(p, s, 0x558a, 0x1FF);//9 bit
    } else if(s->id.PID == OV2640_PID){
        p+=print_reg(p, s, 0xd3, 0xFF);
        p+=print_reg(p, s, 0x111, 0xFF);
        p+=print_reg(p, s, 0x132, 0xFF);
    }

    p += sprintf(p, "\"xclk\":%u,", s->xclk_freq_hz / 1000000);
    p += sprintf(p, "\"pixformat\":%u,", s->pixformat);
    p += sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p += sprintf(p, "\"quality\":%u,", s->status.quality);
    p += sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p += sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p += sprintf(p, "\"saturation\":%d,", s->status.saturation);
    p += sprintf(p, "\"sharpness\":%d,", s->status.sharpness);
    p += sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
    p += sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
    p += sprintf(p, "\"awb\":%u,", s->status.awb);
    p += sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
    p += sprintf(p, "\"aec\":%u,", s->status.aec);
    p += sprintf(p, "\"aec2\":%u,", s->status.aec2);
    p += sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
    p += sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
    p += sprintf(p, "\"agc\":%u,", s->status.agc);
    p += sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
    p += sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
    p += sprintf(p, "\"bpc\":%u,", s->status.bpc);
    p += sprintf(p, "\"wpc\":%u,", s->status.wpc);
    p += sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
    p += sprintf(p, "\"lenc\":%u,", s->status.lenc);
    p += sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
    p += sprintf(p, "\"dcw\":%u,", s->status.dcw);
    p += sprintf(p, "\"colorbar\":%u", s->status.colorbar);
#ifdef CONFIG_LED_ILLUMINATOR_ENABLED
    p += sprintf(p, ",\"led_intensity\":%u", led_duty);
#else
    p += sprintf(p, ",\"led_intensity\":%d", -1);
#endif    
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

static esp_err_t xclk_handler(httpd_req_t *req)
{
    char *buf = NULL;
    char _xclk[32];

    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }
    if (httpd_query_key_value(buf, "xclk", _xclk, sizeof(_xclk)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    int xclk = atoi(_xclk);
    ESP_LOGI(TAG, "Set XCLK: %d MHz", xclk);

    sensor_t *s = esp_camera_sensor_get();
    int res = s->set_xclk(s, LEDC_TIMER_0, xclk);
    if (res) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t reg_handler(httpd_req_t *req)
{
    char *buf = NULL;
    char _reg[32];
    char _mask[32];
    char _val[32];

    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }
    if (httpd_query_key_value(buf, "reg", _reg, sizeof(_reg)) != ESP_OK ||
        httpd_query_key_value(buf, "mask", _mask, sizeof(_mask)) != ESP_OK ||
        httpd_query_key_value(buf, "val", _val, sizeof(_val)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    int reg = atoi(_reg);
    int mask = atoi(_mask);
    int val = atoi(_val);
    ESP_LOGI(TAG, "Set Register: reg: 0x%02x, mask: 0x%02x, value: 0x%02x", reg, mask, val);

    sensor_t *s = esp_camera_sensor_get();
    int res = s->set_reg(s, reg, mask, val);
    if (res) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t greg_handler(httpd_req_t *req)
{
    char *buf = NULL;
    char _reg[32];
    char _mask[32];

    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }
    if (httpd_query_key_value(buf, "reg", _reg, sizeof(_reg)) != ESP_OK ||
        httpd_query_key_value(buf, "mask", _mask, sizeof(_mask)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    int reg = atoi(_reg);
    int mask = atoi(_mask);
    sensor_t *s = esp_camera_sensor_get();
    int res = s->get_reg(s, reg, mask);
    if (res < 0) {
        return httpd_resp_send_500(req);
    }
    ESP_LOGI(TAG, "Get Register: reg: 0x%02x, mask: 0x%02x, value: 0x%02x", reg, mask, res);

    char buffer[20];
    const char * val = itoa(res, buffer, 10);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, val, strlen(val));
}

static int parse_get_var(char *buf, const char * key, int def)
{
    char _int[16];
    if(httpd_query_key_value(buf, key, _int, sizeof(_int)) != ESP_OK){
        return def;
    }
    return atoi(_int);
}

static esp_err_t pll_handler(httpd_req_t *req)
{
    char *buf = NULL;

    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }

    int bypass = parse_get_var(buf, "bypass", 0);
    int mul = parse_get_var(buf, "mul", 0);
    int sys = parse_get_var(buf, "sys", 0);
    int root = parse_get_var(buf, "root", 0);
    int pre = parse_get_var(buf, "pre", 0);
    int seld5 = parse_get_var(buf, "seld5", 0);
    int pclken = parse_get_var(buf, "pclken", 0);
    int pclk = parse_get_var(buf, "pclk", 0);
    free(buf);

    ESP_LOGI(TAG, "Set Pll: bypass: %d, mul: %d, sys: %d, root: %d, pre: %d, seld5: %d, pclken: %d, pclk: %d", bypass, mul, sys, root, pre, seld5, pclken, pclk);
    sensor_t *s = esp_camera_sensor_get();
    int res = s->set_pll(s, bypass, mul, sys, root, pre, seld5, pclken, pclk);
    if (res) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static esp_err_t win_handler(httpd_req_t *req)
{
    char *buf = NULL;

    if (parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }

    int startX = parse_get_var(buf, "sx", 0);
    int startY = parse_get_var(buf, "sy", 0);
    int endX = parse_get_var(buf, "ex", 0);
    int endY = parse_get_var(buf, "ey", 0);
    int offsetX = parse_get_var(buf, "offx", 0);
    int offsetY = parse_get_var(buf, "offy", 0);
    int totalX = parse_get_var(buf, "tx", 0);
    int totalY = parse_get_var(buf, "ty", 0);
    int outputX = parse_get_var(buf, "ox", 0);
    int outputY = parse_get_var(buf, "oy", 0);
    bool scale = parse_get_var(buf, "scale", 0) == 1;
    bool binning = parse_get_var(buf, "binning", 0) == 1;
    free(buf);

    ESP_LOGI(TAG, "Set Window: Start: %d %d, End: %d %d, Offset: %d %d, Total: %d %d, Output: %d %d, Scale: %u, Binning: %u", startX, startY, endX, endY, offsetX, offsetY, totalX, totalY, outputX, outputY, scale, binning);
    sensor_t *s = esp_camera_sensor_get();
    int res = s->set_res_raw(s, startX, startY, endX, endY, offsetX, offsetY, totalX, totalY, outputX, outputY, scale, binning);
    if (res) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

static const char PROGMEM INDEX_HTML_OV2640[] = R"rawliteral(
<!DOCTYPE html><html><body><script>setTimeout(function(){location.href="https://fustyles.github.io/webduino/CameraWebServer_OV2640.html?"+window.location.hostname;},1000);</script></body></html>
)rawliteral";

static const char PROGMEM INDEX_HTML_OV3660[] = R"rawliteral(
<!DOCTYPE html><html><body><script>setTimeout(function(){location.href="https://fustyles.github.io/webduino/CameraWebServer_OV3660.html?"+window.location.hostname;},1000);</script></body></html>
)rawliteral";

static const char PROGMEM INDEX_HTML_OV5640[] = R"rawliteral(
<!DOCTYPE html><html><body><script>setTimeout(function(){location.href="https://fustyles.github.io/webduino/CameraWebServer_OV5640.html?"+window.location.hostname;},1000);</script></body></html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL) {
        if (s->id.PID == OV3660_PID) {
            return httpd_resp_send(req, (const char *)INDEX_HTML_OV3660, strlen(INDEX_HTML_OV3660));
        } else if (s->id.PID == OV5640_PID) {
            return httpd_resp_send(req, (const char *)INDEX_HTML_OV5640, strlen(INDEX_HTML_OV5640));
        } else {
            return httpd_resp_send(req, (const char *)INDEX_HTML_OV2640, strlen(INDEX_HTML_OV2640));
        } 
    } else {
        return httpd_resp_send_500(req);
    }
}

void startCameraServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL};     

    httpd_uri_t cmd_uri = {
        .uri = "/control",
        .method = HTTP_GET,
        .handler = cmd_handler,
        .user_ctx = NULL};

    httpd_uri_t status_uri = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = NULL};   
        
    httpd_uri_t capture_uri = {
        .uri = "/capture",
        .method = HTTP_GET,
        .handler = capture_handler,
        .user_ctx = NULL};

    httpd_uri_t stream_uri = {
        .uri = "/stream",
        .method = HTTP_GET,
        .handler = stream_handler,
        .user_ctx = NULL};

    httpd_uri_t bmp_uri = {
        .uri = "/bmp",
        .method = HTTP_GET,
        .handler = bmp_handler,
        .user_ctx = NULL};

    httpd_uri_t xclk_uri = {
        .uri = "/xclk",
        .method = HTTP_GET,
        .handler = xclk_handler,
        .user_ctx = NULL};

    httpd_uri_t reg_uri = {
        .uri = "/reg",
        .method = HTTP_GET,
        .handler = reg_handler,
        .user_ctx = NULL};

    httpd_uri_t greg_uri = {
        .uri = "/greg",
        .method = HTTP_GET,
        .handler = greg_handler,
        .user_ctx = NULL};

    httpd_uri_t pll_uri = {
        .uri = "/pll",
        .method = HTTP_GET,
        .handler = pll_handler,
        .user_ctx = NULL};

    httpd_uri_t win_uri = {
        .uri = "/resolution",
        .method = HTTP_GET,
        .handler = win_handler,
        .user_ctx = NULL};        

    Serial.printf("Starting web server on port: '%d'", config.server_port);
    Serial.println("");
    if (httpd_start(&camera_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
        httpd_register_uri_handler(camera_httpd, &status_uri);
        httpd_register_uri_handler(camera_httpd, &capture_uri);
        httpd_register_uri_handler(camera_httpd, &bmp_uri);

        httpd_register_uri_handler(camera_httpd, &xclk_uri);
        httpd_register_uri_handler(camera_httpd, &reg_uri);
        httpd_register_uri_handler(camera_httpd, &greg_uri);
        httpd_register_uri_handler(camera_httpd, &pll_uri);
        httpd_register_uri_handler(camera_httpd, &win_uri);        
    }

    config.server_port += 1;
    config.ctrl_port += 1;
    Serial.printf("Starting stream server on port: '%d'", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}
