// Author : ChungYi Fu (Taiwan)  2018-2-6 14:00
// ESP8266 ESP-01 
// AP Static IP: 192.168.4.1
// Query IP : http://192.168.4.1/?ip
// Turn On : http://192.168.4.1/?on
// Turn Off : http://192.168.4.1/?off
// STA Dynammic IP: 
// Query IP : http://192.168.xxx.xxx/?ip
// Turn On : http://192.168.xxx.xxx/?on
// Turn Off : http://192.168.xxx.xxx/?off


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
  byte ReceiveState=0;
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c=mySerial.read();
      delay(10);
      ReceiveData=ReceiveData+String(c);
      if (String(c).indexOf("?")!=-1) ReceiveState=1;
      if (String(c).indexOf(" ")!=-1) ReceiveState=0;
      if ((ReceiveState==1)&&(String(c).indexOf("?")==-1)) command=command+String(c);
    }  
    Serial.println(ReceiveData);
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
    
    if (command=="on")
    {
      pinMode(2,OUTPUT);
      digitalWrite(2,HIGH);
      Feedback(CID,"<font color=\"red\">TURN ON</font>",0);
    }
    else if (command=="off")
    {
      pinMode(2,OUTPUT);
      digitalWrite(2,LOW);  
      Feedback(CID,"<font color=\"blue\">TURN OFF</font>",0);
    }
    else if (command=="ip")
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
    else if (command=="your command")
    {
      // you can do anything
      
      //Feedback(CID,"<font color=\"red\">TURN ON</font>",0);  --> HTML
      //Feedback(CID,"TURN ON",1);  --> XML
      //Feedback(CID,"<html>TURN ON</html>",2);  --> Custom definition
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
    Response="<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head><body>"+Response+"</body></html>";
  else if (datatype==1) 
    Response="<?xml version=\"1.0\" encoding=\"UTF-8\"?><ESP8266><Data><TEXT>"+Response+"</TEXT></Data></ESP8266>";
  else
    Response=Response;
    
  SendData("AT+CIPSEND="+CID+","+(Response.length()+2),2000);
  SendData(Response,10000);
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
