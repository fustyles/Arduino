/* 
ESP8266 ESP-01
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-2-15 10:00
Command format :
?cmd  
Number： ?cmd=num1  ?cmd=num1,num2   (?)
String ： ?&cmd=str1  ?&cmd=str1,str2   (?&)
Number+String ： ?+cmd=num1,str2   (?+)
AP IP： 192.168.4.1
http://192.168.4.1/?&resetwifi=id,pwd
http://192.168.4.1/?ip
http://192.168.4.1/?&at=AT+Command
http://192.168.4.1/?inputpullup=3
http://192.168.4.1/?pinmode=3,1
http://192.168.4.1/?digitalwrite=3,1
http://192.168.4.1/?analogwrite=3,200
http://192.168.4.1/?digitalread=3
http://192.168.4.1/?analogread=3
http://192.168.4.1/?yourcmd=1,180
http://192.168.4.1/?&yourcmd=Hello,World
http://192.168.4.1/?+yourcmd=100,Hello
http://192.168.4.1/?&tcp=parameter,ip,port
STA IP：
Query： http://192.168.4.1/?ip
Link：http://192.168.4.1/?&resetwifi=id,pwd
*/

// Check your Wi-Fi Router's Settings
String WIFI_SSID="yourwifi_id";
String WIFI_PWD="yourwifi_pwd";

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="", command="",cmd="",str1="",str2="";
long int num1=-1,num2=-1;
String APIP="",STAIP="";

void setup()
{
  Serial.begin(9600);
  
  SendData("AT+RST",5000);
  //You must change uart baud rate of ESP-01 to 9600.
  mySerial.begin(115200);   //Default uart baud rate -> 19200,38400,57600,74880,115200
  SendData("AT+UART_CUR=9600,8,1,0,0",2000);   //Change uart baud rate of ESP-01 to 9600
  mySerial.begin(9600);  // 9600 ,you will get more stable data.
  
  initial();
}

void loop() 
{
  getVariable();
  
  if ((ReceiveData.indexOf("?")!=-1)&&(ReceiveData.indexOf(" HTTP")!=-1))
  {
    Serial.println("");
    Serial.println("command: "+command);
    Serial.println("cmd= "+cmd);
    Serial.println("num1= "+String(num1)+" ,num2= "+String(num2));
    Serial.println("str1= "+String(str1)+" ,str2= "+String(str2));
    
    String CID=String(ReceiveData.charAt(ReceiveData.indexOf("+IPD,")+5));
    
    if (cmd=="yourcmd")
      {
        //you can do anything
        
        //Feedback(CID,"<font color=\"red\">"+cmd+"="+num1+","+num2+"</font>",0);  --> HTML
        //Feedback(CID,cmd+"="+num1+","+num2,1);  --> XML
        //Feedback(CID,cmd+"="+num1+","+num2,2);  --> JSON
        //Feedback(CID,"<html>"+cmd+"="+num1+","+num2+"</html>",3);  --> Custom definition
      }
    else if (cmd=="&yourcmd")
      {
         //you can do anything
         //Feedback(CID,"<html>"+cmd+"="+str1+","+str2+"</html>",3);
      }
    else if (cmd=="+yourcmd")
      {
         //you can do anything
         //Feedback(CID,"<html>"+cmd+"="+String(num1)+","+str2+"</html>",3);
      }    
    else if (cmd=="ip")
      {
        Feedback(CID,"<html>APIP: "+APIP+"<br>STAIP: "+STAIP+"</html>",3);
      }
    else if (cmd=="&at")      //  ?&cmd=,str2 -> ?&at=,AT+RST
      {
        mySerial.println(str2);
        mySerial.flush();
        Feedback(CID,"<html>"+WaitReply(5000)+"</html>",3);
      }
    else if (cmd=="&tcp")      // ?&tcp=parameter,www.google.com,80
      {
        String getcommand="GET /"+str1;
        SendData("AT+CIPSTART=0,\"TCP\",\""+str2+"\"",2000);
        SendData("AT+CIPSEND=0,"+String(getcommand.length()+2),2000);
        SendData(getcommand,2000);
        Feedback("0","<html>"+WaitReply(10000)+"</html>",3);
      }      
    else if (cmd=="inputpullup")
      {
        pinMode(num1, INPUT_PULLUP);
        Feedback(CID,"<html>"+command+"</html>",3);
      }  
    else if (cmd=="pinmode")
      {
        pinMode(num1, num2);
        Feedback(CID,"<html>"+command+"</html>",3);
      }        
    else if (cmd=="digitalwrite")
      {
        digitalWrite(num1,num2);
        Feedback(CID,"<html>"+command+"</html>",3);
      }   
    else if (cmd=="digitalread")
      {
        Feedback(CID,"<html>"+String(digitalRead(num1))+"</html>",3);
      }    
    else if (cmd=="analogwrite")
      {
        analogWrite(num1,num2);
        Feedback(CID,"<html>"+command+"</html>",3);
      }       
    else if (cmd=="analogread")
      {
        Feedback(CID,"<html>"+String(analogRead(num1))+"</html>",3);
      }    
    else if (cmd=="&resetwifi")
      {
        Feedback(CID,"<html>"+str1+","+str2+"</html>",3);
        delay(3000);
        WIFI_SSID=str1;
        WIFI_PWD=str2;
        SendData("AT+CWJAP_CUR=\""+WIFI_SSID+"\",\""+WIFI_PWD+"\"",5000);
      }
    else 
      {
        Feedback(CID,"<html>Command is not defined</html>",3);
      }  
  }
  else if ((ReceiveData.indexOf("?")!=-1)&&(ReceiveData.indexOf(" HTTP")==-1))
  {
    if(ReceiveData.indexOf("IPD,")!=-1)
    {
      String CID=String(ReceiveData.charAt(ReceiveData.indexOf("+IPD,")+5));
      Feedback(CID,"<html>FAIL</html>",3);
    }
  }
  else if ((ReceiveData.indexOf("?")==-1)&&(ReceiveData.indexOf(" HTTP")!=-1))
  {
    String CID=String(ReceiveData.charAt(ReceiveData.indexOf("+IPD,")+5));
    Feedback(CID,"<html>Hello World</html>",3);
  }
}

void initial()
{
  SendData("AT+CWMODE_CUR=3",2000);
  SendData("AT+CIPMUX=1",2000);
  SendData("AT+CIPSERVER=1,80",2000);   //port=80
  SendData("AT+CIPSTO=5",2000);  //timeout= 5 seconds
  //SendData("AT+CWSAP_CUR=\"AP_id\",\"AP_pwd\",3,4",2000);
  //String STA_ip="192.168.0.100";
  //String STA_gateway="192.168.0.1";
  //String STA_netmask="255.255.255.0";
  //SendData("AT+CIPSTA_CUR=\""+STA_ip+"\",\""+STA_gateway+"\",\""+STA_netmask+"\"",2000);
  SendData("AT+CWJAP_CUR=\""+WIFI_SSID+"\",\""+WIFI_PWD+"\"",5000);   
}

void SendData(String data,int TimeLimit)
{
  mySerial.println(data);
  mySerial.flush();
  delay(10);
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
  SendData(Response,5000);
  SendData("AT+CIPCLOSE="+CID,2000);
}

String WaitReply(long int TimeLimit)
{
  String ReceiveData="";
  byte ReceiveState=0;
  long int StartTime=millis();
  while( (StartTime+TimeLimit) > millis())
  {
    if (mySerial.available())
    {
      delay(4);
      while(mySerial.available())
      {
        ReceiveData=ReceiveData+String(char(mySerial.read()));
        ReceiveState=1;
      }
      if (ReceiveState==1) return ReceiveData;
    }
  } 
  return ReceiveData;
}

void getVariable()
{
  ReceiveData="";command="";cmd="";str1="";str2="";num1=-1;num2=-1;
  byte ReceiveState=0,cmdState=1,num1State=0,num2State=0,commastate=0;
  
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c=mySerial.read();
      ReceiveData=ReceiveData+String(c);
      
      if (String(c).indexOf("?")!=-1) ReceiveState=1;
      if (String(c).indexOf(" ")!=-1) ReceiveState=0;
      if (ReceiveState==1)
      {
        command=command+String(c);

        if ((String(c).indexOf("=")!=-1)&&(ReceiveState==1)) cmdState=0;
        if ((cmdState==1)&&(String(c).indexOf("?")==-1)) cmd=cmd+String(c);

        if ((String(c).indexOf("=")!=-1)&&(ReceiveState==1)&&(num2State==0)) num1State=1;
        if (((String(c).indexOf(",")!=-1)||(String(c).indexOf(" ")!=-1))&&(ReceiveState==1)) num1State=0;
        if ((num1State==1)&&(String(c).indexOf("=")==-1))
        {
          if (ReceiveData.indexOf("?&")!=-1)
            str1=str1+String(c);
          else
          {
            if (num1==-1) 
              num1=c-'0'; 
            else
              num1=num1*10+(c-'0'); 
          }
        }
        
        if ((String(c).indexOf(",")!=-1)&&(ReceiveState==1)) num2State=1;
        if ((String(c).indexOf(" ")!=-1)&&(ReceiveState==1)) num2State=0;
        if ((num2State==1)&&(String(c).indexOf(",")==-1))
        {
          if ((ReceiveData.indexOf("?&")!=-1)||(ReceiveData.indexOf("?+")!=-1))
            str2=str2+String(c);
          else
          {          
            if (num2==-1) 
              num2=c-'0'; 
            else
              num2=num2*10+(c-'0'); 
          }
        }
        else if ((num2State==1)&&(String(c).indexOf(",")!=-1)&&(commastate==1)&&((ReceiveData.indexOf("?&")!=-1)||(ReceiveData.indexOf("?+")!=-1)))
          str2=str2+String(c); 
        else if (num2State==1)
          commastate=1;
      }
    }  
    Serial.println(ReceiveData);
    
    if (ReceiveData.indexOf("WIFI GOT IP")!=-1)
    { 
        long int StartTime=millis();
        String ok="";
        while( (StartTime+10000) > millis())
        {
            while(mySerial.available())
            {
                ok=ok+String(char(mySerial.read()));
            }
            if (ok.indexOf("OK")!=-1) break;
        } 

        APIP="";STAIP="";
        int apreadstate=0,stareadstate=0,j=0,k=0;
        mySerial.println("AT+CIFSR");
        mySerial.flush();
        delay(6);

        while(mySerial.available())
        {
          char c=mySerial.read();
          String t=String(c);
          
          if (t.indexOf("\"")!=-1) j++;
          if (j==1) 
            apreadstate=1;
          else if (j==2)
            apreadstate=0;
          if ((apreadstate==1)&&(t.indexOf("\"")==-1)) APIP=APIP+t;
          
          if (t.indexOf("\"")!=-1) k++;
          if (k==5) 
            stareadstate=1;
          else if (k==6)
            stareadstate=0;
          if ((stareadstate==1)&&(t.indexOf("\"")==-1)) STAIP=STAIP+t;
        } 

        Serial.println("APIP: "+APIP+"\nSTAIP: "+STAIP);
      
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
}
