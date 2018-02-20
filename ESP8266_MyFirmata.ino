/* 
Arduino Uno + ESP8266 ESP-01
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-2-20 21:30
Command format :  ?cmd=str1;str2;str3
AP IP： 192.168.4.1
http://192.168.4.1/?resetwifi=id;pwd
http://192.168.4.1/?ip
http://192.168.4.1/?at=AT+Command
http://192.168.4.1/?tcp=ip,port;parameter
http://192.168.4.1/?inputpullup=3
http://192.168.4.1/?pinmode=3;1
http://192.168.4.1/?digitalwrite=3;1
http://192.168.4.1/?analogwrite=3;200
http://192.168.4.1/?digitalread=3
http://192.168.4.1/?analogread=3
STA IP：
Query： http://192.168.4.1/?ip
Link：http://192.168.4.1/?resetwifi=id;pwd
*/

// Check your Wi-Fi Router's Settings
String WIFI_SSID="yourwifi_id";
String WIFI_PWD="yourwifi_pwd";

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="", command="",cmd="",str1="",str2="",str3="";
String APIP="",STAIP="",CID="";

void executecommand()
{
  Serial.println("");
  Serial.println("command: "+command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3);
  
  if (cmd=="yourcmd")
    {
      //you can do anything
      
      //Feedback(CID,"<font color=\"red\">"+cmd+"="+str1+";"+str2+";"+str3+"</font>",0);  --> HTML
      //Feedback(CID,cmd+"="+str1+";"+str2+";"+str3,1);  --> XML
      //Feedback(CID,cmd+"="+str1+";"+str2+";"+str3,2);  --> JSON
      //Feedback(CID,"<html>"+cmd+"="+str1+";"+str2+";"+str3+"</html>",3);  --> Custom definition
    } 
  else if (cmd=="ip")
    {
      //Feedback(CID,"<font color=\"red\">APIP: "+APIP+"<br>STAIP: "+STAIP+"</font>",0);
      Feedback(CID,"<html>APIP: "+APIP+"<br>STAIP: "+STAIP+"</html>",3);
    }
  else if (cmd=="at")      //  ?cmd=str1 -> ?at=AT+RST
    {
      Feedback(CID,"<html>"+WaitReply(3000)+"</html>",3);
      delay(1000);
      mySerial.println(str1);
      mySerial.flush();
    }
  else if (cmd=="tcp")      // ?tcp=url,port;parameter
    {
      String getcommand="GET /"+str2;
      SendData("AT+CIPSTART=0,\"TCP\",\""+str1+"\"",2000);
      SendData("AT+CIPSEND=0,"+String(getcommand.length()+2),2000);
      SendData(getcommand,2000);
      delay(10);
      Feedback("0","<html>"+WaitReply(10000)+"</html>",3);
    }      
  else if (cmd=="inputpullup")
    {
      pinMode(str1.toInt(), INPUT_PULLUP);
      Feedback(CID,"<html>"+command+"</html>",3);
    }  
  else if (cmd=="pinmode")
    {
      pinMode(str1.toInt(), str2.toInt());
      Feedback(CID,"<html>"+command+"</html>",3);
    }        
  else if (cmd=="digitalwrite")
    {
      digitalWrite(str1.toInt(),str2.toInt());
      Feedback(CID,"<html>"+command+"</html>",3);
    }   
  else if (cmd=="digitalread")
    {
      Feedback(CID,"<html>"+String(digitalRead(str1.toInt()))+"</html>",3);
    }    
  else if (cmd=="analogwrite")
    {
      analogWrite(str1.toInt(),str2.toInt());
      Feedback(CID,"<html>"+command+"</html>",3);
    }       
  else if (cmd=="analogread")
    {
      Feedback(CID,"<html>"+String(analogRead(str1.toInt()))+"</html>",3);
    }    
  else if (cmd=="resetwifi")
    {
      Feedback(CID,"<html>"+str1+","+str2+"</html>",3);
      delay(3000);
      SendData("AT+CWQAP",2000);
      SendData("AT+CWJAP_CUR=\""+str1+"\",\""+str2+"\"",5000);
    }
  else 
    {
      Feedback(CID,"<html>Command is not defined</html>",3);
    }    
}

void setup()
{
  Serial.begin(9600);
  
  //You must change uart baud rate of ESP-01 to 9600.
  mySerial.begin(115200);   //Default uart baud rate -> 19200,38400,57600,74880,115200
  SendData("AT+UART_CUR=9600,8,1,0,0",2000);   //Change uart baud rate of ESP-01 to 9600
  mySerial.begin(9600);  // 9600 ,you will get more stable data.
  
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

void loop() 
{
  getVariable();
  
  if ((ReceiveData.indexOf("?")!=-1)&&(ReceiveData.indexOf(" HTTP")!=-1))
  {
    CID=String(ReceiveData.charAt(ReceiveData.indexOf("+IPD,")+5));
    executecommand();
  }
  else if ((ReceiveData.indexOf("?")!=-1)&&(ReceiveData.indexOf(" HTTP")==-1))
  {
    if(ReceiveData.indexOf("IPD,")!=-1)
    {
      CID=String(ReceiveData.charAt(ReceiveData.indexOf("+IPD,")+5));
      Feedback(CID,"<html>FAIL</html>",3);
    }
  }
  else if ((ReceiveData.indexOf("?")==-1)&&(ReceiveData.indexOf(" HTTP")!=-1))
  {
    CID=String(ReceiveData.charAt(ReceiveData.indexOf("+IPD,")+5));
    Feedback(CID,"<html>Hello World</html>",3);
  }
}

void SendData(String data,int TimeLimit)
{
  mySerial.println(data);
  mySerial.flush();
  delay(10);
  WaitReply(TimeLimit);
}

void Feedback(String CID,String Response,int datatype)
{
  if (datatype==0)
  {
    Response="<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\"></head><body>"+Response+"</body></html>";
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
      return ReceiveData;
    }
  } 
  return ReceiveData;
}

void getVariable()
{
  ReceiveData="";command="";cmd="";str1="";str2="";str3="";
  byte ReceiveState=0,cmdState=1,str1State=0,str2State=0,str3State=0,equalstate=0,semicolonstate=0;
  
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c=mySerial.read();
      ReceiveData=ReceiveData+String(c);
      
      if (c=='?') ReceiveState=1;
      if ((c==' ')||(c=='\r')) ReceiveState=0;
      
      if (ReceiveState==1)
      {
        command=command+String(c);
        
        if ((c=='=')&&(str1State==0)&&(str2State==0)&&(str3State==0))
        {
          cmdState=0;str1State=1;str2State=0;str3State=0;
        }
        else if ((c==';')&&(str1State==1)&&(str2State==0)&&(str3State==0)) 
        {
          cmdState=0;str1State=0;str2State=1;str3State=0;
        }
        else if ((c==';')&&(str1State==0)&&(str2State==1)&&(str3State==0)) 
        {
          cmdState=0;str1State=0;str2State=0;str3State=1;
        }  
        else if (c==' ')
        {
          cmdState=0;str1State=0;str2State=0;str3State=0;
        }

        if ((cmdState==1)&&(c!='?')) cmd=cmd+String(c);
        if ((str1State==1)&&((c!='=')||(equalstate==1))) str1=str1+String(c);
        if ((str2State==1)&&(c!=';')) str2=str2+String(c);
        if ((str3State==1)&&((c!=';')||(semicolonstate==1))) str3=str3+String(c);
        
        if (str1State==1) equalstate=1;
        if (str3State==1) semicolonstate=1;
      }
      
      if (ReceiveData.indexOf(" HTTP")!=-1)
      {
         while (mySerial.available())
        {
          mySerial.read();
        }
      }
    }  
    Serial.println(ReceiveData);
    
    if (ReceiveData.indexOf("WIFI GOT IP")!=-1)
    { 
      while(!mySerial.find('OK')){} 
      delay(1000);

      APIP="";STAIP="";
      int apreadstate=0,stareadstate=0,j=0;
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
        
        if (j==5) 
          stareadstate=1;
        else if (j==6)
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
