/*
ESP32-CAM (SD Card Manager)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-11-27 01:00
https://www.facebook.com/francefu
http://APIP
http://STAIP

Command Format :  
http://APIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
*/

// Enter your WiFi ssid and password
const char* ssid     = "xxxxx";   //your network SSID
const char* password = "xxxxx";   //your network password

const char* apssid = "ESP32-CAM";
const char* appassword = "12345678";         //AP password require at least 8 characters.

#include <WiFi.h>
#include "esp_camera.h"
#include "Base64.h"
#include "FS.h"
#include "SD_MMC.h"
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled

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

WiFiServer server(80);

String Feedback="", Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
  Serial.println("");

  if (cmd=="your cmd")
  {
    // You can do anything
    // Feedback="<font color=\"red\">Hello World</font>";
  }   
  else if (cmd=="restart")
  { 
    ESP.restart();
  }    
  else if (cmd=="saveimage")
  {
    Feedback=saveCapturedImage(P1)+"<br>"+showimage("/"+P1+".html")+"<br>"+ListImages(); 
  } 
  else if (cmd=="getstill")
  { 
    Feedback=getstill();
  }   
  else if (cmd=="listimages")
  {
    Feedback=ListImages();
  }  
  else if (cmd=="showimage")
  {
    Feedback=P1+"<br>"+showimage(P1)+"<br>"+ListImages(); 
  }  
  else if (cmd=="deleteimage")
  {
    Feedback=deleteimage(P1)+"<br>"+ListImages(); 
  }     
  else {
    Feedback="Command is not defined.";
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
  Serial.begin(115200);
  delay(10);

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
  config.frame_size = FRAMESIZE_CIF;  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
  }

  //SD Card
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
  
  
  WiFi.mode(WIFI_AP_STA);

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  WiFi.begin(ssid, password);

  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+10000) < millis()) break;
  } 

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);         
    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP()); 
  }
  else {
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);    
  }     
    
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());    
  
  server.begin();  

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
        
        getCommand(c);
                
        if (c == '\n') {
          if (currentLine.length() == 0) {    
            if (Feedback=="") {
              Feedback+="<!DOCTYPE html>";
              Feedback+="<head>";
              Feedback+="<meta charset=\"utf-8\">";
              Feedback+="<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
              Feedback+="<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js\"></script>";
              Feedback+="</head><body>";
              Feedback+="<script>var myVar;</script>";
              Feedback+="<table><tr>";
              Feedback+="<td><input type=\"button\" value=\"Restart\" onclick=\"if (myVar) clearInterval(myVar);execute(location.origin+\'?restart\');\"></td>";
              Feedback+="<td><input type=\"button\" value=\"Image List\" onclick=\"if (myVar) clearInterval(myVar);execute(location.origin+\'?listimages\');\"></td>";              
              Feedback+="<td><input type=\"button\" value=\"Get Still\" onclick=\"if (myVar) clearInterval(myVar);execute(location.origin+\'?getstill\');\"></td>";
              Feedback+="<td><input type=\"button\" value=\"Get Still (Timer)\" onclick=\"if (myVar) clearInterval(myVar);myVar = setInterval(function(){execute(location.origin+\'?getstill\');}, Number(document.getElementById(\'interval\').value)*1000);\"></td>";              
              Feedback+="<td><input type=\"button\" value=\"Save Image\" onclick=\"if (myVar) clearInterval(myVar);execute(location.origin+\'?saveimage=\'+(new Date().getFullYear()*10000000000+(new Date().getMonth()+1)*100000000+new Date().getDay()*1000000+new Date().getHours()*10000+new Date().getMinutes()*100+new Date().getSeconds()).toString());\"></td>";  
              Feedback+="</tr><tr><td></td><td></td><td></td><td><input type=\"text\" id=\"interval\" value=\"1\" size=\"2\">(s)</td><td></td></tr></table>";  
              Feedback+="<br><div id=\"show\" ></div>";
              Feedback+="</body></html>"; 
              Feedback+="<script>";
              Feedback+="function execute(target) {";
              Feedback+="  var data = $.ajax({";
              Feedback+="  type: \"get\",";
              Feedback+="  dataType: \"text\",";
              Feedback+="  url: target,";
              Feedback+="  success: function(response)";
              Feedback+="    {";
              Feedback+="      document.getElementById(\'show\').innerHTML = response;";
              Feedback+="    },";
              Feedback+="    error: function(exception)";
              Feedback+="    {";
              Feedback+="      document.getElementById(\'show\').innerHTML = \'fail\';";
              Feedback+="    }";
              Feedback+="  });";
              Feedback+="}";
              Feedback+="</script>";
            }
          
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: text/html; charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            client.println("Connection: close");
            client.println();
            int Index;
            for (Index = 0; Index < Feedback.length(); Index = Index+1000) {
              client.print(Feedback.substring(Index, Index+1000));
            }
            client.println();
                        
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
          if (Command.indexOf("stop")!=-1) {
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
  
  return "<img src=\'"+imageFile+"\'>";
}

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
      filename.toLowerCase();
      if (filename.indexOf(".html")!=-1)
        list = "<tr><td><button onclick=\'execute(location.origin+\"?deleteimage="+String(file.name())+"\");\'>Delete</button></td><td></td><td><a onclick=\'execute(location.origin+\"?showimage="+String(file.name())+"\")\' style=\'color:blue;\'>"+String(file.name())+"</a></td><td align=\'right\'>"+String(file.size())+" B</td></tr>"+list;
      else
        list = "<tr><td><button onclick=\'execute(location.origin+\"?deleteimage="+String(file.name())+"\");\'>Delete</button></td><td>"+String(file.name())+"</td><td></td><td align=\'right\'>"+String(file.size())+" B</td></tr>"+list;  
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

String showimage(String filename) {
  Serial.printf("Reading file: %s\n", filename);

  //SD Card
  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return "Card Mount Failed";
  }  
  
  fs::FS &fs = SD_MMC;
  File file = fs.open(filename);
  if(!file){
    Serial.println("Failed to open file for reading");
    SD_MMC.end();    
    return "Failed to open file for reading";
  }
  Serial.print("Read from file: ");
  String imageFile="";
  while(file.available()){
    char c = file.read();
    imageFile+=String(c);
  }
  file.close();
  SD_MMC.end();

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
    
  return imageFile;
}

String deleteimage(String filename) {
  Serial.printf("Reading file: %s\n", filename);

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

String saveCapturedImage(String filename) {
  String response = ""; 
  String path_html = "/"+filename+".html";
  String path_jpg = "/"+filename+".jpg";

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
  imageFile = "<img src=\'"+imageFile+"\'>";
  
  //SD Card
  if(!SD_MMC.begin()){
    response = "Card Mount Failed";
    return "Card Mount Failed";
  }  
  
  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path_html.c_str());

  File file = fs.open(path_html.c_str(), FILE_WRITE);
  if(!file){
    esp_camera_fb_return(fb);
    SD_MMC.end();
    return "Failed to open file in writing mode";
  } 
  else {
    int Index;
    for (Index=0;Index<imageFile.length();Index=Index+1000) {
      file.print(imageFile.substring(Index, Index+1000));
    }    
    Serial.printf("Saved file to path: %s\n", path_html.c_str());
  }
  file.close();

  Serial.printf("Picture file name: %s\n", path_jpg.c_str());
  file = fs.open(path_jpg.c_str(), FILE_WRITE);
  if(!file){
    esp_camera_fb_return(fb);
    SD_MMC.end();    
    response = "Failed to open file in writing mode";
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
          
