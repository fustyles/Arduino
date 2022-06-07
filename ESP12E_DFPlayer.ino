/* 
NodeMCU ESP12E + DFPlayer Mini MP3
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-6-7 20:00
https://www.facebook.com/francefu

DFPlayer RX -> ESP12E TX 
DFPlayer TX -> ESP12E RX

Sample Code
https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299

Command Format :  
http://APIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

Default APIP： 192.168.4.1

STAIP：
Query： http://192.168.4.1/?ip
Link：http://192.168.4.1/?resetwifi=ssid;password
If you don't need to get response from ESP8266 and want to execute commands quickly, 
you can append a parameter value "stop" at the end of command.

For example:
http://192.168.4.1/?digitalwrite=gpio;value;stop
http://192.168.4.1/?restart=stop
*/

#include <ESP8266WiFi.h>

// Enter your WiFi ssid and password
const char* ssid     = "teacher";   //your network SSID
const char* password = "87654321";   //your network password

const char* apssid = "ESP12E_DFPlayer";
const char* appassword = "12345678";         //AP password require at least 8 characters.

WiFiServer server(80);

#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

String Feedback="", Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void ExecuteCommand()
{
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
    setup();
  }    
  else if (cmd=="resetwifi") {
    WiFi.begin(P1.c_str(), P2.c_str());
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Feedback="STAIP: "+WiFi.localIP().toString();
  }   
  else if (cmd=="volume") {   // value = 0 to 30
    myDFPlayer.pause();
    delay(300);
    if (P1.toInt()>30)
      P1="30";
    else if (P1.toInt()<0)
      P1="0";
    myDFPlayer.volume(P1.toInt());
    delay(300);
    myDFPlayer.start();
  }     
  else if (cmd=="volumeUp") {
    myDFPlayer.pause();
    delay(300);
    myDFPlayer.volumeUp();
    delay(300);
    myDFPlayer.start();
  }   
  else if (cmd=="volumeDown") {
    myDFPlayer.pause();
    delay(300);
    myDFPlayer.volumeDown();
    delay(300);
    myDFPlayer.start();
  }    
  else if (cmd=="EQ") {  //P1 -> NORMAL|POP|ROCK|JAZZ|CLASSIC|BASS
    if (P1=="NORMAL")
      myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
    else if  (P1=="POP")
      myDFPlayer.EQ(DFPLAYER_EQ_POP);
    else if  (P1=="ROCK")
      myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
    else if  (P1=="JAZZ")
      myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
    else if  (P1=="CLASSIC")
      myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
    else if  (P1=="BASS")
      myDFPlayer.EQ(DFPLAYER_EQ_BASS);
  }      
  else if (cmd=="DEVICE") {  //P1 -> U_DISK|SD|AUX|SLEEP|FLASH
    if (P1=="U_DISK")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_U_DISK);
    else if  (P1=="SD")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    else if  (P1=="AUX")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_AUX);
    else if  (P1=="SLEEP")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_SLEEP);
    else if  (P1=="FLASH")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_FLASH);
  }   
  else if (cmd=="sleep") {
    myDFPlayer.sleep();
  }  
  else if (cmd=="reset") {
    myDFPlayer.reset();
  }
  else if (cmd=="enableDAC") {
    myDFPlayer.enableDAC();
  }  
  else if (cmd=="disableDAC") {
    myDFPlayer.disableDAC();
  }
  else if (cmd=="outputSetting") {  //myDFPlayer.outputSetting(true, 15); //enable the output and set the gain to 15
    myDFPlayer.outputSetting(P1.toInt(), P2.toInt());
  }
  else if (cmd=="next") {
    myDFPlayer.next();
  }
  else if (cmd=="previous") {
    myDFPlayer.previous();
  }
  else if (cmd=="play") {
    myDFPlayer.play(P1.toInt());
  }
  else if (cmd=="loop") {
    myDFPlayer.loop(P1.toInt());
  }
  else if (cmd=="pause") {
    myDFPlayer.pause();
  }  
  else if (cmd=="start") {
    myDFPlayer.start();
  }  
  else if (cmd=="playFolder") {  //myDFPlayer.playFolder(15, 4);  //play specific mp3 in SD:/15/004.mp3; Folder Name(1~99); File Name(1~255)
    myDFPlayer.playFolder(P1.toInt(), P2.toInt());
  }   
  else if (cmd=="enableLoopAll") {
    myDFPlayer.enableLoopAll();
  }    
  else if (cmd=="disableLoopAll") {
    myDFPlayer.disableLoopAll();
  }   
  else if (cmd=="playMp3Folder") {  //myDFPlayer.playMp3Folder(4); //play specific mp3 in SD:/MP3/0004.mp3; File Name(0~65535)
    myDFPlayer.playMp3Folder(P1.toInt());
  }     
  else if (cmd=="advertise") {  //myDFPlayer.advertise(3); //advertise specific mp3 in SD:/ADVERT/0003.mp3; File Name(0~65535)
    myDFPlayer.advertise(P1.toInt());
  }   
  else if (cmd=="stopAdvertise") {
    myDFPlayer.stopAdvertise();
  }  
  else if (cmd=="playLargeFolder") {  //myDFPlayer.playLargeFolder(2, 999); //play specific mp3 in SD:/02/004.mp3; Folder Name(1~10); File Name(1~1000)
    myDFPlayer.playLargeFolder(P1.toInt(), P2.toInt());
  }  
  else if (cmd=="loopFolder") {  //myDFPlayer.loopFolder(5); //loop all mp3 files in folder SD:/05.
    myDFPlayer.loopFolder(P1.toInt());
  }  
  else if (cmd=="randomAll") {
    myDFPlayer.randomAll();
  }  
  else if (cmd=="enableLoop") {
    myDFPlayer.enableLoop();
  }  
  else if (cmd=="disableLoop") {
    myDFPlayer.disableLoop();
  }   
  else {
    Feedback="Command is not defined";
  }
  
  if (Feedback=="") Feedback=Command;
}

void setup()
{
    Serial.begin(9600);
    delay(10);
    
    WiFi.mode(WIFI_AP_STA);
  
    //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

    WiFi.begin(ssid, password);

    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+10000) < millis()) break;
    } 

    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
    
    if (WiFi.status() == WL_CONNECTED)
    {
      WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
      /*
      cmd="ifttt";
      P1="eventname";
      P2="key";
      P3=WiFi.localIP().toString();
      ExecuteCommand();
      */
    }
    else
      WiFi.softAP(apssid, appassword);
      
    //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
    Serial.println("");
    Serial.println("APIP address: ");
    Serial.println(WiFi.softAPIP());    
    server.begin(); 
    
    Serial.println();
    Serial.println(F("DFRobotDemo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
    
    if (!myDFPlayer.begin(Serial)) {  //Use softwareSerial to communicate with mp3.
      Serial.println(F("Unable to begin:"));
      Serial.println(F("1.Please recheck the connection!"));
      Serial.println(F("2.Please insert the SD card!"));
    }
    else {
      Serial.println(F("DFPlayer Mini online."));
      myDFPlayer.volume(10);  //Set volume value. From 0 to 30
      
      pinMode(2, OUTPUT);
      for (int i=0;i<10;i++)
      {
        digitalWrite(2,LOW);
        delay(100);
        digitalWrite(2,HIGH);
        delay(100);
      }
    }
}

void loop()
{
  Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

  WiFiClient client = server.available();

  if (client) 
  { 
    String currentLine = "";

    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();             
        
        getCommand(c);
                
        if (c == '\n') 
        {
          if (currentLine.length() == 0) 
          { 
            Feedback+="<br><br>DFPlayer Mini MP3<br><br>";
            Feedback+="<form method=\"get\" action=\"\">";
            Feedback+="cmd:";
            Feedback+="<select name=\"cmd\" id=\"cmd\">";
            Feedback+="<option value=\"\"></option>";          
            Feedback+="<option value=\"play\">play(P1)</option>";
            Feedback+="<option value=\"next\">next</option>";
            Feedback+="<option value=\"previous\">previous</option>";  
            Feedback+="<option value=\"pause\">pause</option>";
            Feedback+="<option value=\"start\">start</option>";  
            Feedback+="<option value=\"loop\">loop(P1)</option>";  
            Feedback+="<option value=\"volume\">volume(P1)</option>";
            Feedback+="<option value=\"volumeUp\">volumeUp</option>";
            Feedback+="<option value=\"volumeDown\">volumeDown</option>";
            Feedback+="<option value=\"EQ\">EQ(P1=NORMAL,POP,ROCK,JAZZ,CLASSIC,BASS)</option>";
            Feedback+="<option value=\"DEVICE\">DEVICE(P1=U_DISK,SD,AUX,SLEEP,FLASH)</option>";
            Feedback+="<option value=\"sleep\">sleep</option>";
            Feedback+="<option value=\"reset\">reset</option>";
            Feedback+="<option value=\"enableDAC\">enableDAC</option>";
            Feedback+="<option value=\"disableDAC\">disableDAC</option>";
            Feedback+="<option value=\"outputSetting\">outputSetting(P1;P2)</option>";
            Feedback+="<option value=\"randomAll\">randomAll</option>";
            Feedback+="<option value=\"playFolder\">playFolder(P1;P2)</option>";
            Feedback+="<option value=\"enableLoopAll\">enableLoopAll</option>";
            Feedback+="<option value=\"disableLoopAll\">disableLoopAll</option>";
            Feedback+="<option value=\"playMp3Folder\">playMp3Folder(P1)</option>";
            Feedback+="<option value=\"advertise\">advertise(P1)</option>";
            Feedback+="<option value=\"stopAdvertise\">stopAdvertise</option>";
            Feedback+="<option value=\"playLargeFolder\">playLargeFolder(P1;P2)</option>";
            Feedback+="<option value=\"loopFolder\">loopFolder(P1)</option>";
            Feedback+="<option value=\"enableLoop\">enableLoop</option>";
            Feedback+="<option value=\"disableLoop\">disableLoop</option>";
            Feedback+="<option value=\"ip\">IP</option>";
            Feedback+="<option value=\"mac\">MAC</option>";
            Feedback+="<option value=\"restart\">Restart</option>";
            Feedback+="<option value=\"resetwifi\">ResetWifi</option>";              
            Feedback+="</select>";
            Feedback+="<br><br>P1:"; 
            Feedback+="<input type=\"text\" name=\"P1\" id=\"P1\" size=\"20\">";      
            Feedback+="<br><br>P2:";
            Feedback+="<input type=\"text\" name=\"P2\" id=\"P2\" size=\"20\">";   
            Feedback+="<br><br>";           
            Feedback+="<input type=\"button\" value=\"Send\" onclick=\"location.href='?'+cmd.value+'='+P1.value+';'+P2.value\">"; 
            Feedback+="</form>";

            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: text/html; charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            //client.println("Connection: close");
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

/*
if (myDFPlayer.available()) {
  printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
}
*/

void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
