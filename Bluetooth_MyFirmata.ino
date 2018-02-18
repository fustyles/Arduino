/*
Bluetooth
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-2-19 01:30 

Command format :  
?cmd = str1 ,str2   

?inputpullup=3
?pinmode=3,1
?digitalwrite=3,1
?analogwrite=3,200
?digitalread=3
?analogread=3
*/


#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="", command="",cmd="",str1="",str2="";

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
    Serial.println("str1= "+String(str1)+" ,str2= "+String(str2));
    
    if (cmd=="yourcmd")
      {
        //you can do anything
        //SendData(cmd+"="+str1+","+str2);
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
        SendData("Command is not defined");
      }  
  }
}

void SendData(String data)
{
  mySerial.print(data);
}

void getVariable()
{
  ReceiveData="";command="";cmd="";str1="";str2="";
  byte ReceiveState=0,cmdState=1,num1State=0,num2State=0,commastate=0,equalstate=0;
  
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
            str1=str1+String(c);
        
        if ((String(c).indexOf(",")!=-1)&&(ReceiveState==1)) num2State=1;
        if ((String(c).indexOf(" ")!=-1)&&(ReceiveState==1)) num2State=0;
        if ((num2State==1)&&(String(c).indexOf(",")==-1))
            str2=str2+String(c);
        else if ((num1State==1)&&(c=='=')&&(equalstate==1))
          str1=str1+String(c); 
        else if ((num2State==1)&&(c==',')&&(commastate==1))
          str2=str2+String(c); 
          
        if (num1State==1) equalstate=1;
        if (num2State==1) commastate=1;
      }
    }  
    Serial.println(ReceiveData);
  }
}
