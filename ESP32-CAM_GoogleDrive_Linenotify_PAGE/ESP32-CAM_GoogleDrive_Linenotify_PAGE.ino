/*
ESP32-CAM 
Open the page in Chrome.
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-1-18 16:00
https://www.facebook.com/francefu

Servo1 -> gpio2 (common ground)
Servo2 -> gpio13 (common ground)

建立Google Apps Script，可應用於雲端硬碟、試算表等存取。設定權限任何能都能存取。
https://github.com/fustyles/webduino/blob/gs/SendCapturedImageToGoogleDriveAndLinenotify_doPost.gs

如何新增Script
https://www.youtube.com/watch?v=f46VBqWwUuI

Google Script管理介面
https://script.google.com/home
https://script.google.com/home/executions
Google雲端硬碟管理介面
https://drive.google.com/drive/my-drive

自訂指令格式 :  
http://APIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

預設AP端IP： 192.168.4.1
http://192.168.xxx.xxx/?ip
http://192.168.xxx.xxx/?mac
http://192.168.xxx.xxx/?restart
http://192.168.xxx.xxx/?flash=value        //value= 0 ~ 255
http://192.168.xxx.xxx/?servo1=value        //value= 1700 ~ 8000
http://192.168.xxx.xxx/?servo2=value        //value= 1700 ~ 8000
http://192.168.xxx.xxx/?getstill
http://192.168.xxx.xxx/?SendCapturedImage=stop
      
查詢Client端IP：
查詢IP：http://192.168.4.1/?ip
重設網路：http://192.168.4.1/?resetwifi=ssid;password
*/

//輸入WIFI連線帳號密碼
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

//輸入AP端連線帳號密碼
const char* apssid = "ESP32-CAM";
const char* appassword = "12345678";         //AP password require at least 8 characters.

String myScript = "/macros/s/**********/exec";    //Create your Google Apps Script and replace the "myScript" path.
String myLineNotifyToken = "myToken=**********";    //Line Notify Token. You can set the value of xxxxxxxxxx empty if you don't want to send picture to Linenotify.
String myFoldername = "&myFoldername=ESP32-CAM";
String myFilename = "&myFilename=ESP32-CAM.jpg";
String myImage = "&myFile=";

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"          //視訊
#include "Base64.h"              //用於轉換視訊影像格式為base64格式，易於上傳google雲端硬碟或資料庫
#include "soc/soc.h"             //用於電源不穩不重開機 
#include "soc/rtc_cntl_reg.h"    //用於電源不穩不重開機
#include <esp32-hal-ledc.h>      //伺服馬達

// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled

//CAMERA_MODEL_AI_THINKER  指定安可信ESP32-CAM模組腳位設定
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

//自訂網頁首頁
static const char PROGMEM INDEX_HTML[] = R"rawliteral(<!doctype html>
<html>
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width,initial-scale=1">
        <script src="https:\/\/ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js"></script> 
        <title>ESP32 OV2460</title>
        <style>
          body{font-family:Arial,Helvetica,sans-serif;background:#181818;color:#EFEFEF;font-size:16px}h2{font-size:18px}section.main{display:flex}#menu,section.main{flex-direction:column}#menu{display:none;flex-wrap:nowrap;min-width:340px;background:#363636;padding:8px;border-radius:4px;margin-top:-10px;margin-right:10px}#content{display:flex;flex-wrap:wrap;align-items:stretch}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}figure img{display:block;width:100%;height:auto;border-radius:4px;margin-top:8px}@media (min-width: 800px) and (orientation:landscape){#content{display:flex;flex-wrap:nowrap;align-items:stretch}figure img{display:block;max-width:100%;max-height:calc(100vh - 40px);width:auto;height:auto}figure{padding:0;margin:0;-webkit-margin-before:0;margin-block-start:0;-webkit-margin-after:0;margin-block-end:0;-webkit-margin-start:0;margin-inline-start:0;-webkit-margin-end:0;margin-inline-end:0}}section#buttons{display:flex;flex-wrap:nowrap;justify-content:space-between}#nav-toggle{cursor:pointer;display:block}#nav-toggle-cb{outline:0;opacity:0;width:0;height:0}#nav-toggle-cb:checked+#menu{display:flex}.input-group{display:flex;flex-wrap:nowrap;line-height:22px;margin:5px 0}.input-group>label{display:inline-block;padding-right:10px;min-width:47%}.input-group input,.input-group select{flex-grow:1}.range-max,.range-min{display:inline-block;padding:0 5px}button{display:block;margin:5px;padding:0 12px;border:0;line-height:28px;cursor:pointer;color:#fff;background:#ff3034;border-radius:5px;font-size:16px;outline:0}button:hover{background:#ff494d}button:active{background:#f21c21}button.disabled{cursor:default;background:#a0a0a0}input[type=range]{-webkit-appearance:none;width:100%;height:22px;background:#363636;cursor:pointer;margin:0}input[type=range]:focus{outline:0}input[type=range]::-webkit-slider-runnable-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-webkit-slider-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;-webkit-appearance:none;margin-top:-11.5px}input[type=range]:focus::-webkit-slider-runnable-track{background:#EFEFEF}input[type=range]::-moz-range-track{width:100%;height:2px;cursor:pointer;background:#EFEFEF;border-radius:0;border:0 solid #EFEFEF}input[type=range]::-moz-range-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer}input[type=range]::-ms-track{width:100%;height:2px;cursor:pointer;background:0 0;border-color:transparent;color:transparent}input[type=range]::-ms-fill-lower{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-fill-upper{background:#EFEFEF;border:0 solid #EFEFEF;border-radius:0}input[type=range]::-ms-thumb{border:1px solid rgba(0,0,30,0);height:22px;width:22px;border-radius:50px;background:#ff3034;cursor:pointer;height:2px}input[type=range]:focus::-ms-fill-lower{background:#EFEFEF}input[type=range]:focus::-ms-fill-upper{background:#363636}.switch{display:block;position:relative;line-height:22px;font-size:16px;height:22px}.switch input{outline:0;opacity:0;width:0;height:0}.slider{width:50px;height:22px;border-radius:22px;cursor:pointer;background-color:grey}.slider,.slider:before{display:inline-block;transition:.4s}.slider:before{position:relative;content:"";border-radius:50%;height:16px;width:16px;left:4px;top:3px;background-color:#fff}input:checked+.slider{background-color:#ff3034}input:checked+.slider:before{-webkit-transform:translateX(26px);transform:translateX(26px)}select{border:1px solid #363636;font-size:14px;height:22px;outline:0;border-radius:5px}.image-container{position:relative;min-width:160px}.close{position:absolute;right:5px;top:5px;background:#ff3034;width:16px;height:16px;border-radius:100px;color:#fff;text-align:center;line-height:18px;cursor:pointer}.hidden{display:none}
        </style>        
    </head>
    <body>
      <table>
      <tr><td colspan="3"><img id="stream" src=""></td></tr>
      <tr><td><button id="restart">Reatart</button></td><td><button id="get-still">Stream</button></td><td><button id="stop">Stop</button></td></tr>
      <tr><td></td><td></td><td><button id="send-image">Send</button></td></tr>
      <tr><td>Flash</td><td align="center" colspan="2"><input type="range" id="flash" min="0" max="255" value="0" onchange="try{fetch(document.location.origin+'/?flash='+this.value);}catch(e){}"></td></tr>
      <tr><td>Servo1</td><td align="center" colspan="2"><input type="range" id="servo1" min="1700" max="8000" step="35" value="4850" onchange="try{fetch(document.location.origin+'/?servo1='+this.value);}catch(e){}"></td></tr>
      <tr><td>Servo2</td><td align="center" colspan="2"><input type="range" id="servo2" min="1700" max="8000" step="35" value="4850" onchange="try{fetch(document.location.origin+'/?servo2='+this.value);}catch(e){}"></td></tr>
        <td>Resolution</td> 
        <td colspan="2">
          <select id="framesize" onclick="try{fetch(document.location.origin+'/?framesize='+this.value);}catch(e){}">
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
      <tr><td>Rotate</td><td align="left" colspan="2">
          <select onchange="document.getElementById('stream').style.transform='rotate('+this.value+')';">
            <option value="0deg">0deg</option>
            <option value="90deg">90deg</option>
            <option value="180deg">180deg</option>
            <option value="270deg">270deg</option>
          </select>
      </td></tr>
      </table><br>
      <span id="show" style="color:red"></span>
    </body>
    <script>
      var stream = document.getElementById('stream');
      var restart = document.getElementById('restart');
      var getStream = document.getElementById('get-still');
      var stopStream = document.getElementById('stop');      
      var sendimage = document.getElementById('send-image');
      var token = document.getElementById('token');
      var chat_id = document.getElementById('chat_id');
      var show = document.getElementById('show');
      var myTimer;
      var streamState = false;

      restart.onclick = function (event) {
        fetch(location.origin+'/?restart=stop');
      }      
      getStream.onclick = function (event) {
        streamState=true;
        stream.onload = function (event) {
          clearInterval(myTimer);
          if (streamState==true) {
            setTimeout(function(){getStream.click();},100);
            myTimer = setInterval(function(){getStream.click();},10000);
          }
          else
            stream.src="";
        }        
        stream.src=location.origin+'/?getstill='+Math.random();    
      }
  
      stopStream.onclick = function (event) {
        clearInterval(myTimer);
        streamState=false;  
        stream.onload = function (event) {}             
        stream.src="";
      }          
      sendimage.onclick = function (event) {
        show.innerHTML='Sending...';
        getFeedback(location.origin+'/?SendCapturedImage='+(new Date().getFullYear()*10000000000+(new Date().getMonth()+1)*100000000+new Date().getDate()*1000000+new Date().getHours()*10000+new Date().getMinutes()*100+new Date().getSeconds()+new Date().getSeconds()*0.001).toString());
      }      
      function getFeedback(target) {
        var data = $.ajax({
        type: "get",
        dataType: "text",
        url: target,
        success: function(response)
          {
            show.innerHTML = response; 
            if (streamState==true) getStream.onclick;
          },
          error: function(exception)
          {
            show.innerHTML = 'fail';
            if (streamState==true) getStream.onclick;
          }
        });
      }       
    </script>
</html>
)rawliteral";

WiFiServer server(80);

String Feedback="", Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
  Serial.println("");
  
  if (cmd=="your cmd") {
    // You can do anything
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
  else if (cmd=="restart") {
    ESP.restart();
  }    
  else if (cmd=="SendCapturedImage") {
    Feedback=SendCapturedImage();
    if (Feedback=="") Feedback="The image failed to send. <br>The framesize may be too large.";
  }
  else if (cmd=="servo1") {
    int val = P1.toInt();
    ledcAttachPin(2, 3);  
    ledcSetup(3, 50, 16);      
    if (val > 8000)
       val = 8000;
    else if (val < 1700)
      val = 1700;   
    val = 1700 + (8000 - val);   
    ledcWrite(3, val); 
  }  
  else if (cmd=="servo2") {
    int val = P1.toInt();
    ledcAttachPin(13, 5);  
    ledcSetup(5, 50, 16);      
    if (val > 8000)
       val = 8000;
    else if (val < 1700)
      val = 1700;   
    val = 1700 + (8000 - val);   
    ledcWrite(5, val); 
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
  s->set_framesize(s, FRAMESIZE_QVGA);  //UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

  //閃光燈(GPIO4)
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);

  //伺服馬達 Servo1
  ledcAttachPin(2, 3);  
  ledcSetup(3, 50, 16);
  ledcWrite(3, 4850);
  //伺服馬達 Servo2
  ledcAttachPin(13, 5);  
  ledcSetup(5, 50, 16);
  ledcWrite(5, 4850);
  
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

  server.begin();         
}

void loop() {
  Feedback="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
   WiFiClient client = server.available();

  if (client) { 
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();             
        
        getCommand(c);   //將緩衝區取得的字元猜解出指令參數
                
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
              client.println();
              esp_camera_fb_return(fb);
            
              pinMode(4, OUTPUT);
              digitalWrite(4, LOW);               
            }
            else {
              //回傳HTML格式
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

String SendCapturedImage() {
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
  
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "data:image/jpeg;base64,";
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    String Data = myLineNotifyToken+myFoldername+myFilename+myImage;
    
    client_tcp.println("POST " + myScript + " HTTP/1.1");
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
    Serial.println(getBody);
  }
  else {
    getBody="Connected to " + String(myDomain) + " failed.";
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }
  
  return getBody;
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
