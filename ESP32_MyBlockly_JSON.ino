/* 
NodeMCU (ESP32)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-1-26 10:00
Command Format :  
http://APIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9
http://STAIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9
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
http://192.168.4.1/?car=pinL1;pinL2;pinR1;pinR2;L_speed;R_speed;Delay;state
STAIP：
Query：http://192.168.4.1/?ip
Link：http://192.168.4.1/?resetwifi=ssid;password
Control Page (http)
https://github.com/fustyles/Arduino/blob/master/ESP8266_MyFirmata.html
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>

// Enter your WiFi ssid and password
const char* ssid     = "";   //your network SSID
const char* password = "";   //your network password

const char* apssid = "MyFirmata ESP32";
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
  
  if (cmd=="your cmd") {
    // You can do anything
    // Feedback="{\"data\":\"Sensor data"\"}";
  }
  else if (cmd=="ip") {
    Feedback="{\"data\":\""+WiFi.softAPIP().toString()+"\"},{\"data\":\""+WiFi.localIP().toString()+"\"}";
  }  
  else if (cmd=="mac") {
    Feedback="{\"data\":\""+WiFi.macAddress()+"\"}";
  }  
  else if (cmd=="restart") {
    setup();
    Feedback="{\"data\":\""+Command+"\"}";
  }    
  else if (cmd=="resetwifi") {
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
    Feedback="{\"data\":\""+WiFi.softAPIP().toString()+"\"},{\"data\":\""+WiFi.localIP().toString()+"\"}";
    /*
    if (WiFi.localIP().toString()!="0.0.0.0") 
    {
      cmd="ifttt";
      str1="eventname";
      str2="key";
      str3=WiFi.localIP().toString();
      ExecuteCommand();
    }
    */
  }    
  else if (cmd=="inputpullup") {
    pinMode(str1.toInt(), INPUT_PULLUP);
    Feedback="{\"data\":\""+Command+"\"}";
  }  
  else if (cmd=="pinmode") {
    if (str2.toInt()==1)
      pinMode(str1.toInt(), OUTPUT);
    else
      pinMode(str1.toInt(), INPUT);
    Feedback="{\"data\":\""+Command+"\"}";
  }        
  else if (cmd=="digitalwrite") {
    ledcDetachPin(str1.toInt());
    pinMode(str1.toInt(), OUTPUT);
    digitalWrite(str1.toInt(), str2.toInt());
    Feedback="{\"data\":\""+Command+"\"}";
  }   
  else if (cmd=="digitalread") {
    Feedback="{\"data\":\""+String(digitalRead(str1.toInt()))+"\"}";
  }
  else if (cmd=="analogwrite") {
    ledcAttachPin(str1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,str2.toInt());
    Feedback="{\"data\":\""+Command+"\"}";
  }       
  else if (cmd=="analogread") {
    Feedback="{\"data\":\""+String(analogRead(str1.toInt()))+"\"}";
  }
  else if (cmd=="touchread") {
    Feedback="{\"data\":\""+String(touchRead(str1.toInt()))+"\"}";
  }  
  else if (cmd=="tcp") {
    String domain=str1;
    int port=str2.toInt();
    String request=str3;
    int wait=str4.toInt();      // wait = 0 or 1

    if ((port==443)||(domain.indexOf("https")==0)||(domain.indexOf("HTTPS")==0))
      Feedback="{\"data\":\""+tcp_https(domain,request,port,wait)+"\"}";
    else
      Feedback="{\"data\":\""+tcp_http(domain,request,port,wait)+"\"}";  
  }
  else if (cmd=="ifttt") {
    String domain="maker.ifttt.com";
    String request = "/trigger/" + str1 + "/with/key/" + str2;
    request += "?value1="+str3+"&value2="+str4+"&value3="+str5;
    Feedback="{\"data\":\""+tcp_https(domain,request,443,0)+"\"}";
  }
  else if (cmd=="thingspeakupdate") {
    String domain="api.thingspeak.com";
    String request = "/update?api_key=" + str1;
    request += "&field1="+str2+"&field2="+str3+"&field3="+str4+"&field4="+str5+"&field5="+str6+"&field6="+str7+"&field7="+str8+"&field8="+str9;
    Feedback="{\"data\":\""+tcp_https(domain,request,443,0)+"\"}";
  }    
  else if (cmd=="thingspeakread") {
    String domain="api.thingspeak.com";
    String request = str1;
    Feedback="{\"data\":\""+tcp_https(domain,request,443,1)+"\"}";
  } 
  else if (cmd=="linenotify") {
    String token = str1;
    String request = str2;
    Feedback="{\"data\":\""+LineNotify(token,request,1)+"\"}";
  } 
  else if (cmd=="car") {
    ledcAttachPin(str1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,0);
    ledcAttachPin(str2.toInt(), 2);
    ledcSetup(2, 5000, 8);
    ledcWrite(2,0);  
    ledcAttachPin(str3.toInt(), 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,0); 
    ledcAttachPin(str4.toInt(), 4);
    ledcSetup(4, 5000, 8);
    ledcWrite(4,0);
    delay(10);
    
    if (str8=="S") {
      //
    }
    else if  (str8=="F") {
      ledcAttachPin(str1.toInt(), 1);
      ledcSetup(1, 5000, 8);
      ledcWrite(1,str5.toInt());
      ledcAttachPin(str4.toInt(), 4);
      ledcSetup(4, 5000, 8);
      ledcWrite(4,str6.toInt());
      if ((str7!="")&&(str7!="0")) {
        delay(str7.toInt());
        ledcAttachPin(str1.toInt(), 1);
        ledcSetup(1, 5000, 8);
        ledcWrite(1,0);
        ledcAttachPin(str4.toInt(), 4);
        ledcSetup(4, 5000, 8);
        ledcWrite(4,0);          
      }     
    }
    else if  (str8=="B") {
      ledcAttachPin(str2.toInt(), 2);
      ledcSetup(2, 5000, 8);
      ledcWrite(2,str5.toInt());  
      ledcAttachPin(str3.toInt(), 3);
      ledcSetup(3, 5000, 8);
      ledcWrite(3,str6.toInt());  
      if ((str7!="")&&(str7!="0")) {
        delay(str7.toInt());
        ledcAttachPin(str2.toInt(), 2);
        ledcSetup(2, 5000, 8);
        ledcWrite(2,0); 
        ledcAttachPin(str3.toInt(), 3);
        ledcSetup(3, 5000, 8);
        ledcWrite(3,0); 
      }     
    }
    else if  (str8=="L") {
      ledcAttachPin(str2.toInt(), 2);
      ledcSetup(2, 5000, 8);
      ledcWrite(2,str5.toInt());  
      ledcAttachPin(str4.toInt(), 4);
      ledcSetup(4, 5000, 8);
      ledcWrite(4,str6.toInt());   
      if ((str7!="")&&(str7!="0")) {
        delay(str7.toInt());
        ledcAttachPin(str2.toInt(), 2);
        ledcSetup(2, 5000, 8);
        ledcWrite(2,0); 
        ledcAttachPin(str4.toInt(), 4);
        ledcSetup(4, 5000, 8);
        ledcWrite(4,0);          
      }
    }
    else if  (str8=="R") {
      ledcAttachPin(str1.toInt(), 1);
      ledcSetup(1, 5000, 8);
      ledcWrite(1,str5.toInt());
      ledcAttachPin(str3.toInt(), 3);
      ledcSetup(3, 5000, 8);
      ledcWrite(3,str6.toInt());  
      if ((str7!="")&&(str7!="0")) {
        delay(str7.toInt());
        ledcAttachPin(str1.toInt(), 1);
        ledcSetup(1, 5000, 8);
        ledcWrite(1,0);
        ledcAttachPin(str3.toInt(), 3);
        ledcSetup(3, 5000, 8);
        ledcWrite(3,0); 
      }        
    }
  }    
  else {
    Feedback="{\"data\":\"Command is not defined\"}";
  }
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
  
    if (WiFi.localIP().toString()!="0.0.0.0")
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
  
    if (WiFi.localIP().toString()!="0.0.0.0")
    {
      WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
      /*
      cmd="ifttt";
      str1="eventname";
      str2="key";
      str3=WiFi.localIP().toString();
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
            client.println("Content-Type: application/json;charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            //client.println("Connection: close");
            client.println();
            client.println("["+Feedback+"]");
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
  /*
  if (SensorValue>LimitValue)
  {
    cmd="yourcmd";
    str1="yourstr1";
    str2="yourstr2";
    str3="yourstr3";
    ...
    str9="yourstr9";
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
            if (c == '\n') 
            {
              if (getResponse.length()==0) state=true; 
              getResponse = "";
            } 
            else if (c != '\r')
              getResponse += String(c);
            if (state==true) Feedback += String(c);
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
            if (c == '\n') 
            {
              if (getResponse.length()==0) state=true; 
              getResponse = "";
            } 
            else if (c != '\r')
              getResponse += String(c);
            if (state==true) Feedback += String(c);
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
  request.replace(" ","%20");
  request.replace("&","%20");
  request.replace("#","%20");
  //request.replace("\'","%27");
  request.replace("\"","%22");
  request.replace("\n","%0D%0A");
  
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
          if (c == '\n') 
          {
            if (getResponse.length()==0) state=true; 
            getResponse = "";
          } 
          else if (c != '\r')
            getResponse += String(c);
          if (state==true) Feedback += String(c);
          if (wait==1)
            startTime = millis();
       }
       if (wait==0)
        if ((state==true)&&(Feedback.length()!= 0)) break;
    }
    client_tcp.stop();
    Serial.println(Feedback);
    if (Feedback.indexOf("ok")!=-1)
      return "LineNotify success.";
    else
      return "LineNotify error.";
  }
  else
    return "Connection failed";  
}
