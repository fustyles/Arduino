//Author : ChungYi Fu

#include <SoftwareSerial.h>
SoftwareSerial mySerial(4, 5); // Arduino RX:4, TX:5 

int pinLED = 2;
String wifi_id = "id";
String wifi_pwd = "pwd";

void setup()
{
  pinMode(pinLED,OUTPUT);
  
  Serial.begin(9600);
  mySerial.begin(9600);

  mySerial.println("AT+RST");
  mySerial.flush();
  waitreply(10000);
  mySerial.println("AT+CWMODE=3");
  mySerial.flush();
  waitreply(1000);
  mySerial.println("AT+CIPMUX=1");
  mySerial.flush();
  waitreply(1000);
  mySerial.println("AT+CIPSERVER=1,80");
  mySerial.flush();
  waitreply(1000);
  mySerial.println("AT+CWJAP=\""+wifi_id+"\",\""+wifi_pwd+"\"");
  mySerial.flush();
  waitreply(10000);
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
      Serial.print(c);
      delay(10);
      str = str + char(toupper(c));
      if (String(c).indexOf("?")!= -1) getstate=1;
      if (String(c).indexOf(" ")!= -1) getstate=0;
      if (getstate==1) cmd = cmd + char(toupper(c));
      if ((str.indexOf("?")!= -1)&&(str.indexOf("HTTP")!= -1)) break;
    }  
  }
  if (str.indexOf("HTTP")!= -1)
  {
    Serial.println("");
    Serial.println("command: "+cmd);
    
    if (cmd=="?TURNON")
    {
      digitalWrite(pinLED,1);
      feedback(str,"<font color=red>TURN ON</font>",32);
    }
    else if (cmd=="?TURNOFF")
    {
      digitalWrite(pinLED,0);  
      feedback(str,"<font color=blue>TURN OFF</font>",34);
    }
    else if (str.indexOf("IPD,")!= -1)
    {
      feedback(str,"FAIL",6);
    }  
  }
}

void waitreply(int timelimit)
{
    int st = 0;
    long int starttime = millis();
    while( (starttime + timelimit) > millis())
    {
        while(mySerial.available())
        {
            char c = mySerial.read(); 
            delay(10);
            st=1;
        }
        if (st==1) return;
    } 
}

void feedback(String str,String response,int len)
{
    String CID = String(str.charAt(str.indexOf("IPD,")+4));
    
    mySerial.println("AT+CIPSEND="+CID+",8");
    mySerial.flush();
    delay(100);
    mySerial.println("<html>");
    mySerial.flush();
    waitreply(2000);

    mySerial.println("AT+CIPSEND="+CID+",8");
    mySerial.flush();
    delay(100);
    mySerial.println("<head>");
    mySerial.flush();
    waitreply(2000);

    mySerial.println("AT+CIPSEND="+CID+",69");
    mySerial.flush();
    delay(100);
    mySerial.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
    mySerial.flush();
    waitreply(10000);    
    
    mySerial.println("AT+CIPSEND="+CID+",9");
    mySerial.flush();
    delay(100);
    mySerial.println("</head>");
    mySerial.flush();
    waitreply(2000);
    
    mySerial.println("AT+CIPSEND="+CID+",8");
    mySerial.flush();
    delay(100);
    mySerial.println("<body>");
    mySerial.flush();
    waitreply(2000);    
    
    mySerial.println("AT+CIPSEND="+CID+","+String(len));
    mySerial.flush();
    delay(200);
    mySerial.println(response);
    mySerial.flush();
    waitreply(20000);
    
    mySerial.println("AT+CIPSEND="+CID+",16");
    mySerial.flush();
    delay(100);
    mySerial.println("</body></html>");
    mySerial.flush();
    waitreply(2000);
    
    mySerial.println("AT+CIPCLOSE="+CID);
    mySerial.flush();
}
