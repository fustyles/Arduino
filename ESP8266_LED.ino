// Author : ChungYi Fu (Taiwan)  2018-2-5
// ESP8266 
// AP static IP: 192.168.4.1
// Turn Off : http://192.168.4.1/?ip
// Turn On : http://192.168.4.1/?on
// Turn Off : http://192.168.4.1/?off
// STA dynammic IP: 
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
  String str="", cmd="";
  int getstate = 0;
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c = mySerial.read();
      delay(10);
      str = str + String(c);
      if (String(c).indexOf("?")!= -1) getstate=1;
      if (String(c).indexOf(" ")!= -1) getstate=0;
      if ((getstate==1)&&(String(c).indexOf("?")== -1)) cmd = cmd + String(c);
    }  
    Serial.println(str);
  }
  
  if (str.indexOf(" HTTP")!= -1)
  {
    Serial.println("");
    Serial.println("command: " + cmd);
    
    String CID = String(str.charAt(str.indexOf("IPD,")+4));
    
    while (mySerial.available())
    {
      mySerial.read();
    }
    
    if (cmd=="on")
    {
      pinMode(2,OUTPUT);
      digitalWrite(2,HIGH);
      feedback(CID,"<font color=red>TURN ON</font>",32);
    }
    else if (cmd=="off")
    {
      pinMode(2,OUTPUT);
      digitalWrite(2,LOW);  
      feedback(CID,"<font color=blue>TURN OFF</font>",34);
    }
    else if (cmd=="ip")
    {
      mySerial.println("AT+CIFSR");
      mySerial.flush();
      delay(10);  //you can try to change number to get complete data 
      String strIP = "";
      while (mySerial.available())
      {
          strIP = strIP + char(mySerial.read());
      }
      feedback(CID,strIP,(strIP.length()+2));
    }    
    else if (cmd=="your command")
    {
      // you can do anything
      // String yourfeedback = "Hello World";
      // feedback(CID,yourfeedback,(yourfeedback.length()+2));
    }
    else 
    {
      feedback(CID,"FAIL",6);
    }  
  }
}

void SendData(String data,int timelimit)
{
  mySerial.println(data);
  mySerial.flush();
  delay(20);
  waitreply(timelimit);
}

void feedback(String CID,String response,int len)
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
    SendData(response,10000);
    SendData("AT+CIPSEND="+CID+",16",0);
    SendData("</body></html>",2000);
    SendData("AT+CIPCLOSE="+CID,2000);
}

String waitreply(int timelimit)
{
  String str = "";
  int readstate = 0;
  long int starttime = millis();
  while( (starttime + timelimit) > millis())
  {
      while(mySerial.available())
      {
          str = str + char(mySerial.read());
          delay(10);
          readstate=1;
      }
      if (readstate==1) return str;
  } 
  return str;
}
