/*
ESP32-CAM 自訂指令
Author : ChungYi Fu (Kaohsiung, Taiwan)  2021-7-14 22:00
https://www.facebook.com/francefu

首頁
http://APIP
http://STAIP

自訂指令格式 :  
http://APIP?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

預設AP端IP： 192.168.4.1

查詢Client端IP：
查詢IP：http://192.168.4.1?ip
重設網路：http://192.168.4.1?resetwifi=ssid;password

自訂指令
http://192.168.xxx.xxx?ip                      //取得APIP, STAIP
http://192.168.xxx.xxx?mac                     //取得MAC位址
http://192.168.xxx.xxx?digitalwrite=pin;value  //數位輸出
http://192.168.xxx.xxx?analogwrite=pin;value   //類比輸出
http://192.168.xxx.xxx?digitalread=pin         //數位讀取
http://192.168.xxx.xxx?analogread=pin          //類比讀取
http://192.168.xxx.xxx?touchread=pin           //觸碰讀取
http://192.168.xxx.xxx?restart                 //重啟電源
http://192.168.xxx.xxx?flash=value             //閃光燈 value= 0~255
http://192.168.xxx.xxx?servo=pin;value             //伺服馬達 value= 0~180
http://192.168.xxx.xxx?relay=pin;value             //繼電器 value = 0, 1
http://192.168.xxx.xxx?quality=value           //畫質 value = 10 to 63
http://192.168.xxx.xxx?brightness=value        //亮度 value = -2 to 2
http://192.168.xxx.xxx?contrast=value          //對比 value = -2 to 2 
http://192.168.xxx.xxx?hmirror=value           //水平鏡像 value = 0 or 1 
http://192.168.xxx.xxx?vflip=value             //垂直翻轉 value = 0 or 1
http://192.168.xxx.xxx?getstill                //取得視訊截圖
http://192.168.xxx.xxx?framesize=size          //解析度 value = 10->UXGA(1600x1200), 9->SXGA(1280x1024), 8->XGA(1024x768) ,7->SVGA(800x600), 6->VGA(640x480), 5->CIF(400x296), 4->QVGA(320x240), 3->HQVGA(240x176), 0->QQVGA(160x120) 改變影像解析度
*/

const char* ssid = "teacher";        //WIFI連線帳號
const char* password = "87654321";   //WIFI連線密碼 (至少8碼)

//新增AP 連線帳號密碼 (首頁網址 http://192.168.4.1)
const char* apssid = "ESP32-CAM";          //不同台可設不同流水號區別
const char* appassword = "12345678";         //AP密碼至少要8個字元以上

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"         //視訊
#include "soc/soc.h"            //用於電源不穩不重開機
#include "soc/rtc_cntl_reg.h"   //用於電源不穩不重開機

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

String Feedback = "";

WiFiServer server(80);
WiFiClient client;

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
  s->set_framesize(s, FRAMESIZE_QVGA);    //解析度 UXGA(1600x1200), SXGA(1280x1024), XGA(1024x768), SVGA(800x600), VGA(640x480), CIF(400x296), QVGA(320x240), HQVGA(240x176), QQVGA(160x120), QXGA(2048x1564 for OV3660)

  //s->set_vflip(s, 1);    //設定垂直翻轉
  //s->set_hmirror(s, 1);  //設定水平鏡像

  //新增閃光燈
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);    

  //新增ssid顯示IP
  if (ssid!="") {
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

  //設定閃光燈為低電位
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); 

  server.begin();  
}

void loop() {
  listenConnection();
}

//執行自訂指令
void ExecuteCommand() {
  //Serial.println("");
  //Serial.println("Command: "+Command);
  if (cmd!="getstill") {
    Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
    Serial.println("");
  }

  //自訂指令區塊  http://192.168.xxx.xxx?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
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
  else if (cmd=="restart") {  //重設WIFI連線
    ESP.restart();
  }  
  else if (cmd=="digitalwrite") {  //數位輸出
    ledcDetachPin(P1.toInt());
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(), P2.toInt());
  }   
  else if (cmd=="digitalread") {  //數位輸入
    Feedback=String(digitalRead(P1.toInt()));
  }
  else if (cmd=="analogwrite") {  //類比輸出
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
  else if (cmd=="analogread") {  //類比讀取
    Feedback=String(analogRead(P1.toInt()));
  }
  else if (cmd=="touchread") {  //觸碰讀取
    Feedback=String(touchRead(P1.toInt()));
  }  
  else if (cmd=="framesize") {  //解析度
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt();
    s->set_framesize(s, (framesize_t)val);   
  }
  else if (cmd=="quality") {  //畫質 
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_quality(s, val);
  }
  else if (cmd=="contrast") {  //對比
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_contrast(s, val);
  }
  else if (cmd=="brightness") {  //亮度
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt();  
    s->set_brightness(s, val);  
  } 
  else if (cmd=="hmirror") {  //水平鏡像
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt();  
    s->set_hmirror(s, val); 
  }   
  else if (cmd=="vflip") {  //垂直翻轉
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt();  
    s->set_vflip(s, val);
  }
  else if (cmd=="serial") {  //序列埠
    Serial.print(P1);
  }  
  else if (cmd=="restart") {  //重啟電源
    ESP.restart();
  }         
  else if (cmd=="flash") {  //閃光燈
    ledcAttachPin(4, 4);  
    ledcSetup(4, 5000, 8);   
    int val = P1.toInt();
    ledcWrite(4,val);  
  }
  else if(cmd=="servo") {  //伺服馬達 (0-180)
    ledcAttachPin(P1.toInt(), 3);
    ledcSetup(3, 50, 16);
     
    int val = 7864-P2.toInt()*34.59; 
    if (val > 7864)
       val = 7864;
    else if (val < 1638)
      val = 1638; 
    ledcWrite(3, val);
  }
  else if (cmd=="relay") {  //繼電器
    pinMode(P1.toInt(), OUTPUT);  
    digitalWrite(13, P2.toInt());  
  }       
  else {
    Feedback="Command is not defined.";
  }  
  
  if (Feedback=="") Feedback=Command;  
}


//自訂網頁首頁管理介面
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <!DOCTYPE html>
  <head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  </head><body>
  <img id="ShowImage" src=""> 
  <table>
  <tr>
    <td><input type="button" id="restart" value="Restart"></td> 
    <td><input type="button" id="getStop" value="Stop Stream"></td>
    <td><input type="button" id="getStill" value="Stream"></td> 
  </tr>
  <tr>
    <td>MirrorImage</td> 
    <td colspan="2">
      <input type="checkbox" id="mirrorimage">
    </td>
  </tr>   
  <tr>
    <td>Resolution</td> 
    <td colspan="2">
      <select id="framesize">
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
    <td>Rotate</td>
    <td align="left" colspan="2">
        <input type="range" id="rotate" min="0" max="360" value="0">
    </td>
  </tr>  
  </table>
  <div id="message" style="color:red"><div>

  </body>
  </html> 
  
  <script>
    var getStill = document.getElementById('getStill');
    var getStop = document.getElementById('getStop');
    var ShowImage = document.getElementById('ShowImage');
    var mirrorimage = document.getElementById("mirrorimage");  
    var flash = document.getElementById('flash'); 
    var restartCount=0;
    var myTimer;

    getStill.onclick = function (event) {
      myTimer = setInterval(function(){
        ShowImage.src=location.origin+'?getstill='+Math.random();
      },500);
    }

    getStop.onclick = function (event) {
      clearInterval(myTimer); 
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

    rotate.onclick = function (event) {
       document.getElementById('ShowImage').style.transform='rotate('+this.value+'deg)';
    }     

    mirrorimage.onchange = function (event) {
      fetch(location.origin+'?hmirror='+Number(this.checked)+';stop');
    }        
  </script>   
)rawliteral";

void listenConnection() {
  Feedback="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
  client = server.available();
  
  if (client) { 
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read(); 
        getCommand(c);   //將緩衝區取得的字元拆解出指令參數
        if (c == '\n') {
          if (currentLine.length() == 0) {
            if (cmd=="getstill") {
              getStill();            
            } else {
              mainPage();
            }         
            Feedback="";
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if ((currentLine.indexOf("?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1)) {
          if (Command.indexOf("stop")!=-1) {  //若指令中含關鍵字stop立即斷線 -> http://192.168.xxx.xxx?cmd=aaa;bbb;ccc;stop
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

void mainPage() {
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
}

void getStill() {
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

//拆解命令字串置入變數
void getCommand(char c) {
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
