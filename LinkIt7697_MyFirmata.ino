/* 
LinkIt7697 MyFirmata
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-03-14 18:00
https://www.facebook.com/francefu

Command Format :  
http://STAIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

http://STAIP/?ip
http://STAIP/?resetwifi=ssid;password
http://STAIP/?inputpullup=pin
http://STAIP/?pinmode=pin;value
http://STAIP/?digitalwrite=pin;value
http://STAIP/?analogwrite=pin;value
http://STAIP/?digitalread=pin
http://STAIP/?analogread=pin

If you don't need to get response from LinkIt7697 and want to execute commands quickly, 
you can append a parameter value "stop" at the end of command.
For example:
http://STAIP/?digitalwrite=gpio;value;stop

Control Page (http)
Source
https://github.com/fustyles/webduino/blob/master/ESP8266_MyFirmata.html
Page
https://fustyles.github.io/webduino/ESP8266_MyFirmata.html
*/

char _lwifi_ssid[] = "*****";   //your network SSID
char _lwifi_pass[] = "*****";   //your network password

#include <LWiFi.h>
WiFiServer server(80);

String Feedback="", Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
  Serial.println("");
  
  if (cmd=="ip") {
    Feedback=WiFi.localIP().toString();
  }    
  else if (cmd=="resetwifi") {
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
    Feedback=WiFi.localIP().toString();
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
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(), P2.toInt());
  }   
  else if (cmd=="digitalread") {
    Feedback=String(digitalRead(P1.toInt()));
  }
  else if (cmd=="analogwrite") {
    analogWrite(P1.toInt(), P2.toInt());
  }       
  else if (cmd=="analogread") {
    Feedback=String(analogRead(P1.toInt()));
  }
  else {
    Feedback="Command is not defined";
  }
  if (Feedback=="") Feedback=Command;  
}

void setup() {
    Serial.begin(9600);
    delay(10);
    //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

    WiFi.begin(_lwifi_ssid, _lwifi_pass);

    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(_lwifi_ssid);
    
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+10000) < millis()) break;
    } 
  
    if (WiFi.status() == WL_CONNECTED) {
      pinMode(LED_BUILTIN, OUTPUT);
      for (int i=0;i<5;i++) {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN,LOW);
        delay(100);
      }
    } 
    else {
      pinMode(LED_BUILTIN, OUTPUT);
      for (int i=0;i<3;i++) {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN,LOW);
        delay(500);
      }
    }

    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
  
    server.begin();
}

void loop() {
  getCommand();
}

void getCommand() {
Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

 WiFiClient client = server.available();

  if (client) { 
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read(); 
         
        if (c=='?') ReceiveState=1;
        if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
        if (ReceiveState==1) {
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
  
        if (c == '\n') {
          if (currentLine.length() == 0) {    
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: text/html; charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            client.println();
            if (Feedback=="")
              client.println("Hello World");
            else
              client.println(Feedback);
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
