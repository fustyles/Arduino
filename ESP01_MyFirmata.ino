/* 
Arduino Uno(Uart) + ESP8266 ESP-01 (1MB Flash, V2.0_AT_Firmware)

Author : ChungYi Fu (Taiwan)  2018-04-06 20:00

Update AT Firmware(V2.0_AT_Firmware)
https://www.youtube.com/watch?v=QVhWVu8NnZc
http://www.electrodragon.com/w/File:V2.0_AT_Firmware(ESP).zip

Command Format :  
http://APIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9
http://STAIP/?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

Default APIP： 192.168.4.1
http://192.168.4.1/?ip
http://192.168.4.1/?mac
http://192.168.4.1/?restart
http://192.168.4.1/?resetwifi=ssid;password
http://192.168.4.1/?at=AT+Command
http://192.168.4.1/?tcp=domain;port;request
http://192.168.4.1/?inputpullup=pin
http://192.168.4.1/?pinmode=pin;value
http://192.168.4.1/?digitalwrite=pin;value
http://192.168.4.1/?analogwrite=pin;value
http://192.168.4.1/?digitalread=pin
http://192.168.4.1/?analogread=pin
http://192.168.4.1/?car=pinL1;pinL2;pinR1;pinR2;L_speed;R_speed;Delay;state

STAIP：
Query： http://192.168.4.1/?ip
Link：http://192.168.4.1/?resetwifi=ssid;password

Control Page
https://github.com/fustyles/webduino/blob/master/ESP8266_MyFirmata.html
*/

// Check your Wi-Fi Router's Settings
String WIFI_SSID = "";   //your network SSID
String WIFI_PWD  = "";    //your network password

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="", command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";
String APIP="",APMAC="",STAIP="",STAMAC="",CID="";

void executecommand()
{
  Serial.println("");
  //Serial.println("command: "+command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  
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
      Feedback(CID,"<html>APIP: "+APIP+"<br>STAIP: "+STAIP+"</html>",3);
    }
  else if (cmd=="mac")
    {
      Feedback(CID,"<html>APMAC: "+APMAC+"<br>STAMAC: "+STAMAC+"</html>",3);
    }    
  else if (cmd=="resetwifi")
    {
      Feedback(CID,"<html>"+str1+","+str2+"</html>",3);
      delay(3000);
      SendData("AT+CWQAP",2000);
      SendData("AT+CWJAP_CUR=\""+str1+"\",\""+str2+"\"",5000);
    }
  else if (cmd=="restart")
    {
      Feedback(CID,"<html>"+command+"</html>",3);
      delay(3000);
      SendData("AT+RST",2000);
      delay(2000);
      setup();
      initial();
    }    
  else if (cmd=="at")      //  ?cmd=str1 -> ?at=AT+RST
    {
      Feedback(CID,"<html>"+WaitReply(3000)+"</html>",3);
      delay(1000);
      mySerial.println(str1);
      mySerial.flush();
    }    
  else if (cmd=="tcp")      //  ?tcp=domain;port;request
    {
      Feedback(CID,"<html>"+command+"</html>",3);
      delay(1000);               
      String Domain=str1;
      String request = "GET /"+str3+" HTTP/1.1\r\nHost: "+Domain+"\r\n\r\n";
      
      SendData("AT+CIPSTART=4,\"TCP\",\""+Domain+"\","+str2, 4000);
      SendData("AT+CIPSEND=4," + String(request.length()+2), 4000);
      SendData(request, 2000);
      delay(1);
      SendData("AT+CIPCLOSE=4",2000);
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
      pinMode(str1.toInt(), OUTPUT);
      digitalWrite(str1.toInt(),str2.toInt());
      Feedback(CID,"<html>"+command+"</html>",3);
    }   
  else if (cmd=="digitalread")
    {
      Feedback(CID,"<html>"+String(digitalRead(str1.toInt()))+"</html>",3);
    }    
  else if (cmd=="analogwrite")
    {
      pinMode(str1.toInt(), OUTPUT);
      analogWrite(str1.toInt(),str2.toInt());
      Feedback(CID,"<html>"+command+"</html>",3);
    }       
  else if (cmd=="analogread")
    {
      Feedback(CID,"<html>"+String(analogRead(str1.toInt()))+"</html>",3);
    }  
  else if (cmd=="car")    // ?car=pinL1;pinL2;pinR1;pinR2;L_speed;R_speed;Delay;state
    {
      pinMode(str1.toInt(), OUTPUT);
      pinMode(str2.toInt(), OUTPUT);
      pinMode(str3.toInt(), OUTPUT);
      pinMode(str4.toInt(), OUTPUT);
      digitalWrite(str1.toInt(), 0);
      digitalWrite(str2.toInt(), 0);
      digitalWrite(str3.toInt(), 0);
      digitalWrite(str4.toInt(), 0);
      delay(10);    
    
      if (str8=="S")
      {
        analogWrite(str1.toInt(),0);
        analogWrite(str2.toInt(),0);
        analogWrite(str3.toInt(),0);
        analogWrite(str4.toInt(),0);
      }
      else if  (str8=="F")
      {
        analogWrite(str1.toInt(),str5.toInt());
        analogWrite(str2.toInt(),0);
        analogWrite(str3.toInt(),0);
        analogWrite(str4.toInt(),str6.toInt());       
        if ((str7!="")&&(str7!="0"))
        {
          delay(str7.toInt());
          analogWrite(str1.toInt(),0);
          analogWrite(str2.toInt(),0);
          analogWrite(str3.toInt(),0);
          analogWrite(str4.toInt(),0);          
        }     
      }
      else if  (str8=="B")
      {
        analogWrite(str1.toInt(),0);
        analogWrite(str2.toInt(),str5.toInt());
        analogWrite(str3.toInt(),str6.toInt());
        analogWrite(str4.toInt(),0);  
        if ((str7!="")&&(str7!="0"))
        {
          delay(str7.toInt());
          analogWrite(str1.toInt(),0);
          analogWrite(str2.toInt(),0);
          analogWrite(str3.toInt(),0);
          analogWrite(str4.toInt(),0);          
        }     
      }
      else if  (str8=="L")
      {
        analogWrite(str1.toInt(),0);
        analogWrite(str2.toInt(),str5.toInt());
        analogWrite(str3.toInt(),0);
        analogWrite(str4.toInt(),str6.toInt());         
        if ((str7!="")&&(str7!="0"))
        {
          delay(str7.toInt());
          analogWrite(str1.toInt(),0);
          analogWrite(str2.toInt(),0);
          analogWrite(str3.toInt(),0);
          analogWrite(str4.toInt(),0);          
        }
      }
      else if  (str8=="R")
      {
        analogWrite(str1.toInt(),str5.toInt());
        analogWrite(str2.toInt(),0);
        analogWrite(str3.toInt(),str6.toInt());
        analogWrite(str4.toInt(),0);
        if ((str7!="")&&(str7!="0"))
        {
          delay(str7.toInt());
          analogWrite(str1.toInt(),0);
          analogWrite(str2.toInt(),0);
          analogWrite(str3.toInt(),0);
          analogWrite(str4.toInt(),0);          
        }        
      }
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
  mySerial.setTimeout(10);
  
  initial();
}

void initial()
{
  SendData("AT+CWMODE_CUR=3",2000);
  SendData("AT+CIPMUX=1",2000);
  SendData("AT+CIPSERVER=1,80",2000);   //port=80
  SendData("AT+CIPSTO=5",2000);  //timeout= 5 seconds
  SendData("AT+CIPAP_CUR=\"192.168.4.1\"",2000);  //APIP: 192.168.4.1
  //SendData("AT+CWSAP_CUR=\""+AP_id+"\",\""+AP_pwd+"\",3,4",2000);
  //String STA_ip="192.168.0.100";
  //String STA_gateway="192.168.0.1";
  //String STA_netmask="255.255.255.0";
  //SendData("AT+CIPSTA_CUR=\""+STA_ip+"\",\""+STA_gateway+"\",\""+STA_netmask+"\"",2000);
  if (WIFI_SSID!="") 
    SendData("AT+CWJAP_CUR=\""+WIFI_SSID+"\",\""+WIFI_PWD+"\"",5000);  
  else
    Serial.print("Please check your network SSID and password");  
}

void loop() 
{
  getCommand();
  
  if ((ReceiveData.indexOf("/?")!=-1)&&(ReceiveData.indexOf(" HTTP")!=-1))
  {
    CID=String(ReceiveData.charAt(ReceiveData.indexOf("+IPD,")+5));
    executecommand();
  }
  else if ((ReceiveData.indexOf("/?")!=-1)&&(ReceiveData.indexOf(" HTTP")==-1))
  {
    if(ReceiveData.indexOf("+IPD,")!=-1)
    {
      CID=String(ReceiveData.charAt(ReceiveData.indexOf("+IPD,")+5));
      Feedback(CID,"<html>It can't work！Check the length of command!</html>",3);
    }
  }
  else if ((ReceiveData.indexOf("/?")==-1)&&(ReceiveData.indexOf(" HTTP")!=-1))
  {
    CID=String(ReceiveData.charAt(ReceiveData.indexOf("+IPD,")+5));
    Feedback(CID,"<html>Hello World</html>",3);
  }
  
  //if (SensorValue>LimitValue)
  //{
  //  cmd="yourcmd";
  //  str1="yourstr1";
  //  str2="yourstr2";
  //  str3="yourstr3";
  //  ...
  //  str9="yourstr9";
  //  ExecuteCommand();
  //  delay(10000);
  //}  
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
  //If the length of response data is too long, it can't work.
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
  delay(1);
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

void getCommand()
{
  ReceiveData="";command="";cmd="";str1="";str2="";str3="";str4="";str5="";str6="";str7="";str8="";str9="";
  byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c=mySerial.read();
      ReceiveData=ReceiveData+String(c);
      
      if (c=='?') ReceiveState=1;
      if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
      
      if (ReceiveState==1)
      {
        command=command+String(c);
        
        if (c=='=') cmdState=0;
        if (c==';') strState++;

        if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
        if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) str1=str1+String(c);
        if ((cmdState==0)&&(strState==2)&&(c!=';')) str2=str2+String(c);
        if ((cmdState==0)&&(strState==3)&&(c!=';')) str3=str3+String(c);
        if ((cmdState==0)&&(strState==4)&&(c!=';')) str4=str4+String(c);
        if ((cmdState==0)&&(strState==5)&&(c!=';')) str5=str5+String(c);
        if ((cmdState==0)&&(strState==6)&&(c!=';')) str6=str6+String(c);
        if ((cmdState==0)&&(strState==7)&&(c!=';')) str7=str7+String(c);
        if ((cmdState==0)&&(strState==8)&&(c!=';')) str8=str8+String(c);
        if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) str9=str9+String(c);
        
        if (c=='?') questionstate=1;
        if (c=='=') equalstate=1;
        if ((strState>=9)&&(c==';')) semicolonstate=1;
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
      delay(10);

      APIP="";APMAC="";STAIP="";STAMAC="";
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
        if ((apipreadstate==1)&&(t!="\"")) APIP=APIP+t;

        if (j==3) 
          apmacreadstate=1;
        else if (j==4)
          apmacreadstate=0;
        if ((apmacreadstate==1)&&(t!="\"")) APMAC=APMAC+t;
        
        if (j==5) 
          staipreadstate=1;
        else if (j==6)
          staipreadstate=0;
        if ((staipreadstate==1)&&(t!="\"")) STAIP=STAIP+t;

        if (j==7) 
          stamacreadstate=1;
        else if (j==8)
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
      
      Serial.println("APIP: "+APIP+"\nAPMAC: "+APMAC+"\nSTAIP: "+STAIP+"\nSTAMAC: "+STAMAC);
    }
  }
}
