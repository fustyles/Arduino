/* 
Electric Fan (NodeMCU ESP12E with 3V DC Motor)
Author : ChungYi Fu (Taiwan)  2018-05-09 17:30
Command Format :  
http://APIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9
http://STAIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9
Default APIP: 
192.168.4.1
STAIP：
http://192.168.4.1/?resetwifi=ssid;password
*/

#include <ESP8266WiFi.h>

const char* ssid     = "";   //your network SSID
const char* password = "";   //your network password

const char* apssid = "ElectricFan";
const char* appassword = "12345678";         //AP password requires at least 8 characters.

WiFiServer server(80);

#include <Servo.h>
Servo myservo;

int servoPin=12;           //Servo PIN -> D6 (GPIO12)
int motorPin1=13;          //Motor Driver IC PIN1 -> D7 (GPIO13)
int motorPin2=15;          //Motor Driver IC PIN2 -> D8 (GPIO15)

int angle=90;              //Angle of Servo position (0~180)
int degree=5;              //Degrees in angle of rotation
int rotateState=0;         //Rotation 1=Start, 0=Stop
int rotationInterval=500;  //Rotation interval (ms)
int speed=0;          //Fan speeds (0~1023)

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
            client.println("Fan Speeds <br/>");
            client.println("<input type=\"range\" name=\"speed\" min=\"0\" max=\"1023\" step=\"100\" value=\""+String(speed)+"\" onchange=\"setspeed.value='Set '+speed.value;\">");
            client.println("<input type=\"button\" onclick=\"location.href='?speed=0'\" value=\"Stop\">");
            client.println("<input type=\"button\" name=\"setspeed\" onclick=\"location.href='?speed='+speed.value;\" value=\"Set "+String(speed)+"\">");
            client.println("<br/><br/>");            
            client.println("Servo Rotation <br/>");
            client.println("<input type=\"button\" onclick=\"location.href='?rotateState=1'\" value=\"Start\">");
            client.println("<input type=\"button\" onclick=\"location.href='?rotateState=0'\" value=\"Stop\">");
            client.println("<br/><br/>");
            client.println("Angle Of Servo Position <br/>");
            client.println("<input type=\"range\" name=\"angle\" min=\"0\" max=\"180\" step=\"10\" value=\""+String(angle)+"\" onchange=\"setAngle.value='Set '+angle.value;\">");
            client.println("<input type=\"button\" name=\"setAngle\" onclick=\"location.href='?angle='+angle.value;\" value=\"Set "+String(angle)+"\">");
            client.println("<br/><br/>");
            client.println("Servo Angle Degrees <br/>");
            client.println("<input type=\"range\" name=\"degree\" min=\"-30\" max=\"30\" step=\"5\" value=\""+String(degree)+"\" onchange=\"setDegree.value='Set '+degree.value;\">");
            client.println("<input type=\"button\" name=\"setDegree\" onclick=\"location.href='?degree='+degree.value;\" value=\"Set "+String(degree)+"\">");
            client.println("<br/><br/>");
            client.println("Servo Rotation Interval <br/>");
            client.println("<input type=\"range\" name=\"rotationInterval\" min=\"100\" max=\"2000\" step=\"100\" value=\""+String(rotationInterval)+"\" onchange=\"setRotationInterval.value='Set '+rotationInterval.value;\">");
            client.println("<input type=\"button\" name=\"setRotationInterval\" onclick=\"location.href='?rotationInterval='+rotationInterval.value;\" value=\"Set "+String(rotationInterval)+"\">");
            client.println("<br/><br/>");
            client.println(Feedback);
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

  if (rotateState==1)
  {
    angle+=degree;
    if ((angle<0)||(angle>180))
    {
      degree*=(-1);
      angle+=degree*2;
    }
    myservo.write(angle);
    delay(rotationInterval);
  }  
}

void ExecuteCommand()
{
  Serial.println("");
  Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  Serial.println("");
  
  if (cmd=="rotateState")
  {
    rotateState=str1.toInt();
    Feedback="rotateState is changed to "+str1;
  }  
  else if (cmd=="speed")
  {
    speed=str1.toInt();
    analogWrite(motorPin1,0);    
    analogWrite(motorPin2,speed);
    Feedback="speed is changed to "+str1;
  }  
  else if (cmd=="angle")
  {
    rotateState=0;
    angle=str1.toInt();
    myservo.write(angle);
    Feedback="angle is changed to "+str1;
  }   
  else if (cmd=="degree")
  {
    degree=str1.toInt();
    Feedback="degree is changed to "+str1;
  }    
  else if (cmd=="rotationInterval")
  {
    rotationInterval=str1.toInt();
    Feedback="rotationInterval is changed to "+str1;
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
