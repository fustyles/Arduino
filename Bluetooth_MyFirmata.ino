/*
Bluetooth
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-2-23 22:30 
Command format : ?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9
?inputpullup=3
?pinmode=3;1
?digitalwrite=3;1
?analogwrite=3;200
?digitalread=3
?analogread=3
*/


#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="", command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";

void executecommand()
{
  Serial.println("");
  //Serial.println("command: "+command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  
  if (cmd=="yourcmd")
    {
      //you can do anything
      //SendData(cmd+"="+str1+";"+str2);
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
  else if (cmd=="car")    //   ?car=pin_L1;pin_L2;pin_R1;pin_R2;status;left_speed;right_speed
    {
      if (str5=="S")
        {
          analogWrite(str1.toInt(),0);
          analogWrite(str2.toInt(),0);
          analogWrite(str3.toInt(),0);
          analogWrite(str4.toInt(),0);
        }
      else if  (str5=="F")
        {
          analogWrite(str1.toInt(),0);
          analogWrite(str2.toInt(),str6.toInt());
          analogWrite(str3.toInt(),str7.toInt());
          analogWrite(str4.toInt(),0);          
        }
      else if  (str5=="B")
        {
          analogWrite(str1.toInt(),str6.toInt());
          analogWrite(str2.toInt(),0);
          analogWrite(str3.toInt(),0);
          analogWrite(str4.toInt(),str7.toInt());          
        }
      else if  (str5=="L")
        {
          analogWrite(str1.toInt(),0);
          analogWrite(str2.toInt(),str6.toInt());
          analogWrite(str3.toInt(),0);
          analogWrite(str4.toInt(),str7.toInt());  
        }
      else if  (str5=="R")
        {
          analogWrite(str1.toInt(),str6.toInt());
          analogWrite(str2.toInt(),0);
          analogWrite(str3.toInt(),str7.toInt());
          analogWrite(str4.toInt(),0);    
        }
    }    
  else 
    {
      SendData("command is not defined");
    }   
}

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);   // Check uart baud rate of bluetooth device.
}

void loop() 
{
  getVariable();

  if (ReceiveData.indexOf("?")==0)
  {
    executecommand();
  }
}

void SendData(String data)
{
  mySerial.print(data);
}

void getVariable()
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
    }  
    Serial.println(ReceiveData);
  }
}
