/* 
ESP32

Author : ChungYi Fu (Taiwan)  2018-3-16 22:00

Command Format :  ?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

http://192.168.x.x/?inputpullup=13
http://192.168.x.x/?pinmode=13;1
http://192.168.x.x/?digitalwrite=13;1
http://192.168.x.x/?analogwrite=13;255
http://192.168.x.x/?digitalread=13
http://192.168.x.x/?analogread=13

*/

#include <WiFi.h>

const char* ssid     = "";   //your network SSID
const char* password = "";   //your network password

WiFiServer server(80);

String Feedback="", Command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";


void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  Serial.println("");
  
  if (cmd=="inputpullup")
  {
    pinMode(str1.toInt(), INPUT_PULLUP);
    Feedback=Command;
  }  
  else if (cmd=="pinmode")
  {
    pinMode(str1.toInt(), str2.toInt());
    Feedback=Command;
  }        
  else if (cmd=="digitalwrite")
  {
    pinMode(str1.toInt(), OUTPUT);
    digitalWrite(str1.toInt(),str2.toInt());
    Feedback=Command;
  }   
  else if (cmd=="digitalread")
  {
    Feedback=String(digitalRead(str1.toInt()));
  }
  else if (cmd=="analogwrite")
  {
    ledcAttachPin(str1.toInt(), 1);
    ledcSetup(1, 12000, 8);
    ledcWrite(1,str2.toInt());
    Feedback=Command;
  }       
  else if (cmd=="analogread")
  {
    Feedback=String(analogRead(str1.toInt()));
  }
  else 
  {
    Feedback="Command is not defined";
  }             
}

void setup()
{
    Serial.begin(115200);
    delay(10);
    
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

}

void loop(){
 WiFiClient client = server.available();

  if (client) { 
    //Serial.println("New Client.");
    String currentLine = "";

    Command="";cmd="";str1="";str2="";str3="";str4="";str5="";str6="";str7="";str8="";str9="";
    byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

    while (client.connected()) {
      
      if (client.available()) {
        char c = client.read();             
        
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
                
        if (c == '\n') {
          if (currentLine.length() == 0) {
            if (Feedback=="") Feedback="Hello World";
              
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(Feedback);
            client.println();
              
            Feedback="";
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if ((currentLine.indexOf("?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1))
        {
          currentLine="";
          Feedback="";
          ExecuteCommand();
        }
      }
    }
    client.stop();
    //Serial.println("Client Disconnected.");
  }
}
