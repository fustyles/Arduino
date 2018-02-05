// Author : ChungYi Fu (Taiwan)  2018-2-5 22:40
// ESP8266 
// AP Static IP: 192.168.4.1
// Turn Off : http://192.168.4.1/?ip
// Turn On : http://192.168.4.1/?on
// Turn Off : http://192.168.4.1/?off
// STA Dynammic IP: 
// Turn Off : http://192.168.xxx.xxx/?ip
// Turn On : http://192.168.xxx.xxx/?on
// Turn Off : http://192.168.xxx.xxx/?off


#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String SSID = "id";
String PWD = "pwd";

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
  String ReceiveData="", Cmd="";
  byte ReceiveState = 0;
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c = mySerial.read();
      delay(10);
      ReceiveData = ReceiveData + String(c);
      if (String(c).indexOf("?")!= -1) ReceiveState=1;
      if (String(c).indexOf(" ")!= -1) ReceiveState=0;
      if ((ReceiveState==1)&&(String(c).indexOf("?")== -1)) Cmd = Cmd + String(c);
    }  
    Serial.println(ReceiveData);
  }
  
  if (ReceiveData.indexOf(" HTTP")!= -1)
  {
    Serial.println("");
    Serial.println("command: " + Cmd);
    
    String CID = String(ReceiveData.charAt(ReceiveData.indexOf("IPD,")+4));
    
    while (mySerial.available())
    {
      mySerial.read();
    }
    
    if (Cmd=="on")
    {
      pinMode(2,OUTPUT);
      digitalWrite(2,HIGH);
      Feedback(CID,"<font color=red>TURN ON</font>",32);
    }
    else if (Cmd=="off")
    {
      pinMode(2,OUTPUT);
      digitalWrite(2,LOW);  
      Feedback(CID,"<font color=blue>TURN OFF</font>",34);
    }
    else if (Cmd=="ip")
    {
      mySerial.println("AT+CIFSR");
      mySerial.flush();
      delay(10);  //you can try to change number to get complete data 
      ReceiveData = "";
      while (mySerial.available())
      {
          ReceiveData = ReceiveData + char(mySerial.read());
      }
      Feedback(CID,ReceiveData,(ReceiveData.length()+2));
    }    
    else if (Cmd=="your command")
    {
      // you can do anything
      // String Response = "Hello World";
      // Feedback(CID,Response,(Response.length()+2));
    }
    else 
    {
      Feedback(CID,"FAIL",6);
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

void Feedback(String CID,String Response,int len)
{
    SendData("AT+CIPSEND="+CID+",8",0);
    SendData("<html>",2000);
    SendData("AT+CIPSEND="+CID+",8",0);
    SendData("<head>",2000);
    SendData("AT+CIPSEND="+CID+",69",0);
    SendData("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">",10000);
    SendData("AT+CIPSEND="+CID+",9",0);
    SendData("</head>",2000);
    SendData("AT+CIPSEND="+CID+",8",0);
    SendData("<body>",2000);
    SendData("AT+CIPSEND="+CID+","+String(len),0);
    SendData(Response,10000);
    SendData("AT+CIPSEND="+CID+",16",0);
    SendData("</body></html>",2000);
    SendData("AT+CIPCLOSE="+CID,2000);
}

String WaitReply(int TimeLimit)
{
  String ReceiveData = "";
  byte ReceiveState = 0;
  long int StartTime = millis();
  while( (StartTime + TimeLimit) > millis())
  {
      while(mySerial.available())
      {
          ReceiveData = ReceiveData + char(mySerial.read());
          delay(10);
          ReceiveState=1;
      }
      if (ReceiveState==1) return ReceiveData;
  } 
  return ReceiveData;
}
