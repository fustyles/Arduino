/* 
Arduino Uno(Uart) + ESP8266 ESP-01 (1MB Flash, V2.0_AT_Firmware)
Author : ChungYi Fu (Taiwan)  2018-04-04 07:00
*/

String WIFI_SSID = "";   //your network SSID
String WIFI_PWD  = "";    //your network password

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="",APIP="",APMAC="",STAIP="",STAMAC="";

void setup()
{
  Serial.begin(9600);
  
  //You must change uart baud rate of ESP-01 to 9600.
  mySerial.begin(115200);   //Default uart baud rate -> 19200,38400,57600,74880,115200
  SendData("AT+UART_CUR=9600,8,1,0,0",2000);   //Change uart baud rate of ESP-01 to 9600
  mySerial.begin(9600);  // 9600 ,you will get more stable data.
  mySerial.setTimeout(10);
  
  SendData("AT+CWMODE_CUR=1",2000);
  SendData("AT+CIPMUX=0",2000);
  SendData("AT+CIPSERVER=0",2000);   //port=80
  if (WIFI_SSID!="") 
    SendData("AT+CWJAP_CUR=\""+WIFI_SSID+"\",\""+WIFI_PWD+"\"",5000);  
  else
    Serial.print("Please check your network SSID and password");  
}

void loop() 
{
  getSTAIP();

  if (STAIP!="")
  {
    int val = rand()%256;
    String Domain="192.168.201.10";
    String request = "GET /?analogwrite=4;"+String(val)+" HTTP/1.1\r\n\r\n";
    SendData("AT+CIPSTART=\"TCP\",\""+Domain+"\",80", 4000);
    SendData("AT+CIPSEND=" + String(request.length()+2), 4000);
    SendData(request, 2000);
    delay(1);
    SendData("AT+CIPCLOSE",2000);
    
    Serial.println(val);
    delay(5000);  
  }
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
  String ReceiveData="";
  long int StartTime=millis();
  while( (StartTime+TimeLimit) > millis())
  {
    if (mySerial.available())
    {
      delay(4);
      while(mySerial.available())
      {
        ReceiveData=ReceiveData+String(char(mySerial.read()));
        //ReceiveData=ReceiveData+mySerial.readStringUntil('\r'); 
      }
      //Serial.println(ReceiveData);
      return ReceiveData;
    }
  } 
  return ReceiveData;
}

void getSTAIP()
{
  ReceiveData="";
  
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c=mySerial.read();
      ReceiveData=ReceiveData+String(c);
    } 
    
    if (ReceiveData.indexOf("WIFI GOT IP")!=-1)
    { 
      while(!mySerial.find('OK')){} 
      delay(10);

      int apipreadstate=0,staipreadstate=0,apmacreadstate=0,stamacreadstate=0,j=0;
      mySerial.println("AT+CIFSR");
      mySerial.flush();
      delay(6);
      
      while(mySerial.available())
      {
        char c=mySerial.read();
        String t=String(c);
        
        if (t=="\"") j++;
        
        if (j==1) 
          apipreadstate=1;
        else if (j==2)
          apipreadstate=0;
        if ((apipreadstate==1)&&(t!="\"")) STAIP=STAIP+t;

        if (j==3) 
          apmacreadstate=1;
        else if (j==4)
          apmacreadstate=0;
        if ((apmacreadstate==1)&&(t!="\"")) STAMAC=STAMAC+t;
        
        delay(1);
      } 
      Serial.println("STAIP: "+STAIP+"\nSTAMAC: "+STAMAC);
    }
  }
}
