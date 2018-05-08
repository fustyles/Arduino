/* 
Electric Fan (NodeMCU ESP12E) with 3V motor.

Author : ChungYi Fu (Taiwan)  2018-05-08 23:30

Command Format :  
http://APIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9
http://STAIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

STAIP：
http://192.168.4.1/?resetwifi=ssid;password
*/

#include <ESP8266WiFi.h>

const char* ssid     = "";   //your network SSID
const char* password = "";   //your network password

const char* apssid = "ESP12E ElectricFan";
const char* appassword = "12345678";         //AP password require at least 8 characters.

WiFiServer server(80);

#include <Servo.h>
Servo myservo;

int angle=90;            //風向初始角度
int degree=5;            //單位時間風向旋轉角度
int servoPin=12;         //伺服馬達腳位  D6
int motorPin1=13;        //馬達驅動IC腳位  D7
int motorPin2=15;        //馬達驅動IC腳位  D8
int rotateState=0;       //風向旋轉狀態 1=旋轉, 0=暫停
int rotateInterval=500;  //風向旋轉間隔時間 (ms)
int speedValue=0;        //初始風速

String Feedback="", Command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void setup()
{
  Serial.begin(115200);
  delay(10);
  
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
      if ((StartTime+5000) < millis()) break;
  } 
  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());
  
  if (WiFi.localIP().toString()!="0.0.0.0")
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
  else
    WiFi.softAP(apssid, appassword);
    
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());    
  server.begin(); 

  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);    
  
  myservo.attach(servoPin);
  myservo.write(angle);
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
            client.println("</head><body><form>");
            client.println(Feedback);
            client.println("<br/><br/>");
            client.println("Rotation: ");
            client.println("<input type=\"button\" onclick=\"location.href='?rotate=1'\" value=\"Start\">");
            client.println("<input type=\"button\" onclick=\"location.href='?rotate=0'\" value=\"Stop\">");
            client.println("<br/><br/>");
            client.println("Speed: ");
            client.println("<input type=\"range\" name=\"pwm\" min=\"0\" max=\"255\" step=\"5\" value=\""+String(speedValue)+"\" onchange=\"setspeed.value='Set '+pwm.value;\">");
            client.println("<input type=\"button\" onclick=\"location.href='?speed=0'\" value=\"Stop\">");
            client.println("<input type=\"button\" name=\"setspeed\" onclick=\"location.href='?speed='+pwm.value;\" value=\"Set "+String(speedValue)+"\">");
            client.println("<br/><br/>");
            client.println("Angle: ");
            client.println("<input type=\"range\" name=\"angle\" min=\"5\" max=\"175\" step=\"5\" value=\""+String(angle)+"\" onchange=\"setangle.value='Set '+angle.value;\">");
            client.println("<input type=\"button\" name=\"setangle\" onclick=\"location.href='?angle='+angle.value;\" value=\"Set "+String(angle)+"\">");
            client.println("</form></body></html>");
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

  if (rotateState==1)      //風向來回轉動
  {
    angle+=degree;
    if ((angle<5)||(angle>175))
    {
      degree*=(-1);
      angle+=degree*2;
    }
    myservo.write(angle);
    delay(rotateInterval);
  }  
}

void ExecuteCommand()
{
  Serial.println("");
  Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  Serial.println("");
  
  if (cmd=="rotate")
  {
    rotateState=str1.toInt();
  }  
  else if (cmd=="speed")
  {
    speedValue=str1.toInt();
    analogWrite(motorPin1,0);    
    analogWrite(motorPin2,str1.toInt());
  }  
  else if (cmd=="angle")
  {
    rotateState=0;
    angle=str1.toInt();
    myservo.write(angle);
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
        if ((StartTime+10000) < millis()) break;
    } 
    Serial.println("");
    Serial.println("STAIP: "+WiFi.localIP().toString());
    Feedback="STAIP: "+WiFi.localIP().toString();
  }    
  else 
  {
    Feedback="Command is not defined";
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
