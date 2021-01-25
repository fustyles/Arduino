/* 
ESP-01 + Arduino Uno (without using AT Command)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-10-13 15:00
https://www.facebook.com/francefu

Uart Command Format : 
?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

?restart
?resetwifi=ssid;password
?tcp=domain;port;request;wait
?ifttt=event;key;value1;value2;value3
?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8
*/

#include <SoftwareSerial.h>
SoftwareSerial mySerial(0, 2); // ESP01 TX(gpio2)->D10, RX(gpio0)->D11

#include <ESP8266WiFi.h>

// Enter your WiFi ssid and password
const char* ssid     = "xxxxx";   //your network SSID
const char* password = "xxxxx";   //your network password

String Feedback="", Command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
boolean debug = false;

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  Serial.println("");
  
  if (cmd=="restart")
  {
    setup();
  }    
  else if (cmd=="resetwifi")
  {
    WiFi.begin(str1.c_str(), str2.c_str());
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Feedback="STAIP: "+WiFi.localIP().toString(); 
  }  
  else if (cmd=="tcp")
  {
    String domain=str1;
    int port=str2.toInt();
    String request=str3;
    int wait=str4.toInt();      // wait = 0 or 1
    Feedback=tcp(domain,request,port,wait);   
  }
  else if (cmd=="ifttt")
  {
    String domain="maker.ifttt.com";
    String request = "/trigger/" + str1 + "/with/key/" + str2;
    request += "?value1="+str3+"&value2="+str4+"&value3="+str5;
    Feedback=tcp(domain,request,80,0);
  }
  else if (cmd=="thingspeakupdate")
  {
    String domain="api.thingspeak.com";
    String request = "/update?api_key=" + str1;
    request += "&field1="+str2+"&field2="+str3+"&field3="+str4+"&field4="+str5+"&field5="+str6+"&field6="+str7+"&field7="+str8+"&field8="+str9;
    Feedback=tcp(domain,request,80,0);
  }    
  else if (cmd=="thingspeakread")
  {
    String domain="api.thingspeak.com";
    String request = str1;
    Feedback=tcp(domain,request,80,1);
  }     
  else 
  {
    Feedback="Command is not defined";
  }
  
  if (debug==true)
  {
    Serial.println(Feedback);
    mySerial.println(Feedback);  // Send Feedback to Arduino Uno
    mySerial.flush();
    delay(10); 
  }
}

void setup()
{
    mySerial.begin(9600);
    mySerial.setTimeout(10);
  
    Serial.begin(115200);
    delay(10);
    
    WiFi.mode(WIFI_STA);
    
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
  
    if (WiFi.status() == WL_CONNECTED)
    {
      pinMode(2, OUTPUT);
      for (int i=0;i<5;i++)
      {
        digitalWrite(2,LOW);
        delay(100);
        digitalWrite(2,HIGH);
        delay(100);
      }
    }  

    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
}

void loop()
{
  Command="";cmd="";str1="";str2="";str3="";str4="";str5="";str6="";str7="";str8="";str9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

  if (Serial.available())
  {
    while (Serial.available())
    {
      char c=Serial.read();
      Serial.print(String(c));
      getCommand(c);
      delay(1);
    }
    if (cmd!="") ExecuteCommand();
  }
  
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c=mySerial.read();
      getCommand(c);
      delay(1);
    }  
    if (cmd!="") ExecuteCommand();
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

String tcp(String domain,String request,int port,byte wait)
{
    WiFiClient client_tcp;
    
    if (client_tcp.connect(domain, port)) 
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
            if (state==true) Feedback += String(c);          
            if (c == '\n') 
            {
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

/*
Arduino Uno
Uart Command Format:
?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

?restart
?resetwifi=ssid;password
?tcp=domain;port;request;wait
?ifttt=event;key;value1;value2;value3
?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);  // ESP01 TX(gpio2)->D10, RX(gpio0)->D11

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);
}
void loop() 
{
  while (mySerial.available())
  {
    char c=mySerial.read();
    Serial.write(c);
  }
  while (Serial.available())
  {
    char c=Serial.read();
    mySerial.print(c);
  }
}
*/
