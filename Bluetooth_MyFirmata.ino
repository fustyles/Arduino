/*
Bluetooth
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-2-20 00:00 
Command format :  ?cmd = str1;str2;str3
?inputpullup=3
?pinmode=3;1
?digitalwrite=3;1
?analogwrite=3;200
?digitalread=3
?analogread=3
*/


#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="", command="",cmd="",str1="",str2="",str3="";

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);    //Check your bluetooth baud rate
}

void loop() 
{
  getVariable();

  if (ReceiveData.indexOf("?")==0)
  {
    Serial.println("");
    Serial.println("command: "+command);
    Serial.println("cmd: "+cmd);
    Serial.println("str1= "+str1+" ,str2= "+str2+" ,str3= "+str3);
    
    if (cmd=="yourcmd")
      {
        //you can do anything
        //SendData(cmd+"="+str1+";"+str2+";"+str3);
      } 
    else if (cmd=="inputpullup")
      {
        pinMode(str1.toInt(), INPUT_PULLUP);
        SendData(command);
      }  
    else if (cmd=="pinmode")
      {
        pinMode(str1.toInt(), str2.toInt());
        SendData(command);
      }        
    else if (cmd=="digitalwrite")
      {
        digitalWrite(str1.toInt(),str2.toInt());
        SendData(command);
      }   
    else if (cmd=="digitalread")
      {
        SendData(String(digitalRead(str1.toInt())));
      }    
    else if (cmd=="analogwrite")
      {
        analogWrite(str1.toInt(),str2.toInt());
        SendData(command);
      }       
    else if (cmd=="analogread")
      {
        SendData(String(analogRead(str1.toInt())));
      }      
    else 
      {
        SendData("command is not defined");
      }  
  }
}

void SendData(String data)
{
  mySerial.print(data);
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
      if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
      
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
    }  
    Serial.println(ReceiveData);
  }
}
