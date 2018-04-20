
/*
Arduino Uno(Uart) + Bluetooth

Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-4-20 09:00 

Command Format : 
?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

?inputpullup=pin
?pinmode=pin;value
?digitalwrite=pin;value
?analogwrite=pin;value
?digitalread=pin
?analogread=pin
?mousemove=xVal;yPos;wheel
?mouseclickleft 
?mouseclickright
?mouseclickmiddle 
?mousepressleft 
?mousepressright
?mousepressmiddle
?mouserelease   
?keyboardpress=keycode1;keycode2;keycode3
?keyboardprint=str1
?keyboardprintln=str1
?keyboardwrite=keycode
*/


#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="", command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";

#include <Keyboard.h>
#include <Mouse.h>

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
      pinMode(str1.toInt(), OUTPUT);
      digitalWrite(str1.toInt(),str2.toInt());
      SendData(command);
    }   
  else if (cmd=="digitalread")
    {
      SendData(String(digitalRead(str1.toInt())));
    }    
  else if (cmd=="analogwrite")
    {
      pinMode(str1.toInt(), OUTPUT);
      analogWrite(str1.toInt(),str2.toInt());
      SendData(command);
    }       
  else if (cmd=="analogread")
    {
      SendData(String(analogRead(str1.toInt())));
    }  
  else if (cmd=="mousemove")
    {
      Mouse.move(str1.toInt(), str2.toInt(), str3.toInt());
      SendData(command);
    }  
  else if (cmd=="mouseclickleft")
    {
      Mouse.click(MOUSE_LEFT);
      SendData(command);
    } 
  else if (cmd=="mouseclickright")
    {
      Mouse.click(MOUSE_RIGHT);
      SendData(command);
    } 
  else if (cmd=="mouseclickmiddle")
    {
      Mouse.click(MOUSE_MIDDLE);
      SendData(command);
    }     
  else if (cmd=="mousepressleft")
    {
      Mouse.press(MOUSE_LEFT);
      SendData(command);
    } 
  else if (cmd=="mousepressright")
    {
      Mouse.press(MOUSE_RIGHT);
      SendData(command);
    } 
  else if (cmd=="mousepressmiddle")
    {
      Mouse.press(MOUSE_MIDDLE);
      SendData(command);
    } 
  else if (cmd=="mouserelease")
    {
      Mouse.release();
      SendData(command);
    }     
  else if (cmd=="keyboardpress")
    {
      if (str1!="") Keyboard.press(char(str1.toInt()));
      if (str2!="") Keyboard.press(char(str2.toInt()));
      if (str3!="") Keyboard.press(char(str3.toInt()));
      delay(100);
      Keyboard.releaseAll();
      SendData(command);
    }  
  else if (cmd=="keyboardprint")
    {
      Keyboard.print(str1);
      SendData(command);
    }  
  else if (cmd=="keyboardprintln")
    {
      Keyboard.println(str1);
      SendData(command);
    }  
  else if (cmd=="keyboardwrite")
    {
      Keyboard.write(char(str1.toInt()));
      SendData(command);
    }      
  else 
    {
      SendData("command is not defined");
    }   
}

void setup()
{
  Serial.begin(9600);
  mySerial.begin(9600);   
  mySerial.setTimeout(10);
}

void loop() 
{
  getCommand();

  if (ReceiveData.indexOf("?")==0)
  {
    executecommand();
  }
}

void SendData(String data)
{
  mySerial.print(data);
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
    }  
    Serial.println(ReceiveData);
  }
}
