/*
ESP32 PMS5003T
Author : ChungYi Fu (Kaohsiung, Taiwan)  2021-1-21 19:30
https://www.facebook.com/francefu

Get sensor values
http://192.168.4.1
http://STAIP

Set WIFI ssid and password
http://192.168.4.1?wifi
http://STAIP?wifi

Set ThingSpeak api_key
http://192.168.4.1?key
http://STAIP?key

Set Line Notify token
http://192.168.4.1?token
http://STAIP?token

Erase SPI Flash
http://192.168.4.1?eraseflash
http://STAIP?eraseflash
refer to
https://ruten-proteus.blogspot.com/2016/12/ESp8266ArduinoQA-02.html
http://wyj-learning.blogspot.com/2018/03/nodemcu-flash.html
  
ESP32 LCD Library
https://github.com/nhatuan84/esp32-lcd
16x2 LCD
5V, GND, SDA:gpio12 , SCL:gpio14 
[Web:Bit]
5V, GND, RX:P6 (gpio12), TX:P7 (gpio14)

PMS5003T
3V3, GND, RX:gpio16, TX:gpio17
[Web:Bit]
3V3, GND, RX:P8 (gpio16), TX:P9 (gpio17)

Command Format :  
http://APIP/?cmd=p1;p2;p3;p4;p5;p6;p7;p8;p9
http://STAIP/?cmd=p1;p2;p3;p4;p5;p6;p7;p8;p9

Default APIP： 192.168.4.1

http://192.168.4.1/?ip
http://192.168.4.1/?mac
http://192.168.4.1/?restart
http://192.168.4.1/?resetwifi=ssid;password
http://192.168.4.1/?inputpullup=pin
http://192.168.4.1/?pinmode=pin;value
http://192.168.4.1/?digitalwrite=pin;value
http://192.168.4.1/?analogwrite=pin;value
http://192.168.4.1/?digitalread=pin
http://192.168.4.1/?analogread=pin
http://192.168.4.1/?touchread=pin
http://192.168.4.1/?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8
http://192.168.4.1/?linenotify=token;request
--> request = message=xxxxx
--> request = message=xxxxx&stickerPackageId=xxxxx&stickerId=xxxxx
*/

const char* ssid     = "";  //WIFI ssid
const char* password = "";  //WIFI pwd

const char* apssid = "ESP32_PMS5003T";
const char* appassword = "12345678";

String thingspeak_api_key = "";   //ThingSpeak
int delaytime = 30;               //delay 30 seconds

String line_token = "";           //Line Notify

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
int lcdAddress = 39;    //0x27=39, 0x3F=63 
LiquidCrystal_I2C lcd(lcdAddress, 16, 2);
int lcdSDA = 12;
int lcdSCL = 14;

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <HardwareSerial.h>
HardwareSerial mySerial(1);  // RX:gpio16   TX:gpio17

#include <ESP.h>
const uint32_t addressStart = 0x3FA000; 
const int len = 64;    // flashWrite, flashRead -> i = 0 to 63

long pmat10 = 0;
long pmat25 = 0;
long pmat100 = 0;
long Temp = 0;
long Humid = 0;
char buf[50];
int delaycount = 0;

WiFiServer server(80);

String Feedback="", Command="",cmd="",p1="",p2="",p3="",p4="",p5="",p6="",p7="",p8="",p9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0,cmdstate=0;

void ExecuteCommand() {
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,p1= "+p1+" ,p2= "+p2+" ,p3= "+p3+" ,p4= "+p4+" ,p5= "+p5+" ,p6= "+p6+" ,p7= "+p7+" ,p8= "+p8+" ,p9= "+p9);
  Serial.println("");
  
  if (cmd=="") {
    Feedback = "PM2.5:    "+String(pmat25)+" ug/m3<br>PM100:    "+String(pmat100)+" ug/m3<br>Temperature:    "+String(Temp)+" °C<br>Humidity:    "+String(Humid)+" %RH";
  }
  else if (cmd=="wifi") { 
    Feedback="SSID: <input type=\"text\" id=\"ssid\"><br>PWD: <input type=\"text\" id=\"pwd\"><br><input type=\"button\" value=\"submit\" onclick=\"location.href='?resetwifi='+document.getElementById('ssid').value+';'+document.getElementById('pwd').value;\">";  
  }
  else if (cmd=="key") { 
    Feedback="ThingSpeak API_KEY: <input type=\"text\" id=\"key\"><input type=\"button\" value=\"submit\" onclick=\"location.href='?thingspeakapikey='+document.getElementById('key').value;\">";  
  }  
  else if (cmd=="token") { 
    Feedback="Line Notify Token: <input type=\"text\" id=\"token\"><input type=\"button\" value=\"submit\" onclick=\"location.href='?linetoken='+document.getElementById('token').value;\">";  
  }     
  else if (cmd=="thingspeakapikey") {
    char buff_key[len]; 
    strcpy(buff_key, p1.c_str());
    flashWrite(buff_key, 2); 

    thingspeak_api_key = p1;
    Feedback="Set ThingSpeak API_KEY = "+p1+" OK";
  }  
  else if (cmd=="linetoken") {
    char buff_token[len]; 
    strcpy(buff_token, p1.c_str());
    flashWrite(buff_token, 3); 
        
    line_token = p1;
    Feedback="Set Line Notify Token = "+p1+" OK";    
  } 
  else if (cmd=="eraseflash") {
    flashErase();
    Feedback="Erase SPI Flash OK. <a href=\"?restart\">Restart the board</a>";
  }   
  else if (cmd=="resetwifi") {
    char buff_ssid[len], buff_password[len]; 
    strcpy(buff_ssid, p1.c_str());
    strcpy(buff_password, p2.c_str());
    flashWrite(buff_ssid, 0);
    flashWrite(buff_password, 1);   

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
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(WiFi.localIP().toString());
    }
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
  else if (cmd=="inputpullup") {
    pinMode(p1.toInt(), INPUT_PULLUP);
  }  
  else if (cmd=="pinmode") {
    if (p2.toInt()==1)
      pinMode(p1.toInt(), OUTPUT);
    else
      pinMode(p1.toInt(), INPUT);
  }        
  else if (cmd=="digitalwrite") {
    ledcDetachPin(p1.toInt());
    pinMode(p1.toInt(), OUTPUT);
    digitalWrite(p1.toInt(), p2.toInt());
  }   
  else if (cmd=="digitalread") {
    Feedback=String(digitalRead(p1.toInt()));
  }
  else if (cmd=="analogwrite") {
    ledcAttachPin(p1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,p2.toInt());
  }       
  else if (cmd=="analogread") {
    Feedback=String(analogRead(p1.toInt()));
  }
  else if (cmd=="touchread") {
    Feedback=String(touchRead(p1.toInt()));
  }   
  else if (cmd=="thingspeakupdate") {
    String domain="api.thingspeak.com";
    String request = "/update?api_key=" + p1;
    request += "&field1="+p2+"&field2="+p3+"&field3="+p4+"&field4="+p5+"&field5="+p6+"&field6="+p7+"&field7="+p8+"&field8="+p9;
    Feedback=tcp_https(domain,request,443,0);
  }    
  else if (cmd=="linenotify") {    //message=xxx&stickerPackageId=xxx&stickerId=xxx
    String token = p1;
    String request = p2;
    Feedback=LineNotify(token,request,1);
  } 
  else {
    Feedback="Command is not defined.";
  }
  if (Feedback=="") Feedback=Command;  
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  
  lcd.begin(lcdSDA, lcdSCL);
  lcd.backlight();  
  lcd.clear();
    
  WiFi.mode(WIFI_AP_STA);

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  WiFi.begin(ssid, password);

  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");       
    if ((StartTime+10000) < millis()) break;
  } 

  if (WiFi.status() == WL_CONNECTED) {    
    pinMode(2, OUTPUT);
    for (int i=0;i<5;i++) {
      digitalWrite(2,HIGH);
      delay(100);
      digitalWrite(2,LOW);
      delay(100);
    }

    lcd.setCursor(0,0);
    lcd.print(WiFi.localIP().toString());
  }
  else {
    char buff_ssid[len], buff_password[len];
    strcpy(buff_ssid, flashRead(0));
    strcpy(buff_password, flashRead(1));
    if ((buff_ssid[0]>=32)&&(buff_ssid[0]<=126)) {
      Serial.println("");
      Serial.println("SPI Flash ssid = "+String(buff_ssid));
      Serial.println("SPI Flash password = "+String(buff_password));  
      
      WiFi.begin(buff_ssid, buff_password);
      Serial.print("\nConnecting to ");
      Serial.println(buff_ssid);
      
      long int StartTime=millis();
      while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");    
        if ((StartTime+5000) < millis()) break;
      }       
    }
    if (WiFi.status() == WL_CONNECTED) {  
      lcd.setCursor(0,0);
      lcd.print(WiFi.localIP().toString());
    }
  }

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());
  
  if (WiFi.status() == WL_CONNECTED)
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
  else
    WiFi.softAP(apssid, appassword);
    
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());   
   
  server.begin();  
  
  delay(3000); 
}

void loop() {

  getRequest();

  delay(1000);
  if (delaycount>0&&delaycount<delaytime) {
    delaycount++;
    return;
  }
  else
    delaycount=1;
  
  if (WiFi.status() == WL_CONNECTED) {
    pmat25 = 0;
    retrievepm25();
    if (pmat25 == 0) {
      delay(3000);
      retrievepm25();
    }
  
    Serial.println("");
    Serial.println("PM2.5 : " + String(pmat25) + " ug/m3");
    Serial.println("PM10 : " + String(pmat100) + " ug/m3");
    Serial.println("Temp : " + String(Temp) + " *C");
    Serial.println("Humid : " + String(Humid) + " %RH");
    Serial.println("");    
  
    String myPm25 = String(pmat25) ;
    if (myPm25.length()==1) myPm25 = " " + myPm25;
    String myPm100 = String(pmat100) ;
    if (myPm100.length()==1) myPm100 = " " + myPm100;
    String myTemp = String(Temp) ;
    if (myTemp.length()==1) myTemp = " " + myTemp;
    String myHumid = String(Humid) ;
    if (myHumid.length()==1) myHumid = " " + myHumid; 
         
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("PM2.5=" + myPm25 + "  " + myTemp + "*C");
    lcd.setCursor(0,1);
    lcd.print("PM10 =" + myPm100 + "  " + myHumid + "%RH");

    //Thingspeak
    String domain="api.thingspeak.com";
    if (thingspeak_api_key=="") {
      char buff_key[len];
      strcpy(buff_key, flashRead(2));
      if ((buff_key[0]>=32)&&(buff_key[0]<=126)) {
        thingspeak_api_key = String(buff_key);
        Serial.println("");
        Serial.println("SPI Flash api_key = "+thingspeak_api_key);
      }
    }
    if (thingspeak_api_key!="") {
      String request = "/update?api_key=" + thingspeak_api_key;
      request += "&field1="+String(pmat25)+"&field2="+String(pmat100)+"&field3="+String(Temp)+"&field4="+String(Humid);
      Serial.println(tcp_https(domain,request,443,0));
    }

    //Line Notify
    if (line_token=="") {
      char buff_token[len];
      strcpy(buff_token, flashRead(3));
      if ((buff_token[0]>=32)&&(buff_token[0]<=126)) {
        line_token = String(buff_token);
        Serial.println("");
        Serial.println("SPI Flash token = "+line_token);
      }
    }
    if (line_token!="") {
      String message = "\nPM2.5:    "+String(pmat25)+" ug/m3\nPM100:    "+String(pmat100)+" ug/m3\nTemperature:    "+String(Temp)+" °C\nHumidity:    "+String(Humid)+" %RH";
      Serial.println(LineNotify(line_token, "message="+message, 1));
    }
  }
}

void retrievepm25() {
  int count = 0;
  unsigned char c;
  unsigned char high;
  while (mySerial.available()) { 
    c = mySerial.read();
    if((count==0 && c!=0x42) || (count==1 && c!=0x4d)) {
      break;
    }
    if(count > 27) { 
      break;
    }
     else if(count == 10 || count == 12 || count == 14 || count == 24 || count == 26) {
      high = c;
    }
    else if(count == 11) {
      pmat10 = 256*high + c;
    }
    else if(count == 13) {
      pmat25 = 256*high + c;
    }
    else if(count == 15) {
      pmat100 = 256*high + c;
    }
     else if(count == 25) {        
      Temp = (256*high + c)/10;
    }
    else if(count == 27) {            
      Humid = (256*high + c)/10;
    }   
    count++;
  }
  while(mySerial.available()) 
    mySerial.read();
}

void getRequest() {
  Command="";cmd="";p1="";p2="";p3="";p4="";p5="";p6="";p7="";p8="";p9="";
  ReceiveState=0;cmdState=1;strState=1;questionstate=0;equalstate=0;semicolonstate=0;cmdstate=0;
  
  WiFiClient client = server.available();

  if (client) { 
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();             
        
        getCommand(c);
                
        if (c == '\n') {
          if (currentLine.length() == 0) {       
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
                        
            Feedback="";
            break;
          } else {
            currentLine = "";
          }
        } 
        else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.indexOf(" HTTP")!=-1&&currentLine.indexOf("favicon.ico")==-1) {
          Serial.println(currentLine);
          if (Command.indexOf("stop")!=-1) {
            client.println();
            client.println();
            client.stop();
          }
          currentLine="";
          Feedback="";
          if (cmdstate==0) {
            cmdstate=1;
            ExecuteCommand();
          }
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
  
  if (ReceiveState==1) {
    Command=Command+String(c);
    
    if (c=='=') cmdState=0;
    if (c==';') strState++;
  
    if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) p1=p1+String(c);
    if ((cmdState==0)&&(strState==2)&&(c!=';')) p2=p2+String(c);
    if ((cmdState==0)&&(strState==3)&&(c!=';')) p3=p3+String(c);
    if ((cmdState==0)&&(strState==4)&&(c!=';')) p4=p4+String(c);
    if ((cmdState==0)&&(strState==5)&&(c!=';')) p5=p5+String(c);
    if ((cmdState==0)&&(strState==6)&&(c!=';')) p6=p6+String(c);
    if ((cmdState==0)&&(strState==7)&&(c!=';')) p7=p7+String(c);
    if ((cmdState==0)&&(strState==8)&&(c!=';')) p8=p8+String(c);
    if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) p9=p9+String(c);
    
    if (c=='?') questionstate=1;
    if (c=='=') equalstate=1;
    if ((strState>=9)&&(c==';')) semicolonstate=1;
  }
}

String tcp_https(String domain,String request,int port,byte wait) {
    WiFiClientSecure client_tcp;
    client_tcp.setInsecure();   //run version 1.0.5 or above
  
    if (client_tcp.connect(domain.c_str(), port)) {
      Serial.println("GET " + request);
      client_tcp.println("GET " + request + " HTTP/1.1");
      client_tcp.println("Host: " + domain);
      client_tcp.println("Connection: close");
      client_tcp.println();

      String getResponse="",Feedback="";
      boolean state = false;
      int waitTime = 3000;   // timeout 3 seconds
      long startTime = millis();
      while ((startTime + waitTime) > millis()) {
        while (client_tcp.available()) {
            char c = client_tcp.read();
            if (state==true) Feedback += String(c);          
            if (c == '\n') {
              if (getResponse.length()==0) state=true; 
              getResponse = "";
            } 
            else if (c != '\r')
              getResponse += String(c);
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

String LineNotify(String token, String request, byte wait) {
  request.replace("%","%25");
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
  client_tcp.setInsecure();   //run version 1.0.5 or above
  
  if (client_tcp.connect("notify-api.line.me", 443)) {
    Serial.println(request);    
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("User-Agent: ESp8266/1.0");
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
    while ((startTime + waitTime) > millis()) {
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (state==true) Feedback += String(c);        
          if (c == '\n') {
            if (getResponse.length()==0) state=true; 
            getResponse = "";
          } 
          else if (c != '\r')
            getResponse += String(c);
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

void flashWrite(char data[len], int i) {      // len = 64 -> i = 0 to 63
  uint32_t flashAddress = addressStart + i*len;
  char buff_write[len];
  strcpy(buff_write, data);
  if (ESP.flashWrite(flashAddress,(uint32_t*)buff_write, sizeof(buff_write)-1))
    Serial.printf("address: %p write \"%s\" [ok]\n", flashAddress, buff_write);
  else 
    Serial.printf("address: %p write \"%s\" [error]\n", flashAddress, buff_write);
}

char* flashRead(int i) {      // len = 64 -> i = 0 to 63
  uint32_t flashAddress = addressStart + i*len;
  static char buff_read[len];
  if (ESP.flashRead(flashAddress,(uint32_t*)buff_read, sizeof(buff_read)-1)) {
    //Serial.printf("data: \"%s\"\n", buff_read);
    return buff_read;
  } else  
    return "";  
}

void flashErase() {
  if (ESP.flashEraseSector(addressStart / 4096))
    Serial.println("\nErase SPI Flash [ok]");
  else
    Serial.println("\nErase SPI Flash [error]");
}

/*
bool flashEraseSector(uint32_t sector);
bool flashWrite(uint32_t offset, uint32_t *data, size_t size);
bool flashRead(uint32_t offset, uint32_t *data, size_t size);
*/
