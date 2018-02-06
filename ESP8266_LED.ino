// Author : ChungYi Fu (Taiwan)  2018-2-7 02:30
// ESP8266 ESP-01 
// command format :  ?cmd  ?cmd=num1  ?cmd=num1,num2
// AP IP： 192.168.4.1
// http://192.168.4.1/?inputpullup=3
// http://192.168.4.1/?pinmode=3,1
// http://192.168.4.1/?digitalwrite=3,1
// http://192.168.4.1/?analogwrite=3,200
// http://192.168.4.1/?digitalread=3
// http://192.168.4.1/?analogread=3
// STA IP：
// query： http://192.168.4.1/?ip


#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String SSID="id";
String PWD="pwd";

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);

  SendData("AT+RST",10000);
  SendData("AT+CWMODE=3",2000);
  SendData("AT+CIPMUX=1",2000);
  SendData("AT+CIPSERVER=1,80",2000);
  //SendData("AT+CIPSTA=\"192.168.0.3\",\"192.168.0.1\",\"255.255.255.0\"",2000);
  SendData("AT+CWJAP=\""+SSID+"\",\""+PWD+"\"",10000);
}

void loop() 
{
  String ReceiveData="", command="";
  int pin=-1,val=-1;
  byte ReceiveState=0;
  byte pinState=0;
  byte setvalueState=0;
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c=mySerial.read();
      delay(10);
      ReceiveData=ReceiveData+String(c);
      if (String(c).indexOf("?")!=-1) ReceiveState=1;
      if (String(c).indexOf(" ")!=-1) ReceiveState=0;
      if ((ReceiveState==1)&&(String(c).indexOf("?")==-1)) 
      {
        command=command+String(c);

        if ((String(c).indexOf("=")!=-1)&&(ReceiveState==1)) pinState=1;
        if (((String(c).indexOf(",")!=-1)||(String(c).indexOf(" ")!=-1))&&(ReceiveState==1)) pinState=0;
        if ((pinState==1)&&(String(c).indexOf("=")==-1))
        {
          if (pin==-1) 
            pin=c-'0'; 
          else
            pin=pin*10+(c-'0'); 
        }
        if ((String(c).indexOf(",")!=-1)&&(ReceiveState==1)) setvalueState=1;
        if ((String(c).indexOf(" ")!=-1)&&(ReceiveState==1)) setvalueState=0;
        if ((setvalueState==1)&&(String(c).indexOf(",")==-1))
        {
          if (val==-1) 
            val=c-'0'; 
          else
            val=val*10+(c-'0'); 
        } 
      }
    }  
    Serial.println(ReceiveData);
    Serial.println(pin);
    Serial.println(val);
  }
  
  if (ReceiveData.indexOf(" HTTP")!=-1)
  {
    Serial.println("");
    Serial.println("command: "+command);
    
    String CID=String(ReceiveData.charAt(ReceiveData.indexOf("IPD,")+4));
    
    while (mySerial.available())
    {
      mySerial.read();
    }
    
    if (command=="ip")
      {
        mySerial.println("AT+CIFSR");
        mySerial.flush();
        delay(5);  //you can try to change number to get complete data 
        ReceiveData="";
        while (mySerial.available())
        {
            ReceiveData=ReceiveData+char(mySerial.read());
        }
        Feedback(CID,ReceiveData,0);
      }
    else if (command.indexOf("inputpullup=")!=-1)
      {
        pinMode(pin, INPUT_PULLUP);
        Feedback(CID,"<html>"+command+"</html>",-1);
      }  
    else if (command.indexOf("pinmode=")!=-1)
      {
        pinMode(pin, val);
        Feedback(CID,"<html>"+command+"</html>",-1);
      }        
   else if (command.indexOf("digitalwrite=")!=-1)
      {
        digitalWrite(pin,val);
        Feedback(CID,"<html>"+command+"</html>",-1);
      }   
    else if (command.indexOf("digitalread=")!=-1)
      {
        delay(100);
        Feedback(CID,"<html>"+String(digitalRead(pin))+"</html>",-1);
      }    
    else if (command.indexOf("analogwrite=")!=-1)
      {
        analogWrite(pin,val);
        Feedback(CID,"<html>"+command+"</html>",-1);
      }       
    else if (command.indexOf("analogread=")!=-1)
      {
        delay(100);
        Feedback(CID,"<html>"+String(analogRead(pin))+"</html>",-1);
      }           
    else if (command=="your command")
      {
        // you can do anything
        
        //Feedback(CID,"<font color=\"red\">TURN ON</font>",0);  --> HTML
        //Feedback(CID,"TURN ON",1);  --> XML
        //Feedback(CID,"TURN ON",2);  --> JSON
        //Feedback(CID,"<html>TURN ON</html>",-1);  --> Custom definition
      }
    else 
      {
        Feedback(CID,"FAIL",0);
      }  
  }
}

void SendData(String data,int TimeLimit)
{
  mySerial.println(data);
  mySerial.flush();
  delay(20);
  WaitReply(TimeLimit);
}

void Feedback(String CID,String Response,byte datatype)
{
  if (datatype==0)
  {
    Response="<!DOCTYPE HTML><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head><body>"+Response+"</body></html>";
  }
  else if (datatype==1) 
  {
    Response="<?xml version=\"1.0\" encoding=\"UTF-8\"?><ESP8266><Data><TEXT>"+Response+"</TEXT></Data></ESP8266>";
  }  
  else if (datatype==2) 
  {
    Response="[{\"ESP8266\":\""+Response+"\"}]";
  }
  else
    Response=Response;

  SendData("AT+CIPSEND="+CID+","+(Response.length()+2),2000);
  SendData(Response,2000);
  SendData("AT+CIPCLOSE="+CID,2000);
}

String WaitReply(long int TimeLimit)
{
  String ReceiveData="";
  byte ReceiveState=0;
  long int StartTime=millis();
  while( (StartTime+TimeLimit) > millis())
  {
      while(mySerial.available())
      {
          ReceiveData=ReceiveData+char(mySerial.read());
          delay(10);
          ReceiveState=1;
      }
      if (ReceiveState==1) return ReceiveData;
  } 
  return ReceiveData;
}
