/*
Arduino Leonardo (keyboard,mouse) + Bluetooth, ESP...with Uart

Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-08-07 13:00 
https://www.facebook.com/francefu

Uart Command Format : 
?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

?inputpullup=pin
?pinmode=pin;value
?digitalwrite=pin;value
?analogwrite=pin;value
?digitalread=pin
?analogread=pin
?mousemove=xPos;yPos;wheel
?mouseclickleft 
?mouseclickright
?mouseclickmiddle 
?mousepressleft 
?mousepressright
?mousepressmiddle
?mouserelease   
?keyboardpress=keycode1;keycode2;keycode3;presstime
?keyboardprint=characters
?keyboardprintln=characters
?keyboardwrite=keycode

Keyboard Modifiers (keyboardpress)
https://www.arduino.cc/en/Reference/KeyboardModifiers

Remote Control for PPT

?keyboardwrite=198          "F5"
?keyboardwrite=211          "PAGE UP"
?keyboardpress=133;198;;10  "SHIFT+F5"     //Format: ?keyboardpress=keycode1;keycode2;keycode3;presstime
?keyboardwrite=214          "PAGE DOWN"
?keyboardwrite=87           "W"
?keyboardwrite=177          "ESC"
?keyboardwrite=66           "B"


Remote Control for Game 
Format: ?keyboardpress=keycode1;keycode2;keycode3;presstime

?keyboardpress=215;;;100     "KEY_RIGHT_ARROW"
?keyboardpress=216;;;100     "KEY_LEFT_ARROW"
?keyboardpress=217;;;100     "KEY_DOWN_ARROW"
?keyboardpress=218;;;200     "KEY_UP_ARROW"
?keyboardpress=215;218;;200  "KEY_RIGHT_ARROW + KEY_UP_ARROW"
?keyboardpress=216;218;;200  "KEY_LEFT_ARROW + KEY_UP_ARROW"

*/

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Bluetooth(or ESP8266) TX->D10, RX->D11 

#include <Keyboard.h>
#include <Mouse.h>

String ReceiveData="", command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";
boolean debug = false;

void executecommand()
{
  Serial.println("");
  //Serial.println("command: "+command);
  Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  
  //If you want to execute command quickly, please don't execute "SendData"!
  
  if (cmd=="yourcmd")
    {
      //you can do anything
      //if (debug == true) SendData(cmd+"="+str1+";"+str2);
    } 
  else if (cmd=="inputpullup")
    {
      pinMode(str1.toInt(), INPUT_PULLUP);
      if (debug == true) SendData(command);
    }  
  else if (cmd=="pinmode")
    {
      pinMode(str1.toInt(), str2.toInt());
      if (debug == true) SendData(command);
    }        
  else if (cmd=="digitalwrite")
    {
      pinMode(str1.toInt(), OUTPUT);
      digitalWrite(str1.toInt(),str2.toInt());
      if (debug == true) SendData(command);
    }   
  else if (cmd=="digitalread")
    {
      SendData(String(digitalRead(str1.toInt())));
    }    
  else if (cmd=="analogwrite")
    {
      pinMode(str1.toInt(), OUTPUT);
      analogWrite(str1.toInt(),str2.toInt());
      if (debug == true) SendData(command);
    }       
  else if (cmd=="analogread")
    {
      SendData(String(analogRead(str1.toInt())));
    }  
  else if (cmd=="mousemove")
    {
      Mouse.move(str1.toInt(), str2.toInt(), str3.toInt());
      if (debug == true) SendData(command);
    }  
  else if (cmd=="mouseclickleft")
    {
      Mouse.click(MOUSE_LEFT);
      if (debug == true) SendData(command);
    } 
  else if (cmd=="mouseclickright")
    {
      Mouse.click(MOUSE_RIGHT);
      if (debug == true) SendData(command);
    } 
  else if (cmd=="mouseclickmiddle")
    {
      Mouse.click(MOUSE_MIDDLE);
      if (debug == true) SendData(command);
    }     
  else if (cmd=="mousepressleft")
    {
      Mouse.press(MOUSE_LEFT);
      if (debug == true) SendData(command);
    } 
  else if (cmd=="mousepressright")
    {
      Mouse.press(MOUSE_RIGHT);
      if (debug == true) SendData(command);
    } 
  else if (cmd=="mousepressmiddle")
    {
      Mouse.press(MOUSE_MIDDLE);
      if (debug == true) SendData(command);
    } 
  else if (cmd=="mouserelease")
    {
      Mouse.release();
      if (debug == true) SendData(command);
    }     
  else if (cmd=="keyboardpress")
    {
      if (str1!="") Keyboard.press(char(str1.toInt()));
      if (str2!="") Keyboard.press(char(str2.toInt()));
      if (str3!="") Keyboard.press(char(str3.toInt()));
      delay(str4.toInt());
      Keyboard.releaseAll();
      if (debug == true) SendData(command);
    }  
  else if (cmd=="keyboardprint")
    {
      Keyboard.print(str1);
      if (debug == true) SendData(command);
    }  
  else if (cmd=="keyboardprintln")
    {
      Keyboard.println(str1);
      if (debug == true) SendData(command);
    }  
  else if (cmd=="keyboardwrite")
    {
      Keyboard.write(char(str1.toInt()));
      if (debug == true) SendData(command);
    }      
  else 
    {
      if (debug == true) SendData("command is not defined");
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
