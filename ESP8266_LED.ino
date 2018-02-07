// Author : ChungYi Fu (Taiwan)  2018-2-7 12:00
// ESP8266 ESP-01 
// command format :  ?command  ?command=num1  ?command=num1,num2
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
  int num1=-1,num2=-1;
  byte ReceiveState=0;
  byte num1State=0;
  byte num2State=0;
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

        if ((String(c).indexOf("=")!=-1)&&(ReceiveState==1)) num1State=1;
        if (((String(c).indexOf(",")!=-1)||(String(c).indexOf(" ")!=-1))&&(ReceiveState==1)) num1State=0;
        if ((num1State==1)&&(String(c).indexOf("=")==-1))
        {
          if (num1==-1) 
            num1=c-'0'; 
          else
            num1=num1*10+(c-'0'); 
        }
        
        if ((String(c).indexOf(",")!=-1)&&(ReceiveState==1)) num2State=1;
        if ((String(c).indexOf(" ")!=-1)&&(ReceiveState==1)) num2State=0;
        if ((num2State==1)&&(String(c).indexOf(",")==-1))
        {
          if (num2==-1) 
            num2=c-'0'; 
          else
            num2=num2*10+(c-'0'); 
        } 
      }
    }  
    Serial.println(ReceiveData);
  }
  
  if (ReceiveData.indexOf(" HTTP")!=-1)
  {
    Serial.println("");
    Serial.println("command: "+command);
    Serial.println(String(num1)+","+String(num2));
    
    String CID=String(ReceiveData.charAt(ReceiveData.indexOf("IPD,")+4));
    
    while (mySerial.available())
    {
      mySerial.read();
    }
    
    if (command=="your command")
      {
        // you can do anything
        
        //Feedback(CID,"<font color=\"red\">Hello World</font>",0);  --> HTML
        //Feedback(CID,"Hello World",1);  --> XML
        //Feedback(CID,"Hello World",2);  --> JSON
        //Feedback(CID,"<html>Hello World</html>",3);  --> Custom definition
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
        Feedback(CID,"<html>"+ReceiveData+"</html>",3);
      }
    else if (command.indexOf("inputpullup=")==0)
      {
        pinMode(num1, INPUT_PULLUP);
        Feedback(CID,"<html>"+command+"</html>",3);
      }  
    else if (command.indexOf("pinmode=")==0)
      {
        pinMode(num1, num2);
        Feedback(CID,"<html>"+command+"</html>",3);
      }        
   else if (command.indexOf("digitalwrite=")==0)
      {
        digitalWrite(num1,num2);
        Feedback(CID,"<html>"+command+"</html>",3);
      }   
    else if (command.indexOf("digitalread=")==0)
      {
        Feedback(CID,"<html>"+String(digitalRead(num1))+"</html>",3);
      }    
    else if (command.indexOf("analogwrite=")==0)
      {
        analogWrite(num1,num2);
        Feedback(CID,"<html>"+command+"</html>",3);
      }       
    else if (command.indexOf("analogread=")==0)
      {
        Feedback(CID,"<html>"+String(analogRead(num1))+"</html>",3);
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
    Response="<?xml version=\"1.0\" encoding=\"UTF-8\"?><ESP8266><Data><Text>"+Response+"</Text></Data></ESP8266>";
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
