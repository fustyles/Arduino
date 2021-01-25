/* 
NodeMCU (ESP32) MyFirmata
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-11-4 09:00
https://www.facebook.com/francefu

Command Format :  
http://APIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
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
http://192.168.4.1/?tcp=domain;port;request;wait
http://192.168.4.1/?ifttt=event;key;value1;value2;value3
http://192.168.4.1/?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8
http://192.168.4.1/?thingspeakread=request
http://192.168.4.1/?linenotify=token;request
--> request = message=xxxxx
--> request = message=xxxxx&stickerPackageId=xxxxx&stickerId=xxxxx
http://192.168.4.1/?car=pinL1;pinL2;pinR1;pinR2;L_speed;R_speed;Delay;state
http://192.168.4.1/?i2cLcd=address;gpioSDA;gpioSCL;text1;text2

STAIP：
Query：http://192.168.4.1/?ip
Link：http://192.168.4.1/?resetwifi=ssid;password

If you don't need to get response from ESP8266 and want to execute commands quickly, 
you can append a parameter value "stop" at the end of command.
For example:
http://192.168.4.1/?digitalwrite=gpio;value;stop
http://192.168.4.1/?restart=stop

Control Page (http)
Source
https://github.com/fustyles/webduino/blob/master/ESP8266_MyFirmata.html
Page
https://fustyles.github.io/webduino/ESP8266_MyFirmata.html

LCD Library
https://github.com/nhatuan84/esp32-lcd
*/

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>

// Enter your WiFi ssid and password
const char* ssid     = "xxxxx";   //your network SSID
const char* password = "xxxxx";   //your network password

const char* apssid = "MyFirmata ESP32";
const char* appassword = "12345678";         //AP password require at least 8 characters.

WiFiServer server(80);

String Feedback="", Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
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
    ESP.restart();
  }    
  else if (cmd=="resetwifi")
  {
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
    Feedback="STAIP: "+WiFi.localIP().toString();
    /*
    if (WiFi.localIP().toString()!="0.0.0.0") 
    {
      cmd="ifttt";
      P1="eventname";
      P2="key";
      P3=WiFi.localIP().toString();
      ExecuteCommand();
    }
    */
  }    
  else if (cmd=="inputpullup")
  {
    pinMode(P1.toInt(), INPUT_PULLUP);
  }  
  else if (cmd=="pinmode")
  {
    if (P2.toInt()==1)
      pinMode(P1.toInt(), OUTPUT);
    else
      pinMode(P1.toInt(), INPUT);
  }        
  else if (cmd=="digitalwrite")
  {
    ledcDetachPin(P1.toInt());
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(), P2.toInt());
  }   
  else if (cmd=="digitalread")
  {
    Feedback=String(digitalRead(P1.toInt()));
  }
  else if (cmd=="analogwrite")
  {
    ledcAttachPin(P1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,P2.toInt());
  }       
  else if (cmd=="analogread")
  {
    Feedback=String(analogRead(P1.toInt()));
  }
  else if (cmd=="touchread")
  {
    Feedback=String(touchRead(P1.toInt()));
  }   
  else if (cmd=="tcp") {
    String domain=P1;
    int port=P2.toInt();
    String request=P3;
    int wait=P4.toInt();      // wait = 0 or 1

    if ((port==443)||(domain.indexOf("https")==0)||(domain.indexOf("HTTPS")==0))
      Feedback=tcp_https(domain,request,port,wait);
    else
      Feedback=tcp_http(domain,request,port,wait);  
  }
  else if (cmd=="ifttt") {
    String domain="maker.ifttt.com";
    String request = "/trigger/" + P1 + "/with/key/" + P2;
    request += "?value1="+P3+"&value2="+P4+"&value3="+P5;
    Feedback=tcp_https(domain,request,443,0);
  }
  else if (cmd=="thingspeakupdate") {
    String domain="api.thingspeak.com";
    String request = "/update?api_key=" + P1;
    request += "&field1="+P2+"&field2="+P3+"&field3="+P4+"&field4="+P5+"&field5="+P6+"&field6="+P7+"&field7="+P8+"&field8="+P9;
    Feedback=tcp_https(domain,request,443,0);
  }    
  else if (cmd=="thingspeakread") {
    String domain="api.thingspeak.com";
    String request = P1;
    Feedback=tcp_https(domain,request,443,1);
    int s=Feedback.indexOf("feeds");
    Feedback=Feedback.substring(s+8);
    int e=Feedback.indexOf("]");
    Feedback=Feedback.substring(0,e);
    Feedback.replace("},{",";");
    Feedback.replace("\":\"",",");
    Feedback.replace("\":",",");
    Feedback.replace("\",\"",","); 
    Feedback.replace("\"","");
    Feedback.replace("{","");
    Feedback.replace("}","");
    Feedback.replace("[","");
    Feedback.replace("]","");
    Feedback.replace(",\"",",");
    Feedback.replace("\":",",");
  } 
  else if (cmd=="linenotify") {    //message=xxx&stickerPackageId=xxx&stickerId=xxx
    String token = P1;
    String request = P2;
    Feedback=LineNotify(token,request,1);
    if (Feedback.indexOf("status")!=-1) {
      int s=Feedback.indexOf("{");
      Feedback=Feedback.substring(s);
      int e=Feedback.indexOf("}");
      Feedback=Feedback.substring(0,e);
      Feedback.replace("\"","");
      Feedback.replace("{","");
      Feedback.replace("}","");
    }
  } 
  else if (cmd=="car") {
    ledcAttachPin(P1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,0);
    ledcAttachPin(P2.toInt(), 2);
    ledcSetup(2, 5000, 8);
    ledcWrite(2,0);  
    ledcAttachPin(P3.toInt(), 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,0); 
    ledcAttachPin(P4.toInt(), 4);
    ledcSetup(4, 5000, 8);
    ledcWrite(4,0);
    delay(10);
    
    if (P8=="S") {
      //
    }
    else if  (P8=="F") {
      ledcAttachPin(P1.toInt(), 1);
      ledcSetup(1, 5000, 8);
      ledcWrite(1,P5.toInt());
      ledcAttachPin(P4.toInt(), 4);
      ledcSetup(4, 5000, 8);
      ledcWrite(4,P6.toInt());
      if ((P7!="")&&(P7!="0")) {
        delay(P7.toInt());
        ledcAttachPin(P1.toInt(), 1);
        ledcSetup(1, 5000, 8);
        ledcWrite(1,0);
        ledcAttachPin(P4.toInt(), 4);
        ledcSetup(4, 5000, 8);
        ledcWrite(4,0);          
      }     
    }
    else if  (P8=="B") {
      ledcAttachPin(P2.toInt(), 2);
      ledcSetup(2, 5000, 8);
      ledcWrite(2,P5.toInt());  
      ledcAttachPin(P3.toInt(), 3);
      ledcSetup(3, 5000, 8);
      ledcWrite(3,P6.toInt());  
      if ((P7!="")&&(P7!="0")) {
        delay(P7.toInt());
        ledcAttachPin(P2.toInt(), 2);
        ledcSetup(2, 5000, 8);
        ledcWrite(2,0); 
        ledcAttachPin(P3.toInt(), 3);
        ledcSetup(3, 5000, 8);
        ledcWrite(3,0); 
      }     
    }
    else if  (P8=="L") {
      ledcAttachPin(P2.toInt(), 2);
      ledcSetup(2, 5000, 8);
      ledcWrite(2,P5.toInt());  
      ledcAttachPin(P4.toInt(), 4);
      ledcSetup(4, 5000, 8);
      ledcWrite(4,P6.toInt());   
      if ((P7!="")&&(P7!="0")) {
        delay(P7.toInt());
        ledcAttachPin(P2.toInt(), 2);
        ledcSetup(2, 5000, 8);
        ledcWrite(2,0); 
        ledcAttachPin(P4.toInt(), 4);
        ledcSetup(4, 5000, 8);
        ledcWrite(4,0);          
      }
    }
    else if  (P8=="R") {
      ledcAttachPin(P1.toInt(), 1);
      ledcSetup(1, 5000, 8);
      ledcWrite(1,P5.toInt());
      ledcAttachPin(P3.toInt(), 3);
      ledcSetup(3, 5000, 8);
      ledcWrite(3,P6.toInt());  
      if ((P7!="")&&(P7!="0")) {
        delay(P7.toInt());
        ledcAttachPin(P1.toInt(), 1);
        ledcSetup(1, 5000, 8);
        ledcWrite(1,0);
        ledcAttachPin(P3.toInt(), 3);
        ledcSetup(3, 5000, 8);
        ledcWrite(3,0); 
      }        
    }
  } 
  else if (cmd=="i2cLcd") {
    LiquidCrystal_I2C lcd(P1.toInt(),16,2);
    lcd.begin(P2.toInt(), P3.toInt());
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(P4);
    lcd.setCursor(0,1);
    lcd.print(P5);
  }
  else {
    Feedback="Command is not defined.";
  }
  if (Feedback=="") Feedback=Command;  
}

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
        if ((StartTime+10000) < millis()) break;
    } 
  
    if (WiFi.status() == WL_CONNECTED)
    {
      pinMode(2, OUTPUT);
      for (int i=0;i<5;i++)
      {
        digitalWrite(2,HIGH);
        delay(100);
        digitalWrite(2,LOW);
        delay(100);
      }
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
            /*
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: application/json;charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            //client.println("Connection: close");
            client.println();
            client.println("[{\"esp32\":\""+Feedback+"\"}]");
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
            client.println("<esp32><feedback>"+Feedback+"</feedback></esp32>");
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
            Feedback+="<option value=\"touchread\">touchRead</option>";
            Feedback+="<option value=\"tcp\">tcp</option>";
            Feedback+="<option value=\"ifttt\">ifttt</option>";
            Feedback+="<option value=\"thingspeakupdate\">thingspeakupdate</option>";
            Feedback+="<option value=\"thingspeakread\">thingspeakread</option>";
            Feedback+="<option value=\"linenotify\">linenotify</option>";
            Feedback+="<option value=\"car\">car(L1,L2,R1,R2,Lspeed,Rspeed,Delay,state)</option>";
            Feedback+="<option value=\"i2cLcd\">i2cLcd(address,gpioSDA,gpioSCL,text1,text2)</option>";
            Feedback+="</select>";
            Feedback+="<br><br>P1:"; 
            Feedback+="<input type=\"text\" name=\"P1\" id=\"P1\" size=\"20\">";      
            Feedback+="<br><br>P2:";
            Feedback+="<input type=\"text\" name=\"P2\" id=\"P2\" size=\"20\">";  
            Feedback+="<br><br>P3:";
            Feedback+="<input type=\"text\" name=\"P3\" id=\"P3\" size=\"20\">";  
            Feedback+="<br><br>P4:";
            Feedback+="<input type=\"text\" name=\"P4\" id=\"P4\" size=\"20\">"; 
            Feedback+="<br><br>P5:";
            Feedback+="<input type=\"text\" name=\"P5\" id=\"P5\" size=\"20\">"; 
            Feedback+="<br><br>P6:";
            Feedback+="<input type=\"text\" name=\"P6\" id=\"P6\" size=\"20\">"; 
            Feedback+="<br>(P6;P7;P8;P9)<br><br>";           
            Feedback+="<input type=\"button\" value=\"Send\" onclick=\"location.href='?'+cmd.value+'='+P1.value+';'+P2.value+';'+P3.value+';'+P4.value+';'+P5.value+';'+P6.value\">"; 
            Feedback+="</form>";

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
  }
  /*
  if (SensorValue>LimitValue)
  {
    cmd="yourcmd";
    P1="yourP1";
    P2="yourP2";
    P3="yourP3";
    ...
    P9="yourP9";
    ExecuteCommand();
    delay(10000);
  }
  */
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

String tcp_http(String domain,String request,int port,byte wait)
{
    WiFiClient client_tcp;

    if (client_tcp.connect(domain.c_str(), port)) 
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

String tcp_https(String domain,String request,int port,byte wait)
{
    WiFiClientSecure client_tcp;

    if (client_tcp.connect(domain.c_str(), port)) 
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

String LineNotify(String token, String request, byte wait)
{
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
  
  if (client_tcp.connect("notify-api.line.me", 443)) 
  {
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("User-Agent: ESP8266/1.0");
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
