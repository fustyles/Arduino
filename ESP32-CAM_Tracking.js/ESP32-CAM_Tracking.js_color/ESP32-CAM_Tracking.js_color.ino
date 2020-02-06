/*
ESP32-CAM Tracking.js Color Detection
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-2-4 18:00
https://www.facebook.com/francefu

Color List
https://en.wikipedia.org/wiki/Web_colors

首頁
http://APIP
http://STAIP

自訂指令格式 :  
http://APIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

預設AP端IP： 192.168.4.1
http://192.168.xxx.xxx?ip
http://192.168.xxx.xxx?mac
http://192.168.xxx.xxx?restart
http://192.168.xxx.xxx?digitalwrite=pin;value        //value= 0 or 1
http://192.168.xxx.xxx?analogwrite=pin;value        //value= 0~255
http://192.168.xxx.xxx?flash=value        //value= 0~255 閃光燈
http://192.168.xxx.xxx?getstill                 //取得視訊影像
http://192.168.xxx.xxx?framesize=size     //size= UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA 改變影像解析度
http://192.168.xxx.xxx?quality=value    // value = 10 to 63
http://192.168.xxx.xxx?brightness=value    // value = -2 to 2
http://192.168.xxx.xxx?contrast=value    // value = -2 to 2 
http://192.168.xxx.xxx/?tcp=domain;port;request;wait
--> request = /xxxxx
--> wait = 0 ro 1
http://192.168.xxx.xxx/?linenotify=token;request
--> request = message=xxxxx
--> request = message=xxxxx&stickerPackageId=xxxxx&stickerId=xxxxx
http://192.168.xxx.xxx?sendCapturedImageToLineNotify=token  //傳送影像截圖至LineNotify，最大解析度是SXGA

查詢Client端IP：
查詢IP：http://192.168.4.1/?ip
重設網路：http://192.168.4.1/?resetwifi=ssid;password
*/

//輸入WIFI連線帳號密碼
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

//輸入AP端連線帳號密碼
const char* apssid = "ESP32-CAM";
const char* appassword = "12345678";    //AP端密碼至少要八個字元以上

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"         //視訊
#include "soc/soc.h"            //用於電源不穩不重開機
#include "soc/rtc_cntl_reg.h"   //用於電源不穩不重開機

String Feedback="";   //回傳客戶端訊息
//指令參數值
String Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
//指令拆解狀態值
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

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

WiFiServer server(80);

void ExecuteCommand()
{
  //Serial.println("");
  //Serial.println("Command: "+Command);
  if (cmd!="getstill") {
    Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
    Serial.println("");
  }
  
  if (cmd=="your cmd") {
    // You can do anything.
    // Feedback="<font color=\"red\">Hello World</font>";
  }
  else if (cmd=="ip") {
    Feedback="AP IP: "+WiFi.softAPIP().toString();    
    Feedback+=", ";
    Feedback+="STA IP: "+WiFi.localIP().toString();
  }  
  else if (cmd=="mac") {
    Feedback="STA MAC: "+WiFi.macAddress();
  }  
  else if (cmd=="resetwifi") {  //重設WIFI連線
    WiFi.begin(P1.c_str(), P2.c_str());
    Serial.print("Connecting to ");
    Serial.println(P1);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Serial.println("");
    Serial.println("STAIP: "+WiFi.localIP().toString());
    Feedback="STAIP: "+WiFi.localIP().toString();
  }    
  else if (cmd=="restart") {
    ESP.restart();
  }
  else if (cmd=="digitalwrite") {
    ledcDetachPin(P1.toInt());
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(), P2.toInt());
  }   
  else if (cmd=="analogwrite") {
    if (P1="4") {
      ledcAttachPin(4, 4);  
      ledcSetup(4, 5000, 8);   
      ledcWrite(4,P2.toInt());  
    }
    else {
      ledcAttachPin(P1.toInt(), 5);
      ledcSetup(5, 5000, 8);
      ledcWrite(5,P2.toInt());
    }
  }  
  else if (cmd=="flash") {
    ledcAttachPin(4, 4);  
    ledcSetup(4, 5000, 8);   
     
    int val = P1.toInt();
    ledcWrite(4,val);  
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
      s->set_framesize(s, FRAMESIZE_QVGA);     
  }
  else if (cmd=="quality") { 
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_quality(s, val);
  }
  else if (cmd=="contrast") {
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_contrast(s, val);
  }
  else if (cmd=="brightness") {
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt();  
    s->set_brightness(s, val);  
  }
  else if (cmd=="detectCount") {
    Serial.println(P1+" = "+P2); 
  }
  else if (cmd=="tcp") {
    String domain=P1;
    int port=P2.toInt();
    String request=P3;
    int wait=P4.toInt();      // wait = 0 or 1

    if ((port==443)||(domain.indexOf("https")==0)||(domain.indexOf("HTTPS")==0))
      Feedback=tcp_https(domain,request,port,wait);
    else
      Feedback=tcp_http(domain,request,port,wait);  
  }
  else if (cmd=="linenotify") {    //message=xxx&stickerPackageId=xxx&stickerId=xxx
    String token = P1;
    String request = P2;
    Feedback=LineNotify(token,request,1);
    if (Feedback.indexOf("status")!=-1) {
      int s=Feedback.indexOf("{");
      Feedback=Feedback.substring(s);
      int e=Feedback.indexOf("}");
      Feedback=Feedback.substring(0,e);
      Feedback.replace("\"","");
      Feedback.replace("{","");
      Feedback.replace("}","");
    }
  }
  else if (cmd=="sendCapturedImageToLineNotify") { 
    Feedback=sendCapturedImageToLineNotify(P1);
    if (Feedback=="") Feedback="The image failed to send. <br>The framesize may be too large.";
  } 
  else {
    Feedback="Command is not defined.";
  }
  if (Feedback=="") Feedback=Command;  
}

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
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);  //UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA  設定初始化影像解析度

  //閃光燈
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);    
  
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

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);  

  server.begin();          
}

//自訂網頁首頁管理介面
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <!DOCTYPE html>
  <head>
  <title>tracking.js - color with camera</title>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <script src="https:\/\/ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js"></script>
  <script src="https:\/\/fustyles.github.io/webduino/Tracking_20190917/tracking-min.js"></script>  
  </head><body>
  <img id="ShowImage" src="" style="display:none">
  <canvas id="canvas" width="0" height="0"></canvas>  
  <table>
  <tr>
    <td><input type="button" id="restart" value="Restart"></td> 
    <td colspan="2"><input type="button" id="getStill" value="Start Detection"></td> 
  </tr>  
  <tr>
    <td>Color Name</td>
    <td>
      <select id="myColor" onchange="changeColor(this.options[this.selectedIndex].text);">
        <option value="AliceBlue">AliceBlue_240,248,255</option>
        <option value="AntiqueWhite">AntiqueWhite_250,235,215</option>
        <option value="Aqua">Aqua_0,255,255</option>
        <option value="Aquamarine">Aquamarine_127,255,212</option>
        <option value="Azure">Azure_240,255,255</option>
        <option value="Beige">Beige_245,245,220</option>
        <option value="Bisque">Bisque_255,228,196</option>
        <option value="Black">Black_0,0,0</option>
        <option value="BlanchedAlmond">BlanchedAlmond_255,235,205</option>
        <option value="Blue">Blue_0,0,255</option>
        <option value="BlueViolet">BlueViolet_138,43,226</option>
        <option value="Brown">Brown_165,42,42</option>
        <option value="Burlywood">Burlywood_222,184,135</option>
        <option value="CadetBlue">CadetBlue_95,158,160</option>
        <option value="Chartreuse">Chartreuse_127,255,0</option>
        <option value="Chocolate">Chocolate_210,105,30</option>
        <option value="Coral">Coral_255,127,80</option>
        <option value="CornflowerBlue">CornflowerBlue_100,149,237</option>
        <option value="Cornsilk">Cornsilk_255,248,220</option>
        <option value="Crimson">Crimson_220,20,60</option>
        <option value="Cyan">Cyan_0,255,255</option>
        <option value="DarkBlue">DarkBlue_0,0,139</option>
        <option value="DarkCyan">DarkCyan_0,139,139</option>
        <option value="DarkGoldenrod">DarkGoldenrod_184,134,11</option>
        <option value="DarkGray">DarkGray_169,169,169</option>
        <option value="DarkGreen">DarkGreen_0,100,0</option>
        <option value="DarkKhaki">DarkKhaki_189,183,107</option>
        <option value="DarkMagenta">DarkMagenta_139,0,139</option>
        <option value="DarkOliveGreen">DarkOliveGreen_85,107,47</option>
        <option value="DarkOrange">DarkOrange_255,140,0</option>
        <option value="DarkOrchid">DarkOrchid_153,50,204</option>
        <option value="DarkRed">DarkRed_139,0,0</option>
        <option value="DarkSalmon">DarkSalmon_233,150,122</option>
        <option value="DarkSeaGreen">DarkSeaGreen_143,188,143</option>
        <option value="DarkSlateBlue">DarkSlateBlue_72,61,139</option>
        <option value="DarkSlateGray">DarkSlateGray_47,79,79</option>
        <option value="DarkTurquoise">DarkTurquoise_0,206,209</option>
        <option value="DarkViolet">DarkViolet_148,0,211</option>
        <option value="DeepPink">DeepPink_255,20,147</option>
        <option value="DeepSkyBlue">DeepSkyBlue_0,191,255</option>
        <option value="DimGray">DimGray_105,105,105</option>
        <option value="DodgerBlue">DodgerBlue_30,144,255</option>
        <option value="Firebrick">Firebrick_178,34,34</option>
        <option value="FloralWhite">FloralWhite_255,250,240</option>
        <option value="ForestGreen">ForestGreen_34,139,34</option>
        <option value="Fuchsia">Fuchsia_255,0,255</option>
        <option value="Gainsboro">Gainsboro_220,220,220</option>
        <option value="GhostWhite">GhostWhite_248,248,255</option>
        <option value="Gold">Gold_255,215,0</option>
        <option value="Goldenrod">Goldenrod_218,165,32</option>
        <option value="Gray">Gray_128,128,128</option>
        <option value="Green">Green_0,128,0</option>
        <option value="GreenYellow">GreenYellow_173,255,47</option>
        <option value="Honeydew">Honeydew_240,255,240</option>
        <option value="HotPink">HotPink_255,105,180</option>
        <option value="IndianRed">IndianRed_205,92,92</option>
        <option value="Indigo">Indigo_75,0,130</option>
        <option value="Ivory">Ivory_255,255,240</option>
        <option value="Khaki">Khaki_240,230,140</option>
        <option value="Lavender">Lavender_230,230,250</option>
        <option value="LavenderBlush">LavenderBlush_255,240,245</option>
        <option value="LawnGreen">LawnGreen_124,252,0</option>
        <option value="LemonChiffon">LemonChiffon_255,250,205</option>
        <option value="LightBlue">LightBlue_173,216,230</option>
        <option value="LightCoral">LightCoral_240,128,128</option>
        <option value="LightCyan">LightCyan_224,255,255</option>
        <option value="LightGoldenrodYellow">LightGoldenrodYellow_250,250,210</option>
        <option value="LightGray">LightGray_211,211,211</option>
        <option value="LightGreen">LightGreen_144,238,144</option>
        <option value="LightPink">LightPink_255,182,193</option>
        <option value="LightSalmon">LightSalmon_255,160,122</option>
        <option value="LightSeaGreen">LightSeaGreen_32,178,170</option>
        <option value="LightSkyBlue">LightSkyBlue_135,206,250</option>
        <option value="LightSlateGray">LightSlateGray_119,136,153</option>
        <option value="LightSteelBlue">LightSteelBlue_176,196,222</option>
        <option value="LightYellow">LightYellow_255,255,224</option>
        <option value="Lime">Lime_0,255,0</option>
        <option value="LimeGreen">LimeGreen_50,205,50</option>
        <option value="Linen">Linen_250,240,230</option>
        <option value="Magenta">Magenta_255,0,255</option>
        <option value="Maroon">Maroon_128,0,0</option>
        <option value="MediumAquamarine">MediumAquamarine_102,205,170</option>
        <option value="MediumBlue">MediumBlue_0,0,205</option>
        <option value="MediumOrchid">MediumOrchid_186,85,211</option>
        <option value="MediumPurple">MediumPurple_147,112,219</option>
        <option value="MediumSeaGreen">MediumSeaGreen_60,179,113</option>
        <option value="MediumSlateBlue">MediumSlateBlue_123,104,238</option>
        <option value="MediumSpringGreen">MediumSpringGreen_0,250,154</option>
        <option value="MediumTurquoise">MediumTurquoise_72,209,204</option>
        <option value="MediumVioletRed">MediumVioletRed_199,21,133</option>
        <option value="MidnightBlue">MidnightBlue_25,25,112</option>
        <option value="MintCream">MintCream_245,255,250</option>
        <option value="MistyRose">MistyRose_255,228,225</option>
        <option value="Moccasin">Moccasin_255,228,181</option>
        <option value="NavajoWhite">NavajoWhite_255,222,173</option>
        <option value="Navy">Navy_0,0,128</option>
        <option value="OldLace">OldLace_253,245,230</option>
        <option value="Olive">Olive_128,128,0</option>
        <option value="OliveDrab">OliveDrab_107,142,35</option>
        <option value="Orange">Orange_255,165,0</option>
        <option value="OrangeRed">OrangeRed_255,69,0</option>
        <option value="Orchid">Orchid_218,112,214</option>
        <option value="PaleGoldenrod">PaleGoldenrod_238,232,170</option>
        <option value="PaleGreen">PaleGreen_152,251,152</option>
        <option value="PaleTurquoise">PaleTurquoise_175,238,238</option>
        <option value="PaleVioletRed">PaleVioletRed_219,112,147</option>
        <option value="PapayaWhip">PapayaWhip_255,239,213</option>
        <option value="PeachPuff">PeachPuff_255,218,185</option>
        <option value="Peru">Peru_205,133,63</option>
        <option value="Pink">Pink_255,192,203</option>
        <option value="Plum">Plum_221,160,221</option>
        <option value="PowderBlue">PowderBlue_176,224,230</option>
        <option value="Purple">Purple_128,0,128</option>
        <option value="Red">Red_255,0,0</option>
        <option value="RosyBrown">RosyBrown_188,143,143</option>
        <option value="RoyalBlue">RoyalBlue_65,105,225</option>
        <option value="SaddleBrown">SaddleBrown_139,69,19</option>
        <option value="Salmon">Salmon_250,128,114</option>
        <option value="SandyBrown">SandyBrown_244,164,96</option>
        <option value="SeaGreen">SeaGreen_46,139,87</option>
        <option value="Seashell">Seashell_255,245,238</option>
        <option value="Sienna">Sienna_160,82,45</option>
        <option value="Silver">Silver_192,192,192</option>
        <option value="SkyBlue">SkyBlue_135,206,235</option>
        <option value="SlateBlue">SlateBlue_106,90,205</option>
        <option value="SlateGray">SlateGray_112,128,144</option>
        <option value="Snow">Snow_255,250,250</option>
        <option value="SpringGreen">SpringGreen_0,255,127</option>
        <option value="SteelBlue">SteelBlue_70,130,180</option>
        <option value="Tan">Tan_210,180,140</option>
        <option value="Teal">Teal_0,128,128</option>
        <option value="Thistle">Thistle_216,191,216</option>
        <option value="Tomato">Tomato_255,99,71</option>
        <option value="Turquoise">Turquoise_64,224,208</option>
        <option value="Violet">Violet_238,130,238</option>
        <option value="Wheat">Wheat_245,222,179</option>
        <option value="White">White_255,255,255</option>
        <option value="WhiteSmoke">WhiteSmoke_245,245,245</option>
        <option value="Yellow">Yellow_255,255,0</option>
        <option value="YellowGreen">YellowGreen_154,205,50</option>
      </select>
    </td>
  </tr>
  <tr>
    <td>Range R</td>
    <td>min<input type="range" id="myColor_r_min" min="0" max="255" step="1" onchange="myColor_r_min_v.innerHTML=this.value;"><span id="myColor_r_min_v">0</span><br>
      max<input type="range" id="myColor_r_max" min="0" max="255" step="1" onchange="myColor_r_max_v.innerHTML=this.value;"><span id="myColor_r_max_v">0</span>
    </td>
  </tr>
  <tr>
    <td>Range G</td>
    <td>min<input type="range" id="myColor_g_min" min="0" max="255" step="1" onchange="myColor_g_min_v.innerHTML=this.value;"><span id="myColor_g_min_v">0</span><br>
      max<input type="range" id="myColor_g_max" min="0" max="255" step="1" onchange="myColor_g_max_v.innerHTML=this.value;"><span id="myColor_g_max_v">0</span>
    </td>
  </tr>
  <tr>
    <td>Range B</td>
    <td>min<input type="range" id="myColor_b_min" min="0" max="255" step="1" onchange="myColor_b_min_v.innerHTML=this.value;"><span id="myColor_b_min_v">0</span><br>
      max<input type="range" id="myColor_b_max" min="0" max="255" step="1" onchange="myColor_b_max_v.innerHTML=this.value;"><span id="myColor_b_max_v">0</span>
    </td>
  </tr>
  <tr>
    <td>Flash</td>
    <td colspan="2"><input type="range" id="flash" min="0" max="255" value="0"></td>
  </tr>
  <tr>
    <td>Quality</td>
    <td colspan="2"><input type="range" id="quality" min="10" max="63" value="10"></td>
  </tr>
  <tr>
    <td>Brightness</td>
    <td colspan="2"><input type="range" id="brightness" min="-2" max="2" value="0"></td>
  </tr>
  <tr>
    <td>Contrast</td>
    <td colspan="2"><input type="range" id="contrast" min="-2" max="2" value="0"></td>
  </tr>
  <tr>
    <td>Resolution</td> 
    <td colspan="2">
    <select id="framesize">
        <option value="UXGA">UXGA(1600x1200)</option>
        <option value="SXGA">SXGA(1280x1024)</option>
        <option value="XGA">XGA(1024x768)</option>
        <option value="SVGA">SVGA(800x600)</option>
        <option value="VGA">VGA(640x480)</option>
        <option value="CIF">CIF(400x296)</option>
        <option value="QVGA" selected="selected">QVGA(320x240)</option>
        <option value="HQVGA">HQVGA(240x176)</option>
        <option value="QQVGA">QQVGA(160x120)</option>
    </select> 
    </td>
  </tr>  
  <tr>
    <td>MirrorImage</td> 
    <td colspan="2">  
      <select id="mirrorimage">
        <option value="1">yes</option>
        <option value="0">no</option>
      </select>
    </td>
  </tr>     
  <tr>
    <td>Rotate</td>
    <td align="left" colspan="2">
        <select onchange="document.getElementById('canvas').style.transform='rotate('+this.value+')';">
          <option value="0deg">0deg</option>
          <option value="90deg">90deg</option>
          <option value="180deg">180deg</option>
          <option value="270deg">270deg</option>
        </select>
    </td>
  </tr>  
  </table>
  <iframe id="ifr" style="display:none"></iframe>
  <div id="result" style="color:red"><div>
  </body>
  </html> 
  
  <script>
    var getStill = document.getElementById('getStill');
    var ShowImage = document.getElementById('ShowImage');
    var canvas = document.getElementById("canvas");
    var context = canvas.getContext("2d"); 
    var myColor = document.getElementById('myColor');
    var mirrorimage = document.getElementById("mirrorimage");   
    var result = document.getElementById('result');
    var flash = document.getElementById('flash'); 
    var ifr = document.getElementById('ifr');
    var lastValue = "";
    var myTimer;
    var restartCount=0;  
    var myColor_r_min,myColor_r_max,myColor_g_min,myColor_g_max,myColor_b_min,myColor_b_max;

    var tracker = new tracking.ColorTracker();         

    //初始化偵測顏色
    document.getElementById('myColor').value = "Black";
    document.getElementById('myColor_r_min').value = 0;
    document.getElementById('myColor_r_min_v').innerHTML = 0;
    document.getElementById('myColor_r_max').value = 50;
    document.getElementById('myColor_r_max_v').innerHTML = 50;
    document.getElementById('myColor_g_min').value = 0;
    document.getElementById('myColor_g_min_v').innerHTML = 0;
    document.getElementById('myColor_g_max').value = 50;
    document.getElementById('myColor_g_max_v').innerHTML = 50;
    document.getElementById('myColor_b_min').value = 0;
    document.getElementById('myColor_b_min_v').innerHTML = 0;
    document.getElementById('myColor_b_max').value = 50;
    document.getElementById('myColor_b_max_v').innerHTML = 50;
  
    function changeColor(detectColor) {
      var val = detectColor.split("_");
      document.getElementById('myColor_r_min').value = val[1].split(",")[0];
      document.getElementById('myColor_r_min_v').innerHTML = val[1].split(",")[0];
      document.getElementById('myColor_r_max').value = val[1].split(",")[0];
      document.getElementById('myColor_r_max_v').innerHTML = val[1].split(",")[0];
      document.getElementById('myColor_g_min').value = val[1].split(",")[1];
      document.getElementById('myColor_g_min_v').innerHTML = val[1].split(",")[1];
      document.getElementById('myColor_g_max').value = val[1].split(",")[1];
      document.getElementById('myColor_g_max_v').innerHTML = val[1].split(",")[1];
      document.getElementById('myColor_b_min').value = val[1].split(",")[2];
      document.getElementById('myColor_b_min_v').innerHTML = val[1].split(",")[2];
      document.getElementById('myColor_b_max').value = val[1].split(",")[2];
      document.getElementById('myColor_b_max_v').innerHTML = val[1].split(",")[2];
    }   

    getStill.onclick = function (event) {  
      clearInterval(myTimer);  
      myTimer = setInterval(function(){error_handle();},5000);
      ShowImage.src=location.origin+'/?getstill='+Math.random();      

      myColor_r_min = document.getElementById('myColor_r_min').value;
      myColor_r_max = document.getElementById('myColor_r_max').value;
      myColor_g_min = document.getElementById('myColor_g_min').value;
      myColor_g_max = document.getElementById('myColor_g_max').value;
      myColor_b_min = document.getElementById('myColor_b_min').value;
      myColor_b_max = document.getElementById('myColor_b_max').value;
  
      tracking.ColorTracker.registerColor('custom', function(r, g, b) {
        if ((r>=myColor_r_min&&r<=myColor_r_max)&&(g>=myColor_g_min&&g<=myColor_g_max)&&(b>=myColor_b_min&&b<=myColor_b_max)) {
          return true;
        }
        return false;
      }); 
    }

    function error_handle() {
      restartCount++;
      clearInterval(myTimer);
      if (restartCount<=2) {
        result.innerHTML = "Get still error. <br>Restart ESP32-CAM "+restartCount+" times.";
        myTimer = setInterval(function(){getStill.click();},10000);
        ifr.src = document.location.origin+'?restart';
      }
      else
        result.innerHTML = "Get still error. <br>Please close the page and check ESP32-CAM.";
    }  

    ShowImage.onload = function (event) {
      clearInterval(myTimer);
      restartCount=0;      
      canvas.setAttribute("width", ShowImage.width);
      canvas.setAttribute("height", ShowImage.height);
      
      if (mirrorimage.value==1) {
        context.translate((canvas.width + ShowImage.width) / 2, 0);
        context.scale(-1, 1);
        context.drawImage(ShowImage, 0, 0, ShowImage.width, ShowImage.height);
        context.setTransform(1, 0, 0, 1, 0, 0);
      }
      else
        context.drawImage(ShowImage,0,0,ShowImage.width,ShowImage.height);
   
      tracking.track('#canvas', tracker);
      
      tracker.on('track', function(event) {
        result.innerHTML = "";
        event.data.forEach(function(rect) {
          if (rect.color === 'custom') {
            rect.color = tracker.customColor;
          }

          context.strokeStyle = rect.color;
          context.strokeRect(rect.x, rect.y, rect.width, rect.height);
          //context.font = '11px Helvetica';
          //context.fillStyle = "#fff";
          //context.fillText('x: ' + rect.x + 'px', rect.x + rect.width + 5, rect.y + 11);
          //context.fillText('y: ' + rect.y + 'px', rect.x + rect.width + 5, rect.y + 22);

          result.innerHTML+= rect.color+","+rect.x+","+rect.y+","+rect.width+","+rect.height+"<br>";
          if (rect.color==tracker.customColor&&lastValue!=tracker.customColor) {  //當偵測到自訂顏色時執行指令
            lastValue = tracker.customColor;
            /*
            var gpio = 4;
            var val = 10;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }
          else if (rect.color=="magenta"&&lastValue!="magenta") {
            lastValue = "magenta";
            /*
            var gpio = 4;
            var val = 10;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }
          else if (rect.color=="cyan"&&lastValue!="cyan") {
            lastValue = "cyan";
            /*
            var gpio = 4;
            var val = 10;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          } 
          else if (rect.color=="yellow"&&lastValue!="yellow") {
            lastValue = "yellow";
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }           
        });
      });

      initGUIControllers(tracker); 

      try { 
        document.createEvent("TouchEvent");
        setTimeout(function(){getStill.click();},250);
      }
      catch(e) { 
        setTimeout(function(){getStill.click();},150);
      }             
    }     
    
    restart.onclick = function (event) {
      fetch(location.origin+'?restart=stop');
    }    

    framesize.onclick = function (event) {
      fetch(document.location.origin+'?framesize='+this.value+';stop');
    }  

    flash.onchange = function (event) {
      fetch(location.origin+'?flash='+this.value+';stop');
    } 

    quality.onclick = function (event) {
      fetch(document.location.origin+'?quality='+this.value+';stop');
    } 

    brightness.onclick = function (event) {
      fetch(document.location.origin+'?brightness='+this.value+';stop');
    } 

    contrast.onclick = function (event) {
      fetch(document.location.origin+'?contrast='+this.value+';stop');
    }                             
    
    function getFeedback(target) {
      var data = $.ajax({
      type: "get",
      dataType: "text",
      url: target,
      success: function(response)
        {
          result.innerHTML = response;
        },
        error: function(exception)
        {
          result.innerHTML = 'fail';
        }
      });
    }      

    function initGUIControllers(tracker) {
  
      var trackedColors = {
        custom: true
      };
  
      Object.keys(tracking.ColorTracker.knownColors_).forEach(function(color) {
        trackedColors[color] = true;
      });
  
      tracker.customColor = myColor.value;
  
      function createCustomColor(value) {
        var components = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(value);
        var customColorR = parseInt(components[1], 16);
        var customColorG = parseInt(components[2], 16);
        var customColorB = parseInt(components[3], 16);
    
        var colorTotal = customColorR + customColorG + customColorB;
    
        if (colorTotal === 0) {
          tracking.ColorTracker.registerColor('custom', function(r, g, b) {
          return r + g + b < 10;
          });
        } else {
          var rRatio = customColorR / colorTotal;
          var gRatio = customColorG / colorTotal;
  
        tracking.ColorTracker.registerColor('custom', function(r, g, b) {
        var colorTotal2 = r + g + b;
  
        if (colorTotal2 === 0) {
          if (colorTotal < 10) {
          return true;
          }
          return false;
        }
  
        var rRatio2 = r / colorTotal2,
          gRatio2 = g / colorTotal2,
          deltaColorTotal = colorTotal / colorTotal2,
          deltaR = rRatio / rRatio2,
          deltaG = gRatio / gRatio2;
  
        return deltaColorTotal > 0.9 && deltaColorTotal < 1.1 &&
          deltaR > 0.9 && deltaR < 1.1 &&
          deltaG > 0.9 && deltaG < 1.1;
        });
      }
  
      updateColors();
      }
  
      function updateColors() {
      var colors = [];
  
      for (var color in trackedColors) {
        if (trackedColors[color]) {
        colors.push(color);
        }
      }
  
      tracker.setColors(colors);
      }
  
      updateColors();
    }
  
    var objectEmit_ = tracking.ObjectTracker.prototype.emit;
    var colorEmit_ = tracking.ColorTracker.prototype.emit;
  
    tracking.ObjectTracker.prototype.emit = function() {
      objectEmit_.apply(this, arguments);
    };
  
    tracking.ColorTracker.prototype.emit = function() {
      colorEmit_.apply(this, arguments);
    };  
  </script>   
)rawliteral";



void loop() {
  Feedback="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
   WiFiClient client = server.available();

  if (client) { 
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();             
        
        getCommand(c);   //將緩衝區取得的字元拆解出指令參數
                
        if (c == '\n') {
          if (currentLine.length() == 0) {    
            
            if (cmd=="getstill") {
              //回傳JPEG格式影像
              camera_fb_t * fb = NULL;
              fb = esp_camera_fb_get();  
              if(!fb) {
                Serial.println("Camera capture failed");
                delay(1000);
                ESP.restart();
              }
  
              client.println("HTTP/1.1 200 OK");
              client.println("Access-Control-Allow-Origin: *");              
              client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
              client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
              client.println("Content-Type: image/jpeg");
              client.println("Content-Disposition: form-data; name=\"imageFile\"; filename=\"picture.jpg\""); 
              client.println("Content-Length: " + String(fb->len));             
              client.println("Connection: close");
              client.println();
              
              uint8_t *fbBuf = fb->buf;
              size_t fbLen = fb->len;
              for (size_t n=0;n<fbLen;n=n+1024) {
                if (n+1024<fbLen) {
                  client.write(fbBuf, 1024);
                  fbBuf += 1024;
                }
                else if (fbLen%1024>0) {
                  size_t remainder = fbLen%1024;
                  client.write(fbBuf, remainder);
                }
              }  
              
              esp_camera_fb_return(fb);
            
              pinMode(4, OUTPUT);
              digitalWrite(4, LOW);               
            }
            else {
              //回傳HTML首頁或Feedback
              client.println("HTTP/1.1 200 OK");
              client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
              client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
              client.println("Content-Type: text/html; charset=utf-8");
              client.println("Access-Control-Allow-Origin: *");
              client.println("Connection: close");
              client.println();
              
              String Data="";
              if (cmd!="")
                Data = Feedback;
              else {
                Data = String((const char *)INDEX_HTML);
              }
              int Index;
              for (Index = 0; Index < Data.length(); Index = Index+1000) {
                client.print(Data.substring(Index, Index+1000));
              }           
              
              client.println();
            }
                        
            Feedback="";
            break;
          } else {
            currentLine = "";
          }
        } 
        else if (c != '\r') {
          currentLine += c;
        }

        if ((currentLine.indexOf("/?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1)) {
          if (Command.indexOf("stop")!=-1) {  //若指令中含關鍵字stop立即斷線 -> http://192.168.xxx.xxx/?cmd=aaa;bbb;ccc;stop
            client.println();
            client.println();
            client.stop();
          }
          currentLine="";
          Feedback="";
          ExecuteCommand();
        }
      }
    }
    delay(1);
    client.stop();
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

String tcp_http(String domain,String request,int port,byte wait)
{
    WiFiClient client_tcp;

    if (client_tcp.connect(domain.c_str(), port)) 
    {
      Serial.println("GET " + request);
      client_tcp.println("GET " + request + " HTTP/1.1");
      client_tcp.println("Host: " + domain);
      client_tcp.println("Connection: close");
      client_tcp.println();

      String getResponse="",Feedback="";
      boolean state = false;
      int waitTime = 3000;   // timeout 3 seconds
      long startTime = millis();
      while ((startTime + waitTime) > millis())
      {
        while (client_tcp.available()) 
        {
            char c = client_tcp.read();
            if (c == '\n') 
            {
              if (getResponse.length()==0) state=true; 
              getResponse = "";
            } 
            else if (c != '\r')
              getResponse += String(c);
            if (state==true) Feedback += String(c);
            if (wait==1)
              startTime = millis();
         }
         if (wait==0)
          if ((state==true)&&(Feedback.length()!= 0)) break;
      }
      client_tcp.stop();
      return Feedback;
    }
    else
      return "Connection failed";  
}

String tcp_https(String domain,String request,int port,byte wait)
{
    WiFiClientSecure client_tcp;

    if (client_tcp.connect(domain.c_str(), port)) 
    {
      Serial.println("GET " + request);
      client_tcp.println("GET " + request + " HTTP/1.1");
      client_tcp.println("Host: " + domain);
      client_tcp.println("Connection: close");
      client_tcp.println();

      String getResponse="",Feedback="";
      boolean state = false;
      int waitTime = 3000;   // timeout 3 seconds
      long startTime = millis();
      while ((startTime + waitTime) > millis())
      {
        while (client_tcp.available()) 
        {
            char c = client_tcp.read();
            if (c == '\n') 
            {
              if (getResponse.length()==0) state=true; 
              getResponse = "";
            } 
            else if (c != '\r')
              getResponse += String(c);
            if (state==true) Feedback += String(c);
            if (wait==1)
              startTime = millis();
         }
         if (wait==0)
          if ((state==true)&&(Feedback.length()!= 0)) break;
      }
      client_tcp.stop();
      return Feedback;
    }
    else
      return "Connection failed";  
}

String LineNotify(String token, String request, byte wait)
{
  request.replace(" ","%20");
  request.replace("&","%20");
  request.replace("#","%20");
  //request.replace("\'","%27");
  request.replace("\"","%22");
  request.replace("\n","%0D%0A");
  request.replace("%3Cbr%3E","%0D%0A");
  request.replace("%3Cbr/%3E","%0D%0A");
  request.replace("%3Cbr%20/%3E","%0D%0A");
  request.replace("%3CBR%3E","%0D%0A");
  request.replace("%3CBR/%3E","%0D%0A");
  request.replace("%3CBR%20/%3E","%0D%0A"); 
  request.replace("%20stickerPackageId","&stickerPackageId");
  request.replace("%20stickerId","&stickerId");    
  
  WiFiClientSecure client_tcp;
  
  if (client_tcp.connect("notify-api.line.me", 443)) 
  {
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("User-Agent: ESP8266/1.0");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    client_tcp.println(request);
    client_tcp.println();
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 3000;   // timeout 3 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis())
    {
      while (client_tcp.available()) 
      {
          char c = client_tcp.read();
          if (c == '\n') 
          {
            if (getResponse.length()==0) state=true; 
            getResponse = "";
          } 
          else if (c != '\r')
            getResponse += String(c);
          if (state==true) Feedback += String(c);
          if (wait==1)
            startTime = millis();
       }
       if (wait==0)
        if ((state==true)&&(Feedback.length()!= 0)) break;
    }
    client_tcp.stop();
    return Feedback;
  }
  else
    return "Connection failed";  
}

String sendCapturedImageToLineNotify(String token) 
{
  String getAll="", getBody = "";
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "";
  }  
      
  WiFiClientSecure client_tcp;
  Serial.println("Connect to notify-api.line.me");
  
  if (client_tcp.connect("notify-api.line.me", 443)) {
    Serial.println("Connection successful");

    String message = "Welcome to Taiwan.";
    String head = "--Taiwan\r\nContent-Disposition: form-data; name=\"message\"; \r\n\r\n" + message + "\r\n--Taiwan\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--Taiwan--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Length: " + String(totalLen));
    client_tcp.println("Content-Type: multipart/form-data; boundary=Taiwan");
    client_tcp.println();
    client_tcp.print(head);
    
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        client_tcp.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client_tcp.write(fbBuf, remainder);
      }
    }  
    
    client_tcp.print(tail);
    esp_camera_fb_return(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTime = millis();
    boolean state = false;
    while ((startTime + waitTime) > millis())
    {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) 
      {
          char c = client_tcp.read();
          if (c == '\n') 
          {
            if (getAll.length()==0) state=true; 
            getAll = "";
          } 
          else if (c != '\r')
            getAll += String(c);
          if (state==true) getBody += String(c);
          startTime = millis();
       }
       if (getBody.length()>0) break;
    }
    client_tcp.stop();
    //Serial.println(getAll); 
    Serial.println(getBody);
  }
  else {
    getAll="Connected to notify-api.line.me failed.";
    getBody="Connected to notify-api.line.me failed.";
    Serial.println("Connected to notify-api.line.me failed.");
  }
  
  //return getAll;
  return getBody;
}
