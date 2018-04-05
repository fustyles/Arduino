/* 
Arduino Uno(Uart) + ESP8266 STATION ESP-01 (1MB Flash, V2.0_AT_Firmware)
Author : ChungYi Fu (Taiwan)  2018-3-29 17:00
*/

// Check your Wi-Fi Router's Settings
String WIFI_SSID = "ESP8266";   //your network SSID
String WIFI_PWD  = "12345678";    //your network password

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="";

void setup()
{
  Serial.begin(9600);
  
  //You must change uart baud rate of ESP-01 to 9600.
  mySerial.begin(115200);   //Default uart baud rate -> 19200,38400,57600,74880,115200
  SendData("AT+UART_CUR=9600,8,1,0,0",2000);   //Change uart baud rate of ESP-01 to 9600
  mySerial.begin(9600);  // 9600 ,you will get more stable data.
  mySerial.setTimeout(10);
  
  SendData("AT+CWMODE_CUR=0",2000);
  SendData("AT+CIPMUX=0",2000);
  //String STA_ip="192.168.0.100";
  //String STA_gateway="192.168.0.1";
  //String STA_netmask="255.255.255.0";
  //SendData("AT+CIPSTA_CUR=\""+STA_ip+"\",\""+STA_gateway+"\",\""+STA_netmask+"\"",2000);
  
  if (WIFI_SSID!="") 
  {
    SendData("AT+CWJAP_CUR=\""+WIFI_SSID+"\",\""+WIFI_PWD+"\"",4000); 
  }
  else
    Serial.print("Please check your network SSID and password");  
}

void loop() 
{
  int x = rand() % 101;
  String request = "GET /update?api_key=123456789"+
  request+="&field1="+String(x);
  request+=" HTTP/1.1\r\n";
  request+="Host: api.thingspeak.com\r\n";
  request+="Connection: close\r\n\r\n";
  Serial.println(x);
  
  SendData("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80", 4000);
  SendData("AT+CIPSEND=" + String(request.length()+2), 2000);
  SendData(request, 2000);
  SendData("AT+CIPCLOSE",2000);
  delay(30000);
}

void SendData(String data,int TimeLimit)
{
  mySerial.println(data);
  mySerial.flush();
  delay(10);
  WaitReply(TimeLimit);
}

String WaitReply(long int TimeLimit)
{
  ReceiveData="";
  long int StartTime=millis();
  while( (StartTime+TimeLimit) > millis())
  {
    if (mySerial.available())
    {
      delay(4);
      while(mySerial.available())
      {
        char c=mySerial.read();
        ReceiveData=ReceiveData+String(c);
        //Serial.print(c);
      }
      return ReceiveData;
    }
  } 
  return ReceiveData;
}

void getSTA()
{
    if (ReceiveData.indexOf("WIFI GOT IP")!=-1)
    { 
      while(!mySerial.find('OK')){} 
      delay(10);

      String STAIP="",STAMAC="";
      int staipreadstate=0,stamacreadstate=0,j=0;
      mySerial.println("AT+CIFSR");
      mySerial.flush();
      delay(6);
      
      while(mySerial.available())
      {
        char c=mySerial.read();
        String t=String(c);
        
        if (t=="\"") j++;
        
        if (j==1) 
          staipreadstate=1;
        else if (j==2)
          staipreadstate=0;
        if ((staipreadstate==1)&&(t!="\"")) STAIP=STAIP+t;

        if (j==3) 
          stamacreadstate=1;
        else if (j==4)
          stamacreadstate=0;
        if ((stamacreadstate==1)&&(t!="\"")) STAMAC=STAMAC+t;
      } 
      
      pinMode(13,1);
      for (int i=0;i<20;i++)
      {
        digitalWrite(13,1);
        delay(50);
        digitalWrite(13,0);
        delay(50);
      }
      
      Serial.println("STAIP: "+STAIP+"\nSTAMAC: "+STAMAC);
    }
}
