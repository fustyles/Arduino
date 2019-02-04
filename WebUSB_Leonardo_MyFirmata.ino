#include <WebUSB.h>
WebUSB WebUSBSerial(1 /* https:// */, "github.com/fustyles/webduino/tree/master/myBlockly");
#define Serial WebUSBSerial

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

String ReceiveData="", Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
boolean debug = true;

void setup()
{
  while (!Serial) {;}
  Serial.begin(9600);
  SendData("Sketch begins.");
}

void loop() 
{
  getCommand();

  if (ReceiveData.indexOf("?")==0)
  {
  if (cmd=="yourcmd")
    {
      //you can do anything
      //if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
    } 
  else if (cmd=="inputpullup")
    {
      pinMode(P1.toInt(), INPUT_PULLUP);
      if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
    }  
  else if (cmd=="pinmode")
    {
      pinMode(P1.toInt(), P2.toInt());
      if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
    }        
  else if (cmd=="digitalwrite")
    {
      pinMode(P1.toInt(), OUTPUT);
      digitalWrite(P1.toInt(),P2.toInt());
      if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
    }   
  else if (cmd=="digitalread")
    {
      SendData("[{\"data\":\""+String(digitalRead(P1.toInt()))+"\"}]");
    }    
  else if (cmd=="analogwrite")
    {
      pinMode(P1.toInt(), OUTPUT);
      analogWrite(P1.toInt(),P2.toInt());
      if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
    }       
  else if (cmd=="analogread")
    {
      SendData("[{\"data\":\""+String(analogRead(P1.toInt()))+"\"}]");
    } 
   else if (cmd=="i2cLcd") {
    P1.toLowerCase();
    if (P1=="0x27") 
      P1="39";
    else if (P1=="0x3f") 
      P1="63";
    LiquidCrystal_I2C lcd(P1.toInt(),16,2);
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(P4);
    lcd.setCursor(0,1);
    lcd.print(P5);
    if (debug == true) SendData("[{\"data\":\""+P4+"\"},{\"data\":\""+P5+"\"}]");
  }
  else 
    {
      SendData("[{\"data\":\"Command is not defined\"}]");
    }  
  }
}

void SendData(String data)
{
  Serial.println(data);
  Serial.flush();
}

void getCommand()
{
  ReceiveData="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  byte ReceiveState=0,cmdState=1,PState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
  if (Serial.available())
  {
    while (Serial.available())
    {
      char c=Serial.read();
      ReceiveData=ReceiveData+String(c);
      
      if (c=='?') ReceiveState=1;
      if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
      
      if (ReceiveState==1)
      {
        Command=Command+String(c);
        
        if (c=='=') cmdState=0;
        if (c==';') PState++;

        if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
        if ((cmdState==0)&&(PState==1)&&((c!='=')||(equalstate==1))) P1=P1+String(c);
        if ((cmdState==0)&&(PState==2)&&(c!=';')) P2=P2+String(c);
        if ((cmdState==0)&&(PState==3)&&(c!=';')) P3=P3+String(c);
        if ((cmdState==0)&&(PState==4)&&(c!=';')) P4=P4+String(c);
        if ((cmdState==0)&&(PState==5)&&(c!=';')) P5=P5+String(c);
        if ((cmdState==0)&&(PState==6)&&(c!=';')) P6=P6+String(c);
        if ((cmdState==0)&&(PState==7)&&(c!=';')) P7=P7+String(c);
        if ((cmdState==0)&&(PState==8)&&(c!=';')) P8=P8+String(c);
        if ((cmdState==0)&&(PState>=9)&&((c!=';')||(semicolonstate==1))) P9=P9+String(c);
        
        if (c=='?') questionstate=1;
        if (c=='=') equalstate=1;
        if ((PState>=9)&&(c==';')) semicolonstate=1;
      }
    }  
  }
}
