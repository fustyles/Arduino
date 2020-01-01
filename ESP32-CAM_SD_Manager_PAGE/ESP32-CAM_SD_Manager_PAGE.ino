/*
ESP32-CAM (SD Card Manager)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-1-2 00:30
https://www.facebook.com/francefu

Arduino IDE settings
Partition Scheme : Huge APP (3MB No OTA/1MB SPIFFS)

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
http://192.168.xxx.xxx?flash=value        //value= 0~255
http://192.168.xxx.xxx?saveimage=/filename  //filename不含附檔名
http://192.168.xxx.xxx?listimages
http://192.168.xxx.xxx?showimage=/filename
http://192.168.xxx.xxx?deleteimage=/filename
http://192.168.xxx.xxx?framesize=size     //size= UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA (支援格式)

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
#include "FS.h"                 //file system wrapper
#include "SD_MMC.h"             //SD卡存取函式庫

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
  else if (cmd=="flash") {
    ledcAttachPin(4, 4);  
    ledcSetup(4, 5000, 8);   
     
    int val = P1.toInt();
    ledcWrite(4,val);  
  }  
  else if (cmd=="saveimage") {
    Feedback=saveCapturedImage(P1)+"<br>";
    Feedback+=ListImages(); 
  }  
  else if (cmd=="listimages") {
    Feedback=ListImages();
  }  
  else if (cmd=="deleteimage") {
    Feedback=deleteimage(P1)+"<br>"+ListImages(); 
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

//自訂網頁首頁管理介面
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <!DOCTYPE html>
  <head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <script src="https:\/\/ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js"></script>
  </head><body>
  <script>var myVar;</script>
  <table>
  <tr>
  <td><input type="button" id="get-still" value="Get Still" onclick="streamState=false;document.getElementById('showimage').src=document.location.origin+'/?getstill='+Math.random();"></td>
  <td><input type="button" id="get-stream" value="Start Stream" onclick="streamState=true;document.getElementById('stream').src=document.location.origin+'/?getstill='+Math.random();"></td>   
  <td><input type="button" id="stop" value="Stop Stream" onclick="streamState=false;"></td> 
  </tr>  
  <tr>
   <td><input type="button" value="Restart" onclick="execute(location.origin+'/?restart');"></td>
  <td><input type="button" value="Image List" onclick="streamState=false;execute(location.origin+'/?listimages');"></td>              
  <td><input type="button" value="Save Image" onclick="streamState=false;execute(location.origin+'/?saveimage='+(new Date().getFullYear()*10000000000+(new Date().getMonth()+1)*100000000+new Date().getDate()*1000000+new Date().getHours()*10000+new Date().getMinutes()*100+new Date().getSeconds()).toString());"></td>  
 </tr>
  <tr>
  <td>Flash</td>
  <td colspan="2"><input type="range" id="flash" min="0" max="255" value="0" onchange="try{fetch(document.location.origin+'/?flash='+this.value);}catch(e){}"></td>
  </tr>
  <tr>
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
  </table>  
  <br><img id="stream" src="">
  <div id="list">
  <img id="showimage" src="">   
  <br><span id="show"></span>
  </div>
  </body>
  </html> 
  
  <script>
    var stream = document.getElementById('stream');
    var getStream = document.getElementById('get-stream');
    var getStill = document.getElementById('get-still');
    var showimage = document.getElementById('showimage');
    var list = document.getElementById('list');
    var myTimer;
    var streamState = false;
    
    stream.onload = function (event) {
      clearTimeout(myTimer);
      if (streamState==true) {
        stream.style.display="block";
        setTimeout(function(){getStream.click();},100);
        myTimer = setTimeout(function(){getStream.click();},10000);
      }
      else
        stream.style.display="none";
    }

    function execute(target) {
      var data = $.ajax({
      type: "get",
      dataType: "text",
      url: target,
      success: function(response)
        {
          document.getElementById('show').innerHTML = response;
        },
        error: function(exception)
        {
          document.getElementById('show').innerHTML = 'fail';
        }
      });
    }    
    </script>  
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
  s->set_framesize(s, FRAMESIZE_QVGA);  //QQVGA|HQVGA|QVGA

  //SD Card偵測
  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD_MMC card attached");
    SD_MMC.end();
    return;
  }
  Serial.println("");
  Serial.print("SD_MMC Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }  
  Serial.printf("SD_MMC Card Size: %lluMB\n", SD_MMC.cardSize() / (1024 * 1024));
  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
  Serial.println();
  
  SD_MMC.end();   

  //Flash
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
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);         
    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());  

    for (int i=0;i<5;i++) {
      ledcWrite(4,10);
      delay(200);
      ledcWrite(4,0);
      delay(200);    
    }         
  }
  else {
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);   //設定SSID顯示客戶端IP           

    for (int i=0;i<2;i++) {
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
            else if (cmd=="showimage") {           
              //回傳SD卡影像檔案
              if(!SD_MMC.begin()){
                Serial.println("Card Mount Failed");
              }  
              
              fs::FS &fs = SD_MMC;
              File file = fs.open(P1);
              if(!file){
                Serial.println("Failed to open file for reading");
                SD_MMC.end();    
              }
              else {
                Serial.println("Read from file: "+P1);
                Serial.println("file size: "+String(file.size()));            

                client.println("HTTP/1.1 200 OK");
                client.println("Access-Control-Allow-Origin: *");              
                client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
                client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
                client.println("Content-Type: image/jpeg");
                client.println("Content-Disposition: form-data; name=\"imageFile\"; filename=\"picture.jpg\""); 
                client.println("Content-Length: " + String(file.size()));             
                client.println("Connection: close");
                client.println();

                byte buf[1024];
                int i = -1;
                while (file.available()) {
                  i++;
                  buf[i] = file.read();
                  if (i==(sizeof(buf)-1)) {
                    client.write((const uint8_t *)buf, sizeof(buf));
                    i = -1;
                  }
                  else if (!file.available())
                    client.write((const uint8_t *)buf, (i+1));
                }
        
                client.println();

                file.close();
                SD_MMC.end();
              }
            
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

//列出TF卡中照片清單
String ListImages() {
  //SD Card
  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return "Card Mount Failed";
  }  
  
  fs::FS &fs = SD_MMC; 
  File root = fs.open("/");
  if(!root){
    Serial.println("Failed to open directory");
    return "Failed to open directory";
  }

  String list="";
  File file = root.openNextFile();
  while(file){
    if(!file.isDirectory()){
      String filename=String(file.name());
      if (filename=="/"+P1+".jpg")
        list = "<tr><td><button onclick=\'execute(location.origin+\"/?deleteimage="+String(file.name())+"\");\'>Delete</button></td><td bgcolor=\"yellow\"><a onclick=\'document.getElementById(\"showimage\").src=location.origin+\"/?showimage="+String(file.name())+"\";\'>"+String(file.name())+"</a></td><td align=\'right\'>"+String(file.size())+" B</td><td><button onclick=\'location.href=location.origin+\"/?showimage="+String(file.name())+"\";\'>download</button></td></tr>"+list;
      else
        list = "<tr><td><button onclick=\'execute(location.origin+\"/?deleteimage="+String(file.name())+"\");\'>Delete</button></td><td><a onclick=\'document.getElementById(\"showimage\").src=location.origin+\"/?showimage="+String(file.name())+"\";\'>"+String(file.name())+"</a></td><td align=\'right\'>"+String(file.size())+" B</td><td><button onclick=\'location.href=location.origin+\"/?showimage="+String(file.name())+"\";\'>download</button></td></tr>"+list;        
    }
    file = root.openNextFile();
  }
  if (list=="") list="<tr><td>null</td><td>null</td><td>null</td><td>null</td></tr>";
  list="<table border=1>"+list+"</table>";

  file.close();
  SD_MMC.end();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);   
  
  return list;
}

//刪除TF卡指定檔名照片
String deleteimage(String filename) {
  //SD Card
  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return "Card Mount Failed";
  }  
  
  fs::FS &fs = SD_MMC;
  File file = fs.open(filename);
  String message="";
  if(fs.remove(filename)){
      message=filename + " File deleted";
  } else {
      message=filename + " Delete failed";
  }
  file.close();
  SD_MMC.end();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);   

  return message;
}

//儲存即時影像至TF卡
String saveCapturedImage(String filename) {    
  String response = ""; 
  String path_jpg = "/"+filename+".jpg";

  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return "Camera capture failed";
  }

  //SD Card
  if(!SD_MMC.begin()){
    response = "Card Mount Failed";
    return "Card Mount Failed";
  }  
  
  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path_jpg.c_str());

  File file = fs.open(path_jpg.c_str(), FILE_WRITE);
  if(!file){
    esp_camera_fb_return(fb);
    SD_MMC.end();
    return "Failed to open file in writing mode";
  } 
  else {
    file.write(fb->buf, fb->len);
    Serial.printf("Saved file to path: %s\n", path_jpg.c_str());
  }
  file.close();
  SD_MMC.end();

  esp_camera_fb_return(fb);
  Serial.println("");

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);  
  
  return response;
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
