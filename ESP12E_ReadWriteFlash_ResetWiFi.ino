/* 
NodeMCU (ESP12E)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-04-21 08:30
https://www.facebook.com/francefu

Command Format :  
http://IP/?scanwifi
http://IP/?resetwifi=ssid;password
http://IP/?erasewifi
*/

#include <ESP8266WiFi.h> 
#include <ESP.h>

const int len = 64;    // flashWrite, flashRead -> i = 0 to 63

char ssid[len]    = "";
char password[len]    = "";

const char* apssid = "MyFirmata ESP12E";
const char* appassword = "12345678";         //AP password require at least 8 characters.

const uint32_t addressStart = 0x3FA000; 
const uint32_t addressEnd   = 0x3FAFFF;

boolean restart = false;

WiFiServer server(80);

String Feedback="", Command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void ExecuteCommand()
{
  //Serial.println("Command: "+Command);
  Serial.println("\ncmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
 
  if (cmd=="your cmd") {
    // You can do anything
    // Feedback="<font color=\"red\">Hello World</font>";
  }
  else if (cmd=="scanwifi") {
    Feedback="<form id=\"resetwifi\" method=\"get\" action=\"\">";
    Feedback+="<table><tr><td>SSID</td><td><select id =\"ssid\">";
    byte numSsid = WiFi.scanNetworks();
    for (int thisNet = 0; thisNet<numSsid; thisNet++) {
      Feedback+="<option value=\""+WiFi.SSID(thisNet)+"\">"+WiFi.SSID(thisNet)+"</option>";
    }
    Feedback+="</select></td></tr><tr><td>PASSWORD</td><td><input type=\"text\" id=\"password\"></td></tr><tr><td colspan=\"2\"><input type=\"reset\" value=\"Reset\"><input type=\"button\" value=\"Send\" onclick=\"location.href=\'?resetwifi=\'+ssid.value+\';\'+password.value;\"></td></tr></table></form>";
  }
  else if (cmd=="resetwifi") {
    char buff_ssid[len], buff_password[len]; 
    strcpy(buff_ssid, str1.c_str());
    strcpy(buff_password, str2.c_str());
    flashErase();
    flashWrite(buff_ssid, 0);  
    flashWrite(buff_password, 1);   
     
    restart = true;     
    Feedback="The ESP12E will retart to connect to "+ str1;
  }  
  else if (cmd=="erasewifi") {
    flashErase();
    Feedback="Erase [ok]";
  }
  else
    Feedback="Command is not defined";
}

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_AP_STA);

  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("\nAPIP address: ");
  Serial.println(WiFi.softAPIP());    

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));
  
  if ((ssid[0]>=32)&&(ssid[0]<=126)) {
    WiFi.begin(ssid, password);
  
    delay(1000);
    Serial.print("\nConnecting to ");
    Serial.println(ssid);
    
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print("."); 
      if ((StartTime+5000) < millis()) break;
    } 
  }

  if (WiFi.status() != WL_CONNECTED) {
    // Read WIFI SSID and password from SPI FLASH.
    char buff_ssid[len], buff_password[len];
    strcpy(buff_ssid, flashRead(0));
    strcpy(buff_password, flashRead(1));
    Serial.printf("\nssid: \"%s\"\n", buff_ssid);
    Serial.printf("password: \"%s\"\n", buff_password);
    if ((buff_ssid[0]>=32)&&(buff_ssid[0]<=126)) {
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
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nSTAIP address: ");
    Serial.println(WiFi.localIP());      
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
    
    pinMode(2, OUTPUT);
    for (int i=0;i<5;i++) {
      digitalWrite(2,HIGH);
      delay(400);
      digitalWrite(2,LOW);
      delay(400);
    }
  }
  else {
    Serial.println("\nConnection failed.");
    WiFi.softAP(apssid, appassword);
  }
  
  server.begin(); 
}

void loop()
{
  Command="";cmd="";str1="";str2="";str3="";str4="";str5="";str6="";str7="";str8="";str9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

  WiFiClient client = server.available();

  if (client) 
  { 
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
        else if (c != '\r') 
        {
          currentLine += c;
        }

        if ((currentLine.indexOf("/?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1))
        {
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
    
    if (restart==true) {
      delay(2000);
      WiFi.disconnect();
      delay(1000);
      Serial.println("ESP32 restart");
      ESP.restart();      
    }   
  }
}

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
    if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) str1=str1+String(c);
    if ((cmdState==0)&&(strState==2)&&(c!=';')) str2=str2+String(c);
    if ((cmdState==0)&&(strState==3)&&(c!=';')) str3=str3+String(c);
    if ((cmdState==0)&&(strState==4)&&(c!=';')) str4=str4+String(c);
    if ((cmdState==0)&&(strState==5)&&(c!=';')) str5=str5+String(c);
    if ((cmdState==0)&&(strState==6)&&(c!=';')) str6=str6+String(c);
    if ((cmdState==0)&&(strState==7)&&(c!=';')) str7=str7+String(c);
    if ((cmdState==0)&&(strState==8)&&(c!=';')) str8=str8+String(c);
    if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) str9=str9+String(c);
    
    if (c=='?') questionstate=1;
    if (c=='=') equalstate=1;
    if ((strState>=9)&&(c==';')) semicolonstate=1;
  }
}

void flashWrite(char data[len], int i) {      // i = 0 to 63
  uint32_t flashAddress = addressStart + i*len;
  char buff_write[len];
  strcpy(buff_write, data);
  if (ESP.flashWrite(flashAddress,(uint32_t*)buff_write, sizeof(buff_write)-1))
    Serial.printf("address: %p write \"%s\" [ok]\n", flashAddress, buff_write);
  else 
    Serial.printf("address: %p write \"%s\" [error]\n", flashAddress, buff_write);
}

char* flashRead(int i) {      // i = 0 to 63
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
    Serial.println("\nErase [ok]");
  else
    Serial.println("\nErase [error]");
}

/*
bool flashEraseSector(uint32_t sector);
bool flashWrite(uint32_t offset, uint32_t *data, size_t size);
bool flashRead(uint32_t offset, uint32_t *data, size_t size);
*/
