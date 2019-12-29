/*
ESP32-CAM MyBlockly (可跨網域連線)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-12-29 21:00
https://www.facebook.com/francefu

馬達驅動IC -> PWM1(gpio12, gpio13), PWM2(gpio14, gpio15)
伺服馬達 -> gpio2  (or)  Servo1 -> gpio2, Servo2 -> gpio13

Arduino IDE settings
Partition Scheme : Huge APP (3MB No OTA/1MB SPIFFS)

http://APIP
http://STAIP

自訂指令格式 :  
http://APIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

預設AP端IP： 192.168.4.1
http://192.168.xxx.xxx/?ip
http://192.168.xxx.xxx/?mac
http://192.168.xxx.xxx/?restart
http://192.168.xxx.xxx/?resetwifi=ssid;password
http://192.168.xxx.xxx/?inputpullup=pin
http://192.168.xxx.xxx/?pinmode=pin;value
http://192.168.xxx.xxx/?digitalwrite=pin;value
http://192.168.xxx.xxx/?analogwrite=pin;value
http://192.168.xxx.xxx/?digitalread=pin
http://192.168.xxx.xxx/?analogread=pin
http://192.168.xxx.xxx/?touchread=pin
http://192.168.xxx.xxx/?tcp=domain;port;request;wait
--> wait = 0 or 1  (持續等待回應)
--> request = /xxxx/xxxx
http://192.168.xxx.xxx/?ifttt=event;key;value1;value2;value3
http://192.168.xxx.xxx/?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8
http://192.168.xxx.xxx/?thingspeakread=request
--> request = /channels/xxxxx/fields/1.json?results=1
http://192.168.xxx.xxx/?linenotify=token;request
--> request = message=xxxxx
--> request = message=xxxxx&stickerPackageId=xxxxx&stickerId=xxxxx
http://192.168.xxx.xxx/?flash=value        //vale= 0~255  (閃光燈)
http://192.168.xxx.xxx/?servo=value        //vale= 1700~8000   伺服馬達(gpio2)
http://192.168.xxx.xxx/?servo1=value       //vale= 1700~8000   伺服馬達1(gpio2)
http://192.168.xxx.xxx/?servo2=value       //vale= 1700~8000   伺服馬達2(gpio13)
http://192.168.xxx.xxx/?speedL=value       //vale= 0~255  (左輪速度)
http://192.168.xxx.xxx/?speedR=value       //vale= 0~255  (右輪速度)
http://192.168.xxx.xxx/?decelerate=value   //vale= 0~100  (轉彎輪子減速後的速度為原速的百分比%)
http://192.168.xxx.xxx/?car=state          //state= 1(前進),2(左轉),3(停止),4(右轉),5(後退),6(左前轉),7(右前轉),8(左後轉),9(右後轉)
http://192.168.xxx.xxx/?getstill           //回傳base64格式文字
http://192.168.xxx.xxx/?getstill=img       //<img id='gameimage_getstill' src='http://192.168.xxx.xxx/?getstill=img'> 回傳影像
http://192.168.xxx.xxx/?downloadstill      //影像檔案下載
http://192.168.xxx.xxx/?framesize=size     //size= UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA (支援格式)
http://192.168.xxx.xxx/?sendCapturedImageToLineNotify=token  //傳送影像截圖至LineNotify


查詢Client端IP：
查詢IP：http://192.168.4.1/?ip
重設網路：http://192.168.4.1/?resetwifi=ssid;password

如果想快速執行指令不需等待回傳值，可在命令中增加參數值為stop。例如：
http://192.168.xxx.xxx/?digitalwrite=gpio;value;stop
http://192.168.xxx.xxx/?restart=stop
*/

//輸入WIFI連線帳號密碼
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

//輸入AP端連線帳號密碼
const char* apssid = "ESP32-CAM";
const char* appassword = "12345678";         //AP端密碼至少要八個字元以上

int speedR = 255;  //右輪初速 (gpio12, gpio13)
int speedL = 255;  //左輪初速 (gpio14, gpio15)
int servoPin = 2;
int servo1Pin = 2;
int servo2Pin = 13;
double decelerate = 60;

#include <WiFi.h>
#include <WiFiClientSecure.h>    //用於https加密傳輸協定
#include <esp32-hal-ledc.h>      //用於控制伺服馬達
#include "esp_camera.h"          //視訊
#include "Base64.h"              //用於轉換視訊影像格式為base64格式，易於上傳google雲端硬碟或資料庫
#include "soc/soc.h"             //用於電源不穩不重開機       
#include "soc/rtc_cntl_reg.h"    //用於電源不穩不重開機

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

WiFiServer server(80);  //伺服器通信協定的埠號 80

String Feedback="";   //回傳客戶端訊息
String Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";   //指令參數值
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;  //指令拆解狀態值

void ExecuteCommand()
{
  Serial.println("");
  Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);    //http://192.168.xxx.xxx?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
  Serial.println("");

  //自訂指令區塊
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
  else if (cmd=="restart") {
    ESP.restart();
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
  else if (cmd=="inputpullup") {
    pinMode(P1.toInt(), INPUT_PULLUP);
  }  
  else if (cmd=="pinmode") {
    if (P2.toInt()==1)
      pinMode(P1.toInt(), OUTPUT);
    else
      pinMode(P1.toInt(), INPUT);
  }        
  else if (cmd=="digitalwrite") {
    ledcDetachPin(P1.toInt());
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(), P2.toInt());
  }   
  else if (cmd=="digitalread") {
    Feedback=String(digitalRead(P1.toInt()));
  }
  else if (cmd=="analogwrite") {
    if (P1=="2") {
      ledcAttachPin(2, 3);  
      ledcSetup(3, 5000, 8);
      ledcWrite(3,P2.toInt());     
    }    
    else if (P1=="4") {
      ledcAttachPin(4, 4);  
      ledcSetup(4, 5000, 8);
      ledcWrite(4,P2.toInt());     
    }
    else if (P1=="12") {
      ledcAttachPin(12, 5);  
      ledcSetup(5, 5000, 8);
      ledcWrite(5,P2.toInt());     
    }
    else if (P1=="13") {
      ledcAttachPin(13, 6);  
      ledcSetup(6, 5000, 8);
      ledcWrite(6,P2.toInt());     
    }
    else if (P1=="14") {
      ledcAttachPin(14, 8);  
      ledcSetup(8, 5000, 8);
      ledcWrite(8,P2.toInt());     
    }
    else if (P1=="15") {
      ledcAttachPin(15, 7);  
      ledcSetup(7, 5000, 8);
      ledcWrite(7,P2.toInt());     
    }
    else {
      ledcAttachPin(P1.toInt(), 9);
      ledcSetup(9, 5000, 8);
      ledcWrite(9,P2.toInt());
    } 
  }       
  else if (cmd=="analogread") {
    Feedback=String(analogRead(P1.toInt()));
  }
  else if (cmd=="touchread") {
    Feedback=String(touchRead(P1.toInt()));
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
  else if (cmd=="ifttt") {
    String domain="maker.ifttt.com";
    String request = "/trigger/" + P1 + "/with/key/" + P2;
    request += "?value1="+P3+"&value2="+P4+"&value3="+P5;
    Feedback=tcp_https(domain,request,443,0);
  }
  else if (cmd=="thingspeakupdate") {
    String domain="api.thingspeak.com";
    String request = "/update?api_key=" + P1;
    request += "&field1="+P2+"&field2="+P3+"&field3="+P4+"&field4="+P5+"&field5="+P6+"&field6="+P7+"&field7="+P8+"&field8="+P9;
    Feedback=tcp_https(domain,request,443,0);
  }    
  else if (cmd=="thingspeakread") {
    String domain="api.thingspeak.com";
    String request = P1;
    Feedback=tcp_https(domain,request,443,1);
    int s=Feedback.indexOf("feeds");
    Feedback=Feedback.substring(s+8);
    int e=Feedback.indexOf("]");
    Feedback=Feedback.substring(0,e);
    Feedback.replace("},{",";");
    Feedback.replace("\":\"",",");
    Feedback.replace("\":",",");
    Feedback.replace("\",\"",","); 
    Feedback.replace("\"","");
    Feedback.replace("{","");
    Feedback.replace("}","");
    Feedback.replace("[","");
    Feedback.replace("]","");
    Feedback.replace(",\"",",");
    Feedback.replace("\":",",");
  } 
  else if (cmd=="linenotify") {
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
  else if (cmd=="flash") {
    ledcAttachPin(4, 4);  
    ledcSetup(4, 5000, 8);   
     
    int val = P1.toInt();
    ledcWrite(4,val);
  }  
  else if (cmd=="servo") {
    ledcAttachPin(servoPin, 3);  
    ledcSetup(3, 50, 16);
    
    int val = P1.toInt();      
    if (val > 8000)
       val = 8000;
    else if (val < 1700)
      val = 1700;   
    val = 1700 + (8000 - val);   
    ledcWrite(3, val); 
  }  
  else if (cmd=="servo1") {
    ledcAttachPin(servo1Pin, 3);  
    ledcSetup(3, 50, 16); 
    
    int val = P1.toInt();     
    if (val > 8000)
       val = 8000;
    else if (val < 1700)
      val = 1700;   
    val = 1700 + (8000 - val);   
    ledcWrite(3, val); 
  }  
  else if (cmd=="servo2") {
    ledcAttachPin(servo2Pin, 6);  
    ledcSetup(6, 50, 16); 
    
    int val = P1.toInt();     
    if (val > 8000)
       val = 8000;
    else if (val < 1700)
      val = 1700;   
    val = 1700 + (8000 - val);   
    ledcWrite(6, val); 
  }
  else if (cmd=="speedL") {
    int val = P1.toInt();
    if (val > 255)
       val = 255;
    else if (val < 0)
      val = 0;       
    speedL = val;
  }  
  else if (cmd=="speedR") {
    int val = P1.toInt();
    if (val > 255)
       val = 255;
    else if (val < 0)
      val = 0;       
    speedR = val;
  }  
  else if (cmd=="decelerate") { 
    int val = P1.toInt();      
    decelerate = val;
  }   
  else if (cmd=="car") {
    ledcAttachPin(12, 5);
    ledcSetup(5, 2000, 8);      
    ledcAttachPin(13, 6);
    ledcSetup(6, 2000, 8);
    ledcWrite(6, 0);  
    ledcAttachPin(15, 7);
    ledcSetup(7, 2000, 8);      
    ledcAttachPin(14, 8);
    ledcSetup(8, 2000, 8); 
      
    int val = P1.toInt();
    if (val==1) {
      Serial.println("Front");     
      ledcWrite(5,speedR);
      ledcWrite(6,0);
      ledcWrite(7,speedL);
      ledcWrite(8,0);   
    }
    else if (val==2) {
      Serial.println("Left");     
      ledcWrite(5,speedR*decelerate/100);
      ledcWrite(6,0);
      ledcWrite(7,0);
      ledcWrite(8,speedL*decelerate/100);  
    }
    else if (val==3) {
      Serial.println("Stop");      
      ledcWrite(5,0);
      ledcWrite(6,0);
      ledcWrite(7,0);
      ledcWrite(8,0);    
    }
    else if (val==4) {
      Serial.println("Right");
      ledcWrite(5,0);
      ledcWrite(6,speedR*decelerate/100);
      ledcWrite(7,speedL*decelerate/100);
      ledcWrite(8,0);          
    }
    else if (val==5) {
      Serial.println("Back");      
      ledcWrite(5,0);
      ledcWrite(6,speedR);
      ledcWrite(7,0);
      ledcWrite(8,speedL);
    }  
    else if (val==6) {
      Serial.println("FrontLeft");     
      ledcWrite(5,speedR);
      ledcWrite(6,0);
      ledcWrite(7,speedL*decelerate/100);
      ledcWrite(8,0);   
    }
    else if (val==7) {
      Serial.println("FrontRight");     
      ledcWrite(5,speedR*decelerate/100);
      ledcWrite(6,0);
      ledcWrite(7,speedL);
      ledcWrite(8,0);   
    }  
    else if (val==8) {
      Serial.println("LeftAfter");      
      ledcWrite(5,0);
      ledcWrite(6,speedR);
      ledcWrite(7,0);
      ledcWrite(8,speedL*decelerate/100);
    } 
    else if (val==9) {
      Serial.println("RightAfter");      
      ledcWrite(5,0);
      ledcWrite(6,speedR*decelerate/100);
      ledcWrite(7,0);
      ledcWrite(8,speedL);
    }       
  }   
  else if (cmd=="getstill") { 
    if (P1=="img")
      Feedback="\<img id=\'gameimage_getstill\' src=\'"+getstill()+"\'\>";
    else
      Feedback=getstill();
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
  else if (cmd=="sendCapturedImageToLineNotify") { 
    Feedback=sendCapturedImageToLineNotify(P1);
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
  s->set_framesize(s, FRAMESIZE_CIF);  //UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

  //閃光燈(GPIO4)
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8); 

  //馬達驅動IC腳位初始化
  ledcAttachPin(12, 5);
  ledcSetup(5, 2000, 8);      
  ledcAttachPin(13, 6);
  ledcSetup(6, 2000, 8);
  ledcWrite(6, 0);  
  ledcAttachPin(15, 7);
  ledcSetup(7, 2000, 8);      
  ledcAttachPin(14, 8);
  ledcSetup(8, 2000, 8);    
  
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

  //設定閃光燈為低電位
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);      
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
            /*  
            //回傳JSON格式
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: application/json;charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            //client.println("Connection: close");
            client.println();
            client.println("[{\"esp32\":\""+Feedback+"\"}]");
            client.println();
            */
            
            /*
            //回傳XML格式
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: text/xml; charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            //client.println("Connection: close");
            client.println();
            client.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            client.println("<esp32><feedback>"+Feedback+"</feedback></esp32>");
            client.println();
            */

            /*
            //回傳TEXT格式
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: text/plain; charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            client.println("Connection: close");
            client.println();
            client.println(Feedback);
            client.println();
            */

            if (cmd=="downloadstill") {
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
              client.println("<!DOCTYPE HTML>");
              client.println("<html><head>");
              client.println("<meta charset=\"UTF-8\">");
              client.println("<meta http-equiv=\"Access-Control-Allow-Origin\" content=\"*\">");
              client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
              client.println("</head><body>");
              client.println(Feedback);
              client.println("</body></html>");
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

String getstill() { 
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return "Camera capture failed";
  }

  String imageFile = "data:image/jpeg;base64,";
  char *input = (char *)fb->buf;
  char output[base64_enc_len(3)];
  for (int i=0;i<fb->len;i++) {
    base64_encode(output, (input++), 3);
    if (i%3==0) imageFile += urlencode(String(output));
  }
  esp_camera_fb_return(fb);

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW); 
  
  return imageFile;
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

String sendCapturedImageToLineNotify(String myLineNotifyToken)
{
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
  
  if (client_tcp.connect("notify-api.line.me", 443)) 
  {
    Serial.println("Connection successful");
    
    String message = "Hello World";
    String head = "--Taiwan\r\nContent-Disposition: form-data; name=\"message\"; \r\n\r\n" + message + "\r\n--Taiwan\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--Taiwan--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("Authorization: Bearer " + myLineNotifyToken);
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
    client_tcp.println();
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 3000;   // timeout 3 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis())
    {
      Serial.print(".");
      delay(100);      
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
          startTime = millis();
       }
    }
    client_tcp.stop();
    esp_camera_fb_return(fb);
    Serial.println(Feedback);
    return Feedback;
  }
  else {
    Serial.println("Connected to notify-api.line.me failed.");
    esp_camera_fb_return(fb);
    return "Connected to notify-api.line.me failed.";
  }
}
