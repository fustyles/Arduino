/*
Arduino Leonardo + Easy Module Shield V1 of Ywbot
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-3-2 18:30
https://www.facebook.com/francefu

https://webusb.github.io/arduino/
The WebUSB library Provides all the extra low-level USB code necessary for WebUSB support except for one thing: 
Your device must be upgraded from USB 2.0 to USB 2.1. 
To do this go into the SDK installation directory and open hardware/arduino/avr/cores/arduino/USBCore.h. 
Then find the line #define USB_VERSION 0x200 and change 0x200 to 0x210. That’s it!

Library
https://github.com/webusb/arduino
https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/
https://github.com/adafruit/Adafruit_Sensor
https://github.com/adafruit/DHT-sensor-library
https://github.com/z3t0/Arduino-IRremote

Command Format
?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

?inputpullup=pin
?pinMode=pin;value
?digitalwrite=pin;value
?digitalread=pin
?analogwrite=pin;value   
?analogread=pin
?i2cLcd=address;text1;text2
?SW1    
?SW2
?DHT11
?Buzzer=frequency;delay 
?IRReceiver
?RGBLED=RedValue;GreenValue;BlueValue
?LED1D=value  
?LED2D=value
?LED2A=value
?Rotation
?Light
?LM35
*/

#include <WebUSB.h>
WebUSB WebUSBSerial(1 /* https:// */, "fustyles.github.io/webduino/myBlockly/");
#define Serial WebUSBSerial

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include<DHT.h>
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#include <IRremote.h>
int RECV_PIN = 6;
IRrecv irrecv(RECV_PIN);
decode_results results;
String ircode = "";

String ReceiveData="", Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
boolean debug = true;

void ExecuteCommand()
{
  if (cmd=="yourcmd") {
    //you can do anything
    //if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  } 
  else if (cmd=="inputpullup") {
    pinMode(P1.toInt(), INPUT_PULLUP);
    if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  }  
  else if (cmd=="pinMode") {
    pinMode(P1.toInt(), P2.toInt());
    if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  }        
  else if (cmd=="digitalwrite") {
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(),P2.toInt());
    if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  }   
  else if (cmd=="digitalread") {
    pinMode(P1.toInt(), INPUT_PULLUP);    
    SendData("[{\"data\":\""+String(digitalRead(P1.toInt()))+"\"}]");
  }    
  else if (cmd=="analogwrite") {
    pinMode(P1.toInt(), OUTPUT);
    analogWrite(P1.toInt(),P2.toInt());
    if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  }       
  else if (cmd=="analogread") {
    pinMode(P1.toInt(), INPUT_PULLUP);    
    SendData("[{\"data\":\""+String(analogRead(P1.toInt()))+"\"}]");
  } 
  // Library: https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/
  // ?i2cLcd=address;text1;text2   
  else if (cmd=="i2cLcd") {
    P1.toLowerCase();
    //You must convert hex value(P1) to decimal value.
    if (P1=="0x27") 
      P1="39";
    else if (P1=="0x3f") 
      P1="63";
    LiquidCrystal_I2C lcd(P1.toInt(), 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(P2);
    lcd.setCursor(0,1);
    lcd.print(P3);
    if (debug == true) SendData("[{\"data\":\""+P2+"\"},{\"data\":\""+P3+"\"}]");
  }
  // Library: https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
  // ?i2cLcd=address;pinSDA;pinSCL;text1;text2
  /*  
  else if (cmd=="i2cLcd") {
    P1.toLowerCase();
    //You must convert hex value(P1) to decimal value.
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
  */  
  else if (cmd=="SW1") {
    pinMode(2, INPUT_PULLUP);    
    SendData("[{\"data\":\""+String(digitalRead(2))+"\"}]");
  }     
  else if (cmd=="SW2") {
    pinMode(3, INPUT_PULLUP);    
    SendData("[{\"data\":\""+String(digitalRead(3))+"\"}]");
  } 
  else if (cmd=="DHT11") {
    float h = dht.readHumidity();
    float t = dht.readTemperature();  
    SendData("[{\"data\":\""+String(t)+"\"},{\"data\":\""+String(h)+"\"}]");
  } 
  else if (cmd=="Buzzer") {
    String f="",d="",split=",";
    int s1=0;
    P1+=",";
    P2+=",";
    for (int i=0;i<P1.length();i++) {
      if (P1[i]==split[0]) {
        f=P1.substring(s1,i);
        s1=i+1;
        for (int j=0;j<P2.length();j++) {
          if (P2[j]==split[0]) {
            d=P2.substring(0,j);
            tone(5,f.toInt(),d.toInt());
            delay(d.toInt());
            P2=P2.substring(j+1);
            break;
          }
        }
      }
    }
    if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  } 
  else if (cmd=="IRReceiver") {
    String result = ircode;
    ircode = "";
    SendData("[{\"data\":\""+String(result)+"\"}]");
  } 
  else if (cmd=="RGBLED") {
    pinMode(9, OUTPUT); 
    pinMode(10, OUTPUT); 
    pinMode(11, OUTPUT);  
    analogWrite(9,P1.toInt());
    analogWrite(10,P2.toInt());
    analogWrite(11,P3.toInt());
    if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  } 
  else if (cmd=="LED1D") {
    pinMode(12, OUTPUT); 
    digitalWrite(12,P1.toInt());   
    if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  }    
  else if (cmd=="LED2D") {
    pinMode(13, OUTPUT); 
    digitalWrite(13,P1.toInt());   
    if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  } 
  else if (cmd=="LED2A") {
    pinMode(13, OUTPUT); 
    analogWrite(13,P1.toInt());   
    if (debug == true) SendData("[{\"data\":\""+Command+"\"}]");
  }  
  else if (cmd=="Rotation") { 
    SendData("[{\"data\":\""+String(analogRead(A0))+"\"}]");
  } 
  else if (cmd=="Light") {
    SendData("[{\"data\":\""+String(analogRead(A1))+"\"}]");
  } 
  else if (cmd=="LM35") { 
    SendData("[{\"data\":\""+String((125*analogRead(A2))>>8)+"\"}]");
  } 
  else {
    SendData("[{\"data\":\"Command is not defined\"}]");
  }  
}

void setup()
{
  while (!Serial) {;}
  Serial.begin(9600);
  SendData("Device is Connected.");
  dht.begin();
  irrecv.enableIRIn();
}

void loop() 
{
  if (irrecv.decode(&results)) {
    if (results.value!=4294967295) ircode = String(results.value);
    irrecv.resume();
  }  

  getCommand();
  if (ReceiveData.indexOf("?")==0) ExecuteCommand();
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
