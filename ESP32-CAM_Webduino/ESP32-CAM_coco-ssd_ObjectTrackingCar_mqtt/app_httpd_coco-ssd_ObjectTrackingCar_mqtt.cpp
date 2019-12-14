/*
ESP32-CAM Object Tracking Car with Webduino MQTT (tfjs coco-ssd)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-10-6 12:00
https://www.facebook.com/francefu

Class
https://github.com/tensorflow/tfjs-models/blob/master/coco-ssd/src/classes.ts
*/

// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "Arduino.h"

#include "fb_gfx.h"
#include "fd_forward.h"
#include "dl_lib.h"  //delete
#include "fr_forward.h"

#define ENROLL_CONFIRM_TIMES 5
#define FACE_ID_SAVE_NUMBER 7

#define FACE_COLOR_WHITE  0x00FFFFFF
#define FACE_COLOR_BLACK  0x00000000
#define FACE_COLOR_RED    0x000000FF
#define FACE_COLOR_GREEN  0x0000FF00
#define FACE_COLOR_BLUE   0x00FF0000
#define FACE_COLOR_YELLOW (FACE_COLOR_RED | FACE_COLOR_GREEN)
#define FACE_COLOR_CYAN   (FACE_COLOR_BLUE | FACE_COLOR_GREEN)
#define FACE_COLOR_PURPLE (FACE_COLOR_BLUE | FACE_COLOR_RED)

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
static int8_t detection_enabled = 0;
static int8_t recognition_enabled = 0;
static int8_t is_enrolling = 0;
static face_id_list id_list = {0};

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

static void rgb_print(dl_matrix3du_t *image_matrix, uint32_t color, const char * str){
    fb_data_t fb;
    fb.width = image_matrix->w;
    fb.height = image_matrix->h;
    fb.data = image_matrix->item;
    fb.bytes_per_pixel = 3;
    fb.format = FB_BGR888;
    fb_gfx_print(&fb, (fb.width - (strlen(str) * 14)) / 2, 10, color, str);
}

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

static int run_face_recognition(dl_matrix3du_t *image_matrix, box_array_t *net_boxes){
    dl_matrix3du_t *aligned_face = NULL;
    int matched_id = 0;

    aligned_face = dl_matrix3du_alloc(1, FACE_WIDTH, FACE_HEIGHT, 3);
    if(!aligned_face){
        Serial.println("Could not allocate face recognition buffer");
        return matched_id;
    }
    if (align_face(net_boxes, image_matrix, aligned_face) == ESP_OK){
        if (is_enrolling == 1){
            int8_t left_sample_face = enroll_face(&id_list, aligned_face);

            if(left_sample_face == (ENROLL_CONFIRM_TIMES - 1)){
                Serial.printf("Enrolling Face ID: %d\n", id_list.tail);
            }
            Serial.printf("Enrolling Face ID: %d sample %d\n", id_list.tail, ENROLL_CONFIRM_TIMES - left_sample_face);
            rgb_printf(image_matrix, FACE_COLOR_CYAN, "ID[%u] Sample[%u]", id_list.tail, ENROLL_CONFIRM_TIMES - left_sample_face);
            if (left_sample_face == 0){
                is_enrolling = 0;
                Serial.printf("Enrolled Face ID: %d\n", id_list.tail);
            }
        } else {
            matched_id = recognize_face(&id_list, aligned_face);
            if (matched_id >= 0) {
                Serial.printf("Match Face ID: %u\n", matched_id);
                rgb_printf(image_matrix, FACE_COLOR_GREEN, "Hello Subject %u", matched_id);
            } else {
                Serial.println("No Match Found");
                rgb_print(image_matrix, FACE_COLOR_RED, "Intruder Alert!");
                matched_id = -1;
            }
        }
    } else {
        Serial.println("Face Not Aligned");
        //rgb_print(image_matrix, FACE_COLOR_YELLOW, "Human Detected");
    }

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

    box_array_t *net_boxes = face_detect(image_matrix, &mtmn_config);

    if (net_boxes){
        detected = true;
        if(recognition_enabled){
            face_id = run_face_recognition(image_matrix, net_boxes);
        }
        draw_face_boxes(image_matrix, net_boxes, face_id);
        free(net_boxes->box);
        free(net_boxes->landmark);
        free(net_boxes);
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

static esp_err_t stream_handler(httpd_req_t *req){
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
        return res;
    }

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
                            net_boxes = face_detect(image_matrix, &mtmn_config);
                        }
                        fr_face = esp_timer_get_time();
                        fr_recognize = fr_face;
                        if (net_boxes || fb->format != PIXFORMAT_JPEG){
                            if(net_boxes){
                                detected = true;
                                if(recognition_enabled){
                                    face_id = run_face_recognition(image_matrix, net_boxes);
                                }
                                fr_recognize = esp_timer_get_time();
                                draw_face_boxes(image_matrix, net_boxes, face_id);
                                free(net_boxes->box);
                                free(net_boxes->landmark);
                                free(net_boxes);
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
        uint32_t avg_frame_time = ra_filter_run(&ra_filter, frame_time);
        Serial.printf("MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps), %u+%u+%u+%u=%u %s%d\n",
            (uint32_t)(_jpg_buf_len),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time,
            avg_frame_time, 1000.0 / avg_frame_time,
            (uint32_t)ready_time, (uint32_t)face_time, (uint32_t)recognize_time, (uint32_t)encode_time, (uint32_t)process_time,
            (detected)?"DETECTED ":"", face_id
        );
    }

    last_frame = 0;
    return res;
}

static esp_err_t cmd_handler(httpd_req_t *req){
    char*  buf;
    size_t buf_len;
    char variable[32] = {0,};
    char value[32] = {0,};

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
            } else {
                free(buf);
                httpd_resp_send_404(req);
                return ESP_FAIL;
            }
        } else {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    int val = atoi(value);
    sensor_t * s = esp_camera_sensor_get();
    int res = 0;
    
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
    else if(!strcmp(variable, "face_detect")) {
        detection_enabled = val;
        if(!detection_enabled) {
            recognition_enabled = 0;
        }
    }
    else if(!strcmp(variable, "face_enroll")) is_enrolling = val;
    else if(!strcmp(variable, "face_recognize")) {
        recognition_enabled = val;
        if(recognition_enabled){
            detection_enabled = val;
        }
    }     
    else if(!strcmp(variable, "flash")) {
      ledcWrite(4,val);
    } 
    else {
        res = -1;
    }
    if(res){
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}

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
    p+=sprintf(p, "\"colorbar\":%u,", s->status.colorbar);
    p+=sprintf(p, "\"face_detect\":%u,", detection_enabled);
    p+=sprintf(p, "\"face_enroll\":%u,", is_enrolling);
    p+=sprintf(p, "\"face_recognize\":%u", recognition_enabled);
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!doctype html>
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
        <script src="http://cdn.jsdelivr.net/npm/@tensorflow/tfjs@1.0.4"> </script>
        <script src="http://cdn.jsdelivr.net/npm/@tensorflow-models/coco-ssd"> </script> 
        <script src="http://blocklypro.webduino.io/components/jquery/dist/jquery.min.js"></script>
        <script src="http://blocklypro.webduino.io/dist/lib/webduino-all-0.4.20.min.js"></script>
        <script src="http://blocklypro.webduino.io/dist/webduino-blockly.min.js"></script>
        <script src="http://blocklypro.webduino.io/dist/lib/firebase.min.js"></script>
        <script src="http://blocklypro.webduino.io/dist/lib/runtime.min.js"></script>
        <script src="http://blocklypro.webduino.io/node_modules/webduino-module-mqtt/mqttClient.min.js"></script>
    </head>
    <body>
        <section class="main">
            <section id="buttons">
                <table>
                <tr style="visibility:hidden"><td colspan="3"><button id="get-still">Start Object Detection (coco-ssd)</button><button id="toggle-stream"></button><button id="face_enroll" class="disabled" disabled="disabled"></button></td></tr>
                <tr><td>Flash</td><td align="center" colspan="2"><input type="range" id="flash" min="0" max="255" value="0" onchange="try{fetch(document.location.origin+'/control?var=flash&val='+this.value);}catch(e){}"></td></tr>
                <tr><td colspan="3">
                    ScoreLimit
                    <select id="score">
                      <option value="1.0">1</option>
                      <option value="0.9">0.9</option>
                      <option value="0.8">0.8</option>
                      <option value="0.7">0.7</option>
                      <option value="0.6">0.6</option>
                      <option value="0.5" selected="selected">0.5</option>
                      <option value="0.4">0.4</option>
                      <option value="0.3">0.3</option>
                      <option value="0.2">0.2</option>
                      <option value="0.1">0.1</option>
                      <option value="0">0</option>
                    </select>
                    MirrorImage
                    <select id="mirrorimage">
                      <option value="1">yes</option>
                      <option value="0">no</option>
                    </select>
                </td></tr> 
                <tr style="display:none"><td colspan="3"><span id="message"></span></td></tr>
                <tr><td colspan="3"><canvas id="canvas" width="0" height="0"></canvas></td></tr>
                </table>
            </section>
            <figure>
              <div id="stream-container" class="image-container hidden">
                <div class="close" id="close-stream">×</div>
                <img id="stream" src="" style="display:none">
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
                                <option value="4" selected="selected">QVGA(320x240)</option>
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
                    </nav>
                </div>
            </div>
        </section>
        <div><input type="checkbox" id="mqtt">Start MQTT
        <select id="object">
          <option value="person">person</option>
          <option value="bicycle">bicycle</option>
          <option value="car">car</option>
          <option value="motorcycle">motorcycle</option>
          <option value="airplane">airplane</option>
          <option value="bus">bus</option>
          <option value="train">train</option>
          <option value="truck">truck</option>
          <option value="boat">boat</option>
          <option value="traffic light">traffic light</option>
          <option value="fire hydrant">fire hydrant</option>
          <option value="stop sign">stop sign</option>
          <option value="parking meter">parking meter</option>
          <option value="bench">bench</option>
          <option value="bird">bird</option>
          <option value="cat">cat</option>
          <option value="dog">dog</option>
          <option value="horse">horse</option>
          <option value="sheep">sheep</option>
          <option value="cow">cow</option>
          <option value="elephant">elephant</option>
          <option value="bear">bear</option>
          <option value="zebra">zebra</option>
          <option value="giraffe">giraffe</option>
          <option value="backpack">backpack</option>
          <option value="umbrella">umbrella</option>
          <option value="handbag">handbag</option>
          <option value="tie">tie</option>
          <option value="suitcase">suitcase</option>
          <option value="frisbee">frisbee</option>
          <option value="skis">skis</option>
          <option value="snowboard">snowboard</option>
          <option value="sports ball">sports ball</option>
          <option value="kite">kite</option>
          <option value="baseball bat">baseball bat</option>
          <option value="baseball glove">baseball glove</option>
          <option value="skateboard">skateboard</option>
          <option value="surfboard">surfboard</option>
          <option value="tennis racket">tennis racket</option>
          <option value="bottle">bottle</option>
          <option value="wine glass">wine glass</option>
          <option value="cup">cup</option>
          <option value="fork">fork</option>
          <option value="knife">knife</option>
          <option value="spoon">spoon</option>
          <option value="bowl">bowl</option>
          <option value="banana">banana</option>
          <option value="apple">apple</option>
          <option value="sandwich">sandwich</option>
          <option value="orange">orange</option>
          <option value="broccoli">broccoli</option>
          <option value="carrot">carrot</option>
          <option value="hot dog">hot dog</option>
          <option value="pizza">pizza</option>
          <option value="donut">donut</option>
          <option value="cake">cake</option>
          <option value="chair">chair</option>
          <option value="couch">couch</option>
          <option value="potted plant">potted plant</option>
          <option value="bed">bed</option>
          <option value="dining table">dining table</option>
          <option value="toilet">toilet</option>
          <option value="tv">tv</option>
          <option value="laptop">laptop</option>
          <option value="mouse">mouse</option>
          <option value="remote">remote</option>
          <option value="keyboard">keyboard</option>
          <option value="cell phone">cell phone</option>
          <option value="microwave">microwave</option>
          <option value="oven">oven</option>
          <option value="toaster">toaster</option>
          <option value="sink">sink</option>
          <option value="refrigerator">refrigerator</option>
          <option value="book">book</option>
          <option value="clock">clock</option>
          <option value="vase">vase</option>
          <option value="scissors">scissors</option>
          <option value="teddy bear">teddy bear</option>
          <option value="hair drier">hair drier</option>
          <option value="toothbrush">toothbrush</option>
        </select>
        </div>
        <div id="result"><div>
        <script>
          document.addEventListener('DOMContentLoaded',function(){function b(B){let C;switch(B.type){case'checkbox':C=B.checked?1:0;break;case'range':case'select-one':C=B.value;break;case'button':case'submit':C='1';break;default:return;}const D=`${c}/control?var=${B.id}&val=${C}`;fetch(D).then(E=>{console.log(`request to ${D} finished, status: ${E.status}`)})}var c=document.location.origin;const e=B=>{B.classList.add('hidden')},f=B=>{B.classList.remove('hidden')},g=B=>{B.classList.add('disabled'),B.disabled=!0},h=B=>{B.classList.remove('disabled'),B.disabled=!1},i=(B,C,D)=>{D=!(null!=D)||D;let E;'checkbox'===B.type?(E=B.checked,C=!!C,B.checked=C):(E=B.value,B.value=C),D&&E!==C?b(B):!D&&('aec'===B.id?C?e(v):f(v):'agc'===B.id?C?(f(t),e(s)):(e(t),f(s)):'awb_gain'===B.id?C?f(x):e(x):'face_recognize'===B.id&&(C?h(n):g(n)))};document.querySelectorAll('.close').forEach(B=>{B.onclick=()=>{e(B.parentNode)}}),fetch(`${c}/status`).then(function(B){return B.json()}).then(function(B){document.querySelectorAll('.default-action').forEach(C=>{i(C,B[C.id],!1)})});const j=document.getElementById('stream'),k=document.getElementById('stream-container'),l=document.getElementById('get-still'),m=document.getElementById('toggle-stream'),n=document.getElementById('face_enroll'),o=document.getElementById('close-stream'),p=()=>{window.stop(),m.innerHTML='Start Stream'},q=()=>{j.src=`${c+':81'}/stream`,f(k),m.innerHTML='Stop Stream'};l.onclick=()=>{p(),j.src=`${c}/capture?_cb=${Date.now()}`,f(k)},o.onclick=()=>{p(),e(k)},m.onclick=()=>{const B='Stop Stream'===m.innerHTML;B?p():q()},n.onclick=()=>{b(n)},document.querySelectorAll('.default-action').forEach(B=>{B.onchange=()=>b(B)});const r=document.getElementById('agc'),s=document.getElementById('agc_gain-group'),t=document.getElementById('gainceiling-group');r.onchange=()=>{b(r),r.checked?(f(t),e(s)):(e(t),f(s))};const u=document.getElementById('aec'),v=document.getElementById('aec_value-group');u.onchange=()=>{b(u),u.checked?e(v):f(v)};const w=document.getElementById('awb_gain'),x=document.getElementById('wb_mode-group');w.onchange=()=>{b(w),w.checked?f(x):e(x)};const y=document.getElementById('face_detect'),z=document.getElementById('face_recognize'),A=document.getElementById('framesize');A.onchange=()=>{b(A),5<A.value&&(i(y,!1),i(z,!1))},y.onchange=()=>{return 5<A.value?(alert('Please select CIF or lower resolution before enabling this feature!'),void i(y,!1)):void(b(y),!y.checked&&(g(n),i(z,!1)))},z.onchange=()=>{return 5<A.value?(alert('Please select CIF or lower resolution before enabling this feature!'),void i(z,!1)):void(b(z),z.checked?(h(n),i(y,!0)):g(n))}});
        </script>
    <!-- Object Detection -->    
    <script>
    var getStill = document.getElementById('get-still');
    var ShowImage = document.getElementById('stream');
    var canvas = document.getElementById("canvas");
    var context = canvas.getContext("2d");  
    var result = document.getElementById('result');
    var Model;

    function ObjectDetect() {
      result.innerHTML = "Please wait for loading model.";
      cocoSsd.load().then(cocoSsd_Model => {
        Model = cocoSsd_Model;
        result.innerHTML = "";
        getStill.click();
      }); 
    }
    function DetectImage() { 
      canvas.setAttribute("width", ShowImage.width);
      canvas.setAttribute("height", ShowImage.height);
      var mirrorimage = Number(document.getElementById("mirrorimage").value);
      if (mirrorimage==1) {
        context.translate((canvas.width + ShowImage.width) / 2, 0);
        context.scale(-1, 1);
        context.drawImage(ShowImage, 0, 0, ShowImage.width, ShowImage.height);
        context.setTransform(1, 0, 0, 1, 0, 0);
      }
      else
        context.drawImage(ShowImage,0,0,ShowImage.width,ShowImage.height);
      
      Model.detect(canvas).then(Predictions => {
        var s = (ShowImage.width>ShowImage.height)?ShowImage.width:ShowImage.height;
        var score = Number(document.getElementById("score").value);
        //console.log(score);
        var trackState = 0;
        var x, y, width, height;
        //console.log('Predictions: ', Predictions);
        if (Predictions.length>0) {
          result.innerHTML = "";
          for (var i=0;i<Predictions.length;i++) {
            x = Number(Predictions[i].bbox[0]);
            y = Number(Predictions[i].bbox[1]);
            width = Number(Predictions[i].bbox[2]);
            height = Number(Predictions[i].bbox[3]);
            context.lineWidth = Math.round(s/200);
            context.strokeStyle = "#00FFFF";
            context.beginPath();
            context.rect(x, y, width, height);
            context.stroke(); 
            context.lineWidth = "2";
            context.fillStyle = "red";
            context.font = Math.round(s/30) + "px Arial";
            context.fillText(Predictions[i].class, x, y);
            //context.fillText(i, x, y);
            result.innerHTML+= "[ "+i+" ] "+Predictions[i].class+", "+Math.round(Predictions[i].score*100)+"%, "+Math.round(x)+", "+Math.round(y)+", "+Math.round(width)+", "+Math.round(height)+"<br>";
            var object = document.getElementById('object').value;
            //https://github.com/tensorflow/tfjs-models/blob/master/coco-ssd/src/classes.ts
            if (Predictions[i].class==object&&Predictions[i].score>=score&&trackState==0) {   
              try{
                trackState = 1;
                var midX = Math.round(x)+Math.round(width)/2;
                var midY = Math.round(y)+Math.round(height)/2;
                message.innerHTML = Math.round((midX-160)/1.6,2)+","+Math.round((midY-120)/1.2,2)+","+Math.round(x)+","+Math.round(y)+","+Math.round(width)+","+Math.round(height);
              }
              catch(e){}
            }  
          }
          if (trackState == 0)
            message.innerHTML = "0,0,0,0,0,0";
        }
        else
          result.innerHTML = "Unrecognizable";
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
    
    <script id="jsbin-javascript">
      (async function () {
        var topic = "my/xxxxx";
        var webduinoMQTTClient = new webduino.module.mqttClient();
        await webduinoMQTTClient.connect({ server: 'wss://mqtt1.webduino.io/mqtt' });
        await webduinoMQTTClient.subscribe(topic);
        setInterval(async function () {
          var message = document.getElementById('message').innerHTML;
          document.getElementById('message').innerHTML = "";
          if (document.getElementById('mqtt').checked == true&&message!="") {
            webduinoMQTTClient.send({topic: topic,message: message});
          }
        }, 100);
      }());
    </script>      
    </body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req){
    httpd_resp_set_type(req, "text/html");
    //httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    //return httpd_resp_send(req, (const char *)index_html_gz, index_html_gz_len);
    return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

void startCameraServer(){
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

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
    
    mtmn_config.min_face = 80;
    mtmn_config.pyramid = 0.7;
    mtmn_config.p_threshold.score = 0.6;
    mtmn_config.p_threshold.nms = 0.7;
    mtmn_config.r_threshold.score = 0.7;
    mtmn_config.r_threshold.nms = 0.7;
    mtmn_config.r_threshold.candidate_number = 4;
    mtmn_config.o_threshold.score = 0.7;
    mtmn_config.o_threshold.nms = 0.4;
    mtmn_config.o_threshold.candidate_number = 1;
    
    face_id_init(&id_list, FACE_ID_SAVE_NUMBER, ENROLL_CONFIRM_TIMES);
    
    Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &cmd_uri);
        httpd_register_uri_handler(camera_httpd, &status_uri);
        httpd_register_uri_handler(camera_httpd, &capture_uri);
    }

    config.server_port += 1;
    config.ctrl_port += 1;
    Serial.printf("Starting stream server on port: '%d'\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}
