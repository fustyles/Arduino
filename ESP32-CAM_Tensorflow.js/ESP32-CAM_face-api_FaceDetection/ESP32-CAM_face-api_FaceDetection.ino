/*
ESP32-CAM
Open the page in Chrome.
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-1-5 01:00
https://www.facebook.com/francefu

自訂指令格式 :  
http://APIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

預設AP端IP： 192.168.4.1
http://192.168.xxx.xxx?ip
http://192.168.xxx.xxx?mac
http://192.168.xxx.xxx?restart
http://192.168.xxx.xxx?flash=value        //value= 0~255
      
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
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
  Serial.println("");
  
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
  else if (cmd=="flash") {
    ledcAttachPin(4, 4);  
    ledcSetup(4, 5000, 8);   
     
    int val = P1.toInt();
    ledcWrite(4,val);  
  }    
  else {
    Feedback="Command is not defined.";
  }
  if (Feedback=="") Feedback=Command;  
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
        <script src="https:\/\/code.jquery.com/jquery-3.3.1.min.js"></script>
        <script src='https:\/\/fustyles.github.io/webduino/TensorFlow/Face-api/face-api.min.js'></script>
        <style>
          #container {
            padding: 0;
          }
          #tools {
            position: absolute;
            top: 250px;
            left: 0;
          }
          #message {
            position: absolute;
            top: 320px;
            left: 0;            
            color: red;
          }          
          canvas {
            position: absolute;
            top: 0;
            left: 0;
            z-index:999;
          }
        </style>
    </head>
    <body>
    <div id="container"></div>
    <img id="stream" style="display:none"></img>
    <table id="tools">
      <tr><td><button id="get-still" onclick="ShowImage.src=document.location.origin+'/?getstill='+Math.random();">Restart</button></td><td></td><td></td></tr>
      <tr><td>Flash</td><td align="center" colspan="2"><input type="range" id="flash" min="0" max="255" value="0" onchange="try{fetch(document.location.origin+'?flash='+this.value);}catch(e){}"></td></tr>
      <tr style="display:none"><td colspan="3"><iframe id="ifr"></iframe></td></tr>
    </table>
    <span id="message">Please wait for loading model.</span>
 
    <!-- Face Detection -->    
    <script>
    var getStill = document.getElementById('get-still');
    var ShowImage = document.getElementById('stream');
    var message = document.getElementById('message');
    var ifr = document.getElementById('ifr');
    
    const modelPath = 'https://fustyles.github.io/webduino/TensorFlow/Face-api/';
    let currentStream;
    let displaySize = { width:320, height: 240 }
    let convas;
    let faceDetection;
    
    Promise.all([
      faceapi.nets.tinyFaceDetector.load(modelPath),
      faceapi.nets.faceLandmark68TinyNet.load(modelPath),
      faceapi.nets.faceRecognitionNet.load(modelPath),
      faceapi.nets.faceExpressionNet.load(modelPath),
      faceapi.nets.ageGenderNet.load(modelPath)
    ])

    message.innerHTML = "Start Detection";
    
    async function DetectImage() {
      const detections = await faceapi.detectAllFaces(ShowImage, new faceapi.TinyFaceDetectorOptions()).withFaceLandmarks(true).withFaceExpressions().withAgeAndGender()
      const resizedDetections = faceapi.resizeResults(detections, displaySize)
      canvas.getContext('2d').clearRect(0, 0, canvas.width, canvas.height)
      canvas.getContext('2d').drawImage(ShowImage,0,0,ShowImage.width,ShowImage.height); 
      faceapi.draw.drawDetections(canvas, resizedDetections)
      faceapi.draw.drawFaceLandmarks(canvas, resizedDetections)
      faceapi.draw.drawFaceExpressions(canvas, resizedDetections)
      resizedDetections.forEach(result => {
        const { age, gender, genderProbability } = result
        message.innerHTML = JSON.stringify(result);
        new faceapi.draw.DrawTextField(
          [
            `${faceapi.round(age, 0)} years`,
            `${gender} (${faceapi.round(genderProbability)})`
          ],
          result.detection.box.bottomRight
        ).draw(canvas)
      })
      getStill.click();      
    }
    
    ShowImage.onload = function (event) {
      try { 
        document.createEvent("TouchEvent");
        setTimeout(function(){DetectImage();},250);
      }
      catch(e) { 
        setTimeout(function(){DetectImage();},150);
      }       
    }
    
    window.onload = function () { 
      if( document.getElementsByTagName("canvas").length == 0 ) {
        canvas = faceapi.createCanvasFromMedia(ShowImage)
        document.getElementById('container').append(canvas)
        faceapi.matchDimensions(canvas, displaySize)
      }        
      getStill.click();
    }
    </script>
    </body>
</html>
)rawliteral";

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
  s->set_framesize(s, FRAMESIZE_QVGA);

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
              
              client.println();
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
