/*
ESP32-CAM (Read the image file from SD card and upload it to Google Drive)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2021-6-9 21:30
https://www.facebook.com/francefu

如何新增Google Script
https://www.youtube.com/watch?v=f46VBqWwUuI

Google Script管理介面
https://script.google.com/home
https://script.google.com/home/executions

Google雲端硬碟
https://drive.google.com/drive/my-drive

Google Script程式碼
function doPost(e) {
  var myFoldername = e.parameter.myFoldername;  //取得資料夾名
  var myFile = e.parameter.myFile;  //取得影像
  var myFilename = Utilities.formatDate(new Date(), "GMT+8", "yyyyMMddHHmmss")+"_"+e.parameter.myFilename;  ////取得影像檔名
  
  var contentType = myFile.substring(myFile.indexOf(":")+1, myFile.indexOf(";"));
  var data = myFile.substring(myFile.indexOf(",")+1);
  data = Utilities.base64Decode(data);
  var blob = Utilities.newBlob(data, contentType, myFilename);
  
  var folder, folders = DriveApp.getFoldersByName(myFoldername);
  if (folders.hasNext()) {
    folder = folders.next();
  } else {
    folder = DriveApp.createFolder(myFoldername);
  }
  var file = folder.createFile(blob);    
  file.setDescription("Uploaded by " + myFilename);
  
  var imageID = file.getUrl().substring(file.getUrl().indexOf("/d/")+3,file.getUrl().indexOf("view")-1);
  var imageUrl = "https://drive.google.com/uc?authuser=0&id="+imageID; 
    
  return  ContentService.createTextOutput(myFoldername+"/"+myFilename+"\n"+imageUrl);
}
*/

//輸入Wi-Fi帳密
const char* ssid     = "*****";   //Wi-Fi帳號
const char* password = "*****";   //Wi-Fi密碼

const char* myDomain = "script.google.com";
String myScript = "/macros/s/**********************/exec";    //設定Google Script路徑
String myFoldername = "&myFoldername=ESP32-CAM";    //設定Google drive存放影像資料夾名
String myFilename = "&myFilename=ESP32-CAM.jpg";    //設定Google drive存放影像檔名
String myImage = "&myFile=";

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"            //用於電源不穩不重開機
#include "soc/rtc_cntl_reg.h"   //用於電源不穩不重開機
#include "FS.h"                 //file system wrapper
#include "SD_MMC.h"             //SD卡存取函式庫
#include "Base64.h"             //不可使用Arduino IDE內建的函式庫，請從github下載Base64.cpp, Base64.h置於同一資料夾

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //關閉電壓不穩時重啟電源設定
  
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);

  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    if ((StartTime+10000) < millis()) break;
  } 

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());
    
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reset");
    
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    delay(200);
    ledcWrite(3,0);
    delay(200);    
    ledcDetachPin(3);
        
    delay(1000);
    ESP.restart();   //若未連上Wi-Fi閃燈兩次後重啟
  }
  else {
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    for (int i=0;i<5;i++) {   //若連上Wi-Fi閃燈五次
      ledcWrite(3,10);
      delay(200);
      ledcWrite(3,0);
      delay(200);    
    }
    ledcDetachPin(3);      
  }

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

  //閃光燈
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8); 


  //read a image file from sd card and upload it to Google drive.
  sendSDImageToGoogleDrive("/test.jpg");
}

void loop() {
  delay(100);
}

String sendSDImageToGoogleDrive(String filepath) 
{
  if(!SD_MMC.begin()){
    Serial.println("Card Mount Failed");
    return "";
  }  
  
  fs::FS &fs = SD_MMC;
  File file = fs.open(filepath);
  if(!file){
    Serial.println("Failed to open file for reading");
    SD_MMC.end();  
    return "";  
  }

  Serial.println("Read from file: "+filepath);
  Serial.println("file size: "+String(file.size()));
  Serial.println("");

  uint8_t *fileinput;
  unsigned int fileSize = file.size();
  fileinput = (uint8_t*)malloc(fileSize + 1);
  file.read(fileinput, fileSize);
  fileinput[fileSize] = '\0';
  file.close();
  SD_MMC.end();
  
  char *input = (char *)fileinput;
  String imageFile = "data:image/jpeg;base64,";
  char output[base64_enc_len(3)];
  for (int i=0;i<fileSize;i++) {
    base64_encode(output, (input++), 3);
    if (i%3==0) imageFile += urlencode(String(output));
  }
  
  String Data = myFoldername+myFilename+myImage;  
  
  const char* myDomain = "script.google.com";
  String getAll="", getBody = "";

  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above  

  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");

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
          if (state==true) getBody += String(c);        
          if (c == '\n') 
          {
            if (getAll.length()==0) state=true; 
            getAll = "";
          } 
          else if (c != '\r')
            getAll += String(c);
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
