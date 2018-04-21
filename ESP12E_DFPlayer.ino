/* 
NodeMCU (ESP12E) + DFPlayer Mini MP3

Author : ChungYi Fu (Taiwan)  2018-4-19 20:00

Command Format :  
http://APIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9
http://STAIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

Default APIP： 192.168.4.1

STAIP：
Query： http://192.168.4.1/?ip
Link：http://192.168.4.1/?resetwifi=ssid;password
*/

#include <ESP8266WiFi.h>

const char* ssid     = "";   //your network SSID
const char* password = "";   //your network password

const char* apssid = "MyFirmata ESP12E";
const char* appassword = "12345678";         //AP password require at least 8 characters.

WiFiServer server(80);

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(13, 15); // RX(D7), TX(D8)
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

String Feedback="", Command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  Serial.println("");

  myDFPlayer.pause();
  delay(300);
  
  if (cmd=="your cmd")
  {
    // You can do anything
    // Feedback="<font color=\"red\">Hello World</font>";
  }
  else if (cmd=="ip")
  {
    Feedback="AP IP: "+WiFi.softAPIP().toString();    
    Feedback+=", ";
    Feedback+="STA IP: "+WiFi.localIP().toString();
  }  
  else if (cmd=="mac")
  {
    Feedback="STA MAC: "+WiFi.macAddress();
  }  
  else if (cmd=="restart")
  {
    setup();
    Feedback=Command;
  }    
  else if (cmd=="resetwifi")
  {
    WiFi.begin(str1.c_str(), str2.c_str());
    Serial.print("Connecting to ");
    Serial.println(str1);
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
  else if (cmd=="volume")
  {
    if (str1.toInt()>30)
      str1="30";
    else if (str1.toInt()<0)
      str1="0";
    myDFPlayer.volume(str1.toInt());
    
    Feedback=Command;
  }     
  else if (cmd=="volumeUp")
  {
    myDFPlayer.volumeUp();
    Feedback=Command;
  }   
  else if (cmd=="volumeDown")
  {
    myDFPlayer.volumeDown();
    Feedback=Command;
  }    
  else if (cmd=="EQ")
  {
    if (str1=="NORMAL")
      myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
    else if  (str1=="POP")
      myDFPlayer.EQ(DFPLAYER_EQ_POP);
    else if  (str1=="ROCK")
      myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
    else if  (str1=="JAZZ")
      myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
    else if  (str1=="CLASSIC")
      myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
    else if  (str1=="BASS")
      myDFPlayer.EQ(DFPLAYER_EQ_BASS);
    
    Feedback=Command;
  }      
  else if (cmd=="DEVICE")
  {
    if (str1=="U_DISK")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_U_DISK);
    else if  (str1=="SD")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    else if  (str1=="AUX")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_AUX);
    else if  (str1=="SLEEP")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_SLEEP);
    else if  (str1=="FLASH")
      myDFPlayer.outputDevice(DFPLAYER_DEVICE_FLASH);

    Feedback=Command;    
  }   
  else if (cmd=="sleep")
  {
    myDFPlayer.sleep();
    Feedback=Command;
  }  
  else if (cmd=="reset")
  {
    myDFPlayer.reset();
    Feedback=Command;
  }
    else if (cmd=="enableDAC")
  {
    myDFPlayer.enableDAC();
    Feedback=Command;
  }  
  else if (cmd=="disableDAC")
  {
    myDFPlayer.disableDAC();
    Feedback=Command;
  }
  else if (cmd=="outputSetting")
  {
    myDFPlayer.outputSetting(str1.toInt(), str2.toInt());
    Feedback=Command;
  }
  else if (cmd=="next")
  {
    myDFPlayer.next();
    Feedback=Command;
  }
  else if (cmd=="previous")
  {
    myDFPlayer.previous();
    Feedback=Command;
  }
  else if (cmd=="play")
  {
    myDFPlayer.play(str1.toInt());
    Feedback=Command;
  }
  else if (cmd=="loop")
  {
    myDFPlayer.loop(str1.toInt());
    Feedback=Command;
  }
  else if (cmd=="pause")
  {
    myDFPlayer.pause();
    Feedback=Command;
  }  
  else if (cmd=="start")
  {
    myDFPlayer.start();
    Feedback=Command;
  }  
  else if (cmd=="playFolder")
  {
    myDFPlayer.playFolder(str1.toInt(), str2.toInt());
    Feedback=Command;
  }   
  else if (cmd=="enableLoopAll")
  {
    myDFPlayer.enableLoopAll();
    Feedback=Command;
  }    
  else if (cmd=="disableLoopAll")
  {
    myDFPlayer.disableLoopAll();
    Feedback=Command;
  }   
  else if (cmd=="playMp3Folder")
  {
    myDFPlayer.playMp3Folder(str1.toInt());
    Feedback=Command;
  }     
  else if (cmd=="advertise")
  {
    myDFPlayer.advertise(str1.toInt());
    Feedback=Command;
  }   
  else if (cmd=="stopAdvertise")
  {
    myDFPlayer.stopAdvertise();
    Feedback=Command;
  }  
  else if (cmd=="playLargeFolder")
  {
    myDFPlayer.playLargeFolder(str1.toInt(), str2.toInt());
    Feedback=Command;
  }  
  else if (cmd=="loopFolder")
  {
    myDFPlayer.loopFolder(str1.toInt());
    Feedback=Command;
  }  
  else if (cmd=="randomAll")
  {
    myDFPlayer.randomAll();
    Feedback=Command;
  }  
  else if (cmd=="enableLoop")
  {
    myDFPlayer.enableLoop();
    Feedback=Command;
  }  
  else if (cmd=="disableLoop")
  {
    myDFPlayer.disableLoop();
    Feedback=Command;
  }   
  else 
  {
    Feedback="Command is not defined";
  }

  delay(300);
  myDFPlayer.start();
  delay(300);
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    
    WiFi.mode(WIFI_AP_STA);
    
    WiFi.softAP(apssid, appassword);
  
    //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  
    delay(1000);
    Serial.println("");
    Serial.println("APIP address: ");
    Serial.println(WiFi.softAPIP());  
  
    //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

    WiFi.begin(ssid, password);

    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 

    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();


    mySoftwareSerial.begin(9600);
    
    Serial.println();
    Serial.println(F("DFRobotDemo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
    
    if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
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
  Command="";cmd="";str1="";str2="";str3="";str4="";str5="";str6="";str7="";str8="";str9="";
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
            Feedback+="<br><br>";
            Feedback+="<form method=\"get\" action=\"\">";
            Feedback+="cmd:";
            Feedback+="<select name=\"cmd\" id=\"cmd\">";
            Feedback+="<option value=\"\"></option>";          
            Feedback+="<option value=\"play\">play(str1)</option>";
            Feedback+="<option value=\"next\">next</option>";
            Feedback+="<option value=\"previous\">previous</option>";  
            Feedback+="<option value=\"pause\">pause</option>";
            Feedback+="<option value=\"start\">start</option>";  
            Feedback+="<option value=\"loop\">loop(str1)</option>";  
            Feedback+="<option value=\"volume\">volume(str1)</option>";
            Feedback+="<option value=\"volumeUp\">volumeUp</option>";
            Feedback+="<option value=\"volumeDown\">volumeDown</option>";
            Feedback+="<option value=\"EQ\">EQ(str1=NORMAL,POP,ROCK,JAZZ,CLASSIC,BASS)</option>";
            Feedback+="<option value=\"DEVICE\">DEVICE(str1=U_DISK,SD,AUX,SLEEP,FLASH)</option>";
            Feedback+="<option value=\"sleep\">sleep</option>";
            Feedback+="<option value=\"reset\">reset</option>";
            Feedback+="<option value=\"enableDAC\">enableDAC</option>";
            Feedback+="<option value=\"disableDAC\">disableDAC</option>";
            Feedback+="<option value=\"outputSetting\">outputSetting(str1;str2)</option>";
            Feedback+="<option value=\"randomAll\">randomAll</option>";
            Feedback+="<option value=\"playFolder\">playFolder(str1;str2)</option>";
            Feedback+="<option value=\"enableLoopAll\">enableLoopAll</option>";
            Feedback+="<option value=\"disableLoopAll\">disableLoopAll</option>";
            Feedback+="<option value=\"playMp3Folder\">playMp3Folder(str1)</option>";
            Feedback+="<option value=\"advertise\">advertise(str1)</option>";
            Feedback+="<option value=\"stopAdvertise\">stopAdvertise</option>";
            Feedback+="<option value=\"playLargeFolder\">playLargeFolder(str1;str2)</option>";
            Feedback+="<option value=\"loopFolder\">loopFolder(str1)</option>";
            Feedback+="<option value=\"enableLoop\">enableLoop</option>";
            Feedback+="<option value=\"disableLoop\">disableLoop</option>";
            Feedback+="<option value=\"ip\">IP</option>";
            Feedback+="<option value=\"mac\">MAC</option>";
            Feedback+="<option value=\"restart\">Restart</option>";
            Feedback+="<option value=\"resetwifi\">ResetWifi</option>";              
            Feedback+="</select>";
            Feedback+="<br><br>str1:"; 
            Feedback+="<input type=\"text\" name=\"str1\" id=\"str1\" size=\"20\">";      
            Feedback+="<br><br>str2:";
            Feedback+="<input type=\"text\" name=\"str2\" id=\"str2\" size=\"20\">";   
            Feedback+="<br><br>";           
            Feedback+="<input type=\"button\" value=\"Send\" onclick=\"location.href='?'+cmd.value+'='+str1.value+';'+str2.value\">"; 
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
