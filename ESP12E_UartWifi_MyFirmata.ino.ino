/* 
Arduino Uno + NodeMCU (ESP12E) (without using AT Command)

Author : ChungYi Fu (Taiwan)  2018-04-26 18:30

Wifi Command Format :  
http://APIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9
http://STAIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

Uart Command Format : 
?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

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
http://192.168.4.1/?tcp=domain;port;request;waitstate
http://192.168.4.1/?ifttt=event;key;value1;value2;value3
http://192.168.4.1/?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8
STAIP：
Query： http://192.168.4.1/?ip
Link：http://192.168.4.1/?resetwifi=ssid;password
Control Page (http)
https://github.com/fustyles/webduino/blob/master/ESP8266_MyFirmata.html
*/

#include <SoftwareSerial.h>
SoftwareSerial mySerial(13, 15); // RX(D7), TX(D8)

#include <ESP8266WiFi.h>

const char* ssid     = "";   //your network SSID
const char* password = "";   //your network password

const char* apssid = "MyFirmata ESP12E";
const char* appassword = "12345678";         //AP password require at least 8 characters.

WiFiServer server(80);

String Feedback="", Command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  Serial.println("");
  
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
  else if (cmd=="inputpullup")
  {
    pinMode(str1.toInt(), INPUT_PULLUP);
    Feedback=Command;
  }  
  else if (cmd=="pinmode")
  {
    if (str2.toInt()==1)
      pinMode(str1.toInt(), OUTPUT);
    else
      pinMode(str1.toInt(), INPUT);
    Feedback=Command;
  }        
  else if (cmd=="digitalwrite")
  {
    pinMode(str1.toInt(), OUTPUT);
    digitalWrite(str1.toInt(), str2.toInt());
    Feedback=Command;
  }   
  else if (cmd=="digitalread")
  {
    Feedback=String(digitalRead(str1.toInt()));
  }
  else if (cmd=="analogwrite")
  {
    pinMode(str1.toInt(), OUTPUT);
    analogWrite(str1.toInt(), str2.toInt());
    Feedback=Command;
  }       
  else if (cmd=="analogread")
  {
    Feedback=String(analogRead(str1.toInt()));
  }
  else if (cmd=="tcp")  // If it can't get response, you can set waitstate to 1.
  {
    String domain=str1;
    String request ="/" + str3;
    int port=str2.toInt();
    int waitstate=str4.toInt();
    tcp(domain,request,port,waitstate);
  }
  else if (cmd=="ifttt")
  {
    String domain="maker.ifttt.com";
    String request = "/trigger/" + str1 + "/with/key/" + str2;
    request += "?value1="+str3+"&value2="+str4+"&value3="+str5;
    tcp(domain,request,80,1);   // If it can't get response, you can set waitstate to 1.
  }
  else if (cmd=="thingspeakupdate")
  {
    String domain="api.thingspeak.com";
    String request = "/update?api_key=" + str1;
    request += "&field1="+str2+"&field2="+str3+"&field3="+str4+"&field4="+str5+"&field5="+str6+"&field6="+str7+"&field7="+str8+"&field8="+str9;
    tcp(domain,request,80,1);   // If it can't get response, you can set waitstate to 1.
  }    
  else 
  {
    mySerial.println(Command);  // Send command to Arduino Uno
    mySerial.flush();
    delay(10); 
    Feedback="Command is not defined";
  }
}

void setup()
{
    mySerial.begin(9600);
    mySerial.setTimeout(10);
      
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
  
    if (WiFi.localIP().toString()!="0.0.0.0")
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
    
    server.begin();
}

void loop()
{
  Command="";cmd="";str1="";str2="";str3="";str4="";str5="";str6="";str7="";str8="";str9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

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
            /*
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: application/json;charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            //client.println("Connection: close");
            client.println();
            client.println("[{\"esp8266\":\""+Feedback+"\"}]");
            client.println();
            */
            
            /*
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: text/xml; charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            //client.println("Connection: close");
            client.println();
            client.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
            client.println("<esp8266><feedback>"+Feedback+"</feedback></esp8266>");
            client.println();
           */
            
            Feedback+="<br><br>";
            Feedback+="<form method=\"get\" action=\"\">";
            Feedback+="cmd:";
            Feedback+="<select name=\"cmd\" id=\"cmd\">";
            Feedback+="<option value=\"ip\">IP</option>";
            Feedback+="<option value=\"mac\">MAC</option>";
            Feedback+="<option value=\"restart\">Restart</option>";
            Feedback+="<option value=\"resetwifi\">ResetWifi</option>";
            Feedback+="<option value=\"inputpullup\">InputPullUp</option>";
            Feedback+="<option value=\"pinmode\">pinMode</option>";
            Feedback+="<option value=\"digitalwrite\">digitalWrite</option>";
            Feedback+="<option value=\"analogwrite\">analogWrite</option>";
            Feedback+="<option value=\"digitalread\">digitalRead</option>";
            Feedback+="<option value=\"analogread\">analogRead</option>";  
            Feedback+="<option value=\"tcp\">tcp</option>";
            Feedback+="<option value=\"ifttt\">ifttt</option>";
            Feedback+="<option value=\"thingspeakupdate\">thingspeakupdate</option>";
            Feedback+="</select>";
            Feedback+="<br><br>str1:"; 
            Feedback+="<input type=\"text\" name=\"str1\" id=\"str1\" size=\"20\">";      
            Feedback+="<br><br>str2:";
            Feedback+="<input type=\"text\" name=\"str2\" id=\"str2\" size=\"20\">";  
            Feedback+="<br><br>str3:";
            Feedback+="<input type=\"text\" name=\"str3\" id=\"str3\" size=\"20\">";  
            Feedback+="<br>(str3;str4;str5;str6;str7;str8;str9)<br><br>";           
            Feedback+="<input type=\"button\" value=\"Send\" onclick=\"location.href='?'+cmd.value+'='+str1.value+';'+str2.value+';'+str3.value\">"; 
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
  
  //if (SensorValue>LimitValue)
  //{
  //  cmd="yourcmd";
  //  str1="yourstr1";
  //  str2="yourstr2";
  //  str3="yourstr3";
  //  ...
  //  str9="yourstr9";
  //  ExecuteCommand();
  //  delay(10000);
  //}
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

void tcp(String domain,String request,int port,int waitstate)   // If it can't get response, you can set waitstate to 1.
{
    WiFiClient client_tcp;
    
    if (client_tcp.connect(domain, port)) 
    {
      Serial.println("GET " + request);
      client_tcp.println("GET " + request + " HTTP/1.1");
      client_tcp.println("Host: " + domain);
      client_tcp.println("Connection: close");
      client_tcp.println();

      String getResponse="";
      boolean state = false;
      long StartTime = millis();
      while ((StartTime+3000) > millis())
      {
        while (client_tcp.available()) 
        {
            char c = client_tcp.read();
            if (c == '\n') 
            {
              if (getResponse.length()==0) state=true; 
              getResponse = "";
            } 
            else if (c != '\r')
              getResponse += String(c);
            if (state==true) Feedback += String(c);
         }
         if ((waitstate==0)&&(Feedback.length()!= 0)) break;
      }
      Serial.println(Feedback);
      client_tcp.stop();
    }
    else
      Feedback="Connection failed";  
}

/*
Arduino Uno

Uart Command Format:
?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

?tcp=domain;port;request;waitstate
?ifttt=event;key;value1;value2;value3
?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // connect to NodeMCU (ESP12E)

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
