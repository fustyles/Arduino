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
  while (!Serial) {}
  
  mySerial.begin(9600);

  mySerial.println("AT+RST");
  waitreply(10000);
  mySerial.println("AT+CWMODE=3");
  waitreply(1000);
  mySerial.println("AT+CIPMUX=1");
  waitreply(1000);
  mySerial.println("AT+CIPSERVER=1,80");
  waitreply(1000);
  mySerial.println("AT+CWJAP=\""+wifi_id+"\",\""+wifi_pwd+"\"");
  waitreply(10000);
}
void loop() 
{
  String str="";
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c = mySerial.read();
      str += char(toupper(c));
      Serial.print(c);
      delay(10);
      if (str.indexOf("HTTP")!= -1) break;
    }  
  }

  if (str.indexOf("?TURNON")!= -1)
  {
    digitalWrite(pinLED,1);
    feedback(str,"<font color=red>TURN ON</font>",32);
  }
  else if (str.indexOf("?TURNOFF")!= -1)
  {
    digitalWrite(pinLED,0);  
    feedback(str,"<font color=blue>TURN OFF</font>",34);
  }
  else if (str.indexOf("IPD,")!= -1)
  {
    feedback(str,"FAIL",6);
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
    delay(50);
    mySerial.println("<html>");
    waitreply(2000);

    mySerial.println("AT+CIPSEND="+CID+",8");
    delay(50);
    mySerial.println("<head>");
    waitreply(2000);

    mySerial.println("AT+CIPSEND="+CID+",69");
    delay(100);
    mySerial.println("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
    waitreply(10000);    
    
    mySerial.println("AT+CIPSEND="+CID+",9");
    delay(50);
    mySerial.println("</head>");
    waitreply(2000);
    
    mySerial.println("AT+CIPSEND="+CID+",8");
    delay(50);
    mySerial.println("<body>");
    waitreply(2000);    
    
    mySerial.println("AT+CIPSEND="+CID+","+String(len));
    delay(50);
    mySerial.println(response);
    waitreply(10000);
    
    mySerial.println("AT+CIPSEND="+CID+",16");
    delay(50);
    mySerial.println("</body></html>");
    waitreply(2000);
    mySerial.println("AT+CIPCLOSE="+CID);
}
