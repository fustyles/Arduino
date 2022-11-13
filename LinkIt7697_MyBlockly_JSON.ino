/* 
LinkIt7697 (MyBlockly JSON)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-10-29 00:00
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
http://STAIP/?tcp=domain;port;request;wait  
--> wait = 0 or 1  (waiting for response)
--> request = /xxxx/xxxx
http://STAIP/?ifttt=event;key;value1;value2;value3
http://STAIP/?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8
http://STAIP/?thingspeakread=request   
--> request = /channels/xxxxx/fields/1.json?results=1
http://STAIP/?linenotify=token;request
--> request = message=xxxxx
--> request = message=xxxxx&stickerPackageId=xxxxx&stickerId=xxxxx
http://STAIP/?car=pinL1;pinL2;pinR1;pinR2;L_speed;R_speed;Delay;state
http://STAIP/?i2cLcd=address;text1;text2   
--> address(Decimal) : 0x27=39, 0x3F=63   
--> SDA->gpio9, SCL->gpio8

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

const char* ssid     = "xxxxx";   //your network SSID
const char* password = "xxxxx";   //your network password
String LineNotifyToken = "";  //Send STAIP address message to LineNotify.

//I2C LCD 16x2 Library: https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/
#include <LiquidCrystal_I2C.h>

#include <LWiFi.h>
WiFiServer server(80);

// This is the root certificate for our host.
// Different host server may have different root CA.
static const char rootCA[] = "-----BEGIN CERTIFICATE-----\r\n"
"MIIDBjCCAe6gAwIBAgIBATANBgkqhkiG9w0BAQsFADA8MRYwFAYDVQQDDA1teWVz\r\n"
"cDMyLmxvY2FsMRUwEwYDVQQKDAxGYW5jeUNvbXBhbnkxCzAJBgNVBAYTAkRFMB4X\r\n"
"DTE5MDEwMTAwMDAwMFoXDTMwMDEwMTAwMDAwMFowPDEWMBQGA1UEAwwNbXllc3Az\r\n"
"Mi5sb2NhbDEVMBMGA1UECgwMRmFuY3lDb21wYW55MQswCQYDVQQGEwJERTCCASIw\r\n"
"DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANpf4boK1Eox4tNBrePNiSwFs4D5\r\n"
"9vDd/j/HuLtxiQlvcP6k7+jeoVF8dYUh6wzi2o53jTB1FeFJqeJz0SmlFBxVf/Gg\r\n"
"nQZ4ARYZ/EaEgF3/Vec8yEtuPYKC8+REGzmj667T/3Gyqw4rSin11KrVg2NIxNeE\r\n"
"BvDRn7kSIPseCbtlbJaif1XLokeOFyynbr6Cr+H6Co9QPe3SOP4yhrW8efsw5Plj\r\n"
"CrfIacWLB6XiUipPrbV/T5vYh5IuYTjAq1EWrhSv46IyTOQoNV5OEph6ELR3iNwC\r\n"
"StLF/nI97CQPE95vU/N6i7q4sOB9eBRYth6vKzd/HIvgfJ0rNlGqJzQOh60CAwEA\r\n"
"AaMTMBEwDwYDVR0TBAgwBgEB/wIBADANBgkqhkiG9w0BAQsFAAOCAQEA2Q4sNW4D\r\n"
"X4KLIulCZrv0rAR5byGhIQ43nYdMEntlb9Aeb6Pb1wCC0xhVrknce+9jWFimRTVo\r\n"
"EqBPuwb6gXyDtSlwVEAJG+NXQ52x9s/ymbXLQ0zRvRvR6DmHmvjUmxZIsIM2oj1R\r\n"
"cHsyzIStZpWslbdNkZqSxoC3AEzkdcJEYIgiyPOlFM3/cG09no8rpCwPcHLGxCtA\r\n"
"2wzd/z3F4ywxaysmdqVWOizVJOPIN5B5X5wn/fC9n3VuY93K/9Qp1v+TO7YJWsP/\r\n"
"h7ZwlOsXbj2+uGtsXL9jEsl0L9pEmHPQDXX6GUlTPabQTiAIJTIkASh46HOw4XiE\r\n"
"16lLWc41LGJriA==\r\n"
"-----END CERTIFICATE-----\r\n";

String Feedback="", Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
  Serial.println("");
  
  if (cmd=="your cmd") {
    // You can do anything
    // Feedback="{\"data\":\"Sensor data"\"}";
  }
  else if (cmd=="ip") {
    Feedback="{\"data\":\""+WiFi.localIP().toString()+"\"}";
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
    Feedback="{\"data\":\""+WiFi.localIP().toString()+"\"}";
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
    Feedback="{\"data\":\""+String(digitalRead(P1.toInt()))+"\"}";
  }
  else if (cmd=="analogwrite") {
    analogWrite(P1.toInt(), P2.toInt());
  }       
  else if (cmd=="analogread") {
    Feedback="{\"data\":\""+String(analogRead(P1.toInt()))+"\"}";
  }
  else if (cmd=="tcp") {
    String domain=P1;
    int port=P2.toInt();
    String request=P3;
    int wait=P4.toInt();      // wait = 0 or 1

    if ((port==443)||(domain.indexOf("https")==0)||(domain.indexOf("HTTPS")==0))
      Feedback="{\"data\":\""+tcp_https(domain,request,port,wait)+"\"}";
    else
      Feedback="{\"data\":\""+tcp_http(domain,request,port,wait)+"\"}";  
  }
  else if (cmd=="ifttt") {
    String domain="maker.ifttt.com";
    String request = "/trigger/" + P1 + "/with/key/" + P2;
    request += "?value1="+P3+"&value2="+P4+"&value3="+P5;
    Feedback="{\"data\":\""+tcp_https(domain,request,443,0)+"\"}";
  }
  else if (cmd=="thingspeakupdate") {
    String domain="api.thingspeak.com";
    String request = "/update?api_key=" + P1;
    request += "&field1="+P2+"&field2="+P3+"&field3="+P4+"&field4="+P5+"&field5="+P6+"&field6="+P7+"&field7="+P8+"&field8="+P9;
    Feedback="{\"data\":\""+tcp_https(domain,request,443,0)+"\"}";
  }    
  else if (cmd=="thingspeakread") {    //request -> /channels/xxxxxx/fields/1.json?results=2
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
    Feedback="{\"data\":\""+Feedback+"\"}";
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
    Feedback="{\"data\":\""+Feedback+"\"}";
  } 
  else if (cmd=="car") {
    analogWrite(P1.toInt(),0);
    analogWrite(P2.toInt(),0);  
    analogWrite(P3.toInt(),0); 
    analogWrite(P4.toInt(),0);
    delay(10);
    
    if (P8=="S") {
      //
    }
    else if  (P8=="F") {
      analogWrite(P1.toInt(),P5.toInt());
      analogWrite(P4.toInt(),P6.toInt());
      if ((P7!="")&&(P7!="0")) {
        delay(P7.toInt());
        analogWrite(P1.toInt(),0);
        analogWrite(P4.toInt(),0);          
      }     
    }
    else if  (P8=="B") {
      analogWrite(P2.toInt(),P5.toInt());  
      analogWrite(P3.toInt(),P6.toInt());  
      if ((P7!="")&&(P7!="0")) {
        delay(P7.toInt());
        analogWrite(P2.toInt(),0); 
        analogWrite(P3.toInt(),0); 
      }     
    }
    else if  (P8=="L") {
      analogWrite(P2.toInt(),P5.toInt());  
      analogWrite(P4.toInt(),P6.toInt());   
      if ((P7!="")&&(P7!="0")) {
        delay(P7.toInt());
        analogWrite(P2.toInt(),0); 
        analogWrite(P4.toInt(),0);          
      }
    }
    else if  (P8=="R") {
      analogWrite(P1.toInt(),P5.toInt());
      analogWrite(P3.toInt(),P6.toInt());  
      if ((P7!="")&&(P7!="0")) {
        delay(P7.toInt());
        analogWrite(P1.toInt(),0);
        analogWrite(P3.toInt(),0); 
      }        
    }
  } 
  else if (cmd=="i2cLcd") {
    LiquidCrystal_I2C lcd(P1.toInt());
    lcd.begin(16, 2);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(P2);
    lcd.setCursor(0,1);
    lcd.print(P3);
  }
  else {
    Feedback="{\"data\":\"Command is not defined\"}";
  }
  if (Feedback=="") Feedback="{\"data\":\""+Command+"\"}";  
}

void setup()
{
    Serial.begin(9600);
    delay(10);
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
      pinMode(LED_BUILTIN, OUTPUT);
      for (int i=0;i<5;i++)
      {
        digitalWrite(LED_BUILTIN,HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN,LOW);
        delay(100);
      }
      if (LineNotifyToken!="") LineNotify(LineNotifyToken,"message="+WiFi.localIP().toString(),1);
    } 
    else {
      pinMode(LED_BUILTIN, OUTPUT);
      for (int i=0;i<3;i++)
      {
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
            
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: application/json;charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            client.println("Connection: close");
            client.println();
            if (Feedback=="")
              client.println("[{\"data\":\"Hello World\"}]");
            else
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

String tcp_http(String domain,String request,int port,byte wait)
{
  WiFiClient client_tcp;

  if (client_tcp.connect(domain.c_str(), port)) {
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
  TLSClient client_tcp;
  client_tcp.setRootCA(rootCA, sizeof(rootCA));

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
  
  TLSClient client_tcp;
  client_tcp.setRootCA(rootCA, sizeof(rootCA));
  
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
