/* 
ESP-01 + Arduino Uno (AT Command)

Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-08-05 22:00 
https://www.facebook.com/francefu

Expanding Arduino Serial Port Buffer Size
https://internetofhomethings.com/homethings/?p=927
https://www.facebook.com/francefu/videos/10211231615856099/
*/

// Enter your WiFi ssid and password
String WIFI_SSID = "";   //your network SSID
String WIFI_PWD  = "";    //your network password

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11);  // ESP01 TX->D10, RX->D11  

String ReceiveData="",STAIP="",STAMAC="";

void setup()
{
  Serial.begin(9600);
  
  //You must change baud rate to 9600.
  mySerial.begin(115200);   //Default baud rate
  SendData("AT+UART_CUR=9600,8,1,0,0",2000);   //Change baud rate to 9600
  mySerial.begin(9600);  // you will get more stable data without junk chars.
  mySerial.setTimeout(10);

  SendData("AT+CIPSERVER=0",2000);
  SendData("AT+CWMODE_CUR=1",2000);
  SendData("AT+CIPMUX=0",2000);
  
  if (WIFI_SSID!="") 
    SendData("AT+CWJAP_CUR=\""+WIFI_SSID+"\",\""+WIFI_PWD+"\"",5000);  
  else
    Serial.print("Please check your network SSID and password settings");  
}

void loop() 
{
  if (STAIP=="")
  {
    getSTAIP();
  
    if (STAIP!="")
    {
      pinMode(13,1);
      for (int i=0;i<20;i++)
      {
        digitalWrite(13,1);
        delay(50);
        digitalWrite(13,0);
        delay(50);
      }      
    }
  }
  else
  {
    //Get sensor Data from Arduino Uno
    int SensorTemperature = rand()%40;    
    int SensorHumidity = rand()%100;  

    //ThingSpeak
    String domain="api.thingspeak.com";
    String key="xxxxxxxxxxxxxxxxxxxx";
    String field1=String(SensorTemperature);
    String field2=String(SensorHumidity);
    /*
    If request length is too long, it can't work!
    If you change buffer size to 256 bytes, request length must be less than or equal to 128.
    */
    String request="GET /update?api_key="+key+"&field1="+field1+"&field2="+field2+" HTTP/1.1\r\nHost: "+domain+"\r\n\r\n";
    Serial.println(request);
    
    SendData("AT+CIPSTART=\"TCP\",\""+domain+"\",80", 4000);
    SendData("AT+CIPSEND=" + String(request.length()+2), 2000);  //Serial.println(request) -> request+"\r\n" -> request.length()+2 
    SendData(request, 4000);
    SendData("AT+CIPCLOSE",2000);
    
    // alarm (IFTTT: Webhooks+Line)
    if ((SensorTemperature<10)||(SensorHumidity<20))
    {  
      delay(1000);
      //IFTTT      
      String domain="maker.ifttt.com";
      String event="xxxxxx";
      String key="xxxxxxxxxxxxxxxxxxxx";
      String value1=String(SensorTemperature);
      String value2=String(SensorHumidity);
      /*
      If request length is too long, it can't work!
      Expanding Arduino Serial Port Buffer Size
      https://internetofhomethings.com/homethings/?p=927
      If you change buffer size to 256 bytes, request length must be less than or equal to 128.
      */
      request = "GET /trigger/"+event+"/with/key/"+key+"?value1="+value1+"&value2="+value2+" HTTP/1.1\r\nHost: "+domain+"\r\n\r\n";
      Serial.println(request);
      
      SendData("AT+CIPSTART=\"TCP\",\""+domain+"\",80", 4000);
      SendData("AT+CIPSEND=" + String(request.length()+2), 2000);  //Serial.println(request) -> request+"\r\n" -> request.length()+2
      SendData(request, 4000);
      SendData("AT+CIPCLOSE",2000);
    }  
    
    delay(20000);  // Time interval should be more than or equal to 15 seconds.(ThingSpeak)
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
  ReceiveData="";
  long int StartTime=millis();
  while( (StartTime+TimeLimit) > millis())
  {
    if (mySerial.available())
    {
      delay(4);
      while(mySerial.available())
      {
        ReceiveData=ReceiveData+String(char(mySerial.read()));
      }
      Serial.println(ReceiveData);
      if (ReceiveData.indexOf("OK")!=-1) return ReceiveData;
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
        
        delay(1);
      } 
      Serial.println("STAIP: "+STAIP+"\nSTAMAC: "+STAMAC+"\n");
    }
  }
}
