#include <WebUSB.h>
WebUSB WebUSBSerial(1 /* https:// */, "github.com/fustyles/webduino/tree/master/myBlockly");
#define Serial WebUSBSerial

String ReceiveData="", command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";
boolean debug = true;

void setup()
{
  while (!Serial) {;}
  Serial.begin(9600);
  SendData("Sketch begins.\r\n");
}

void loop() 
{
  getCommand();

  if (ReceiveData.indexOf("?")==0)
  {
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
  else if (cmd=="i2cLcd") {
    if (P1toLowerCase()=="0x27") P1="39";
    if (P1toLowerCase()=="0x3f") P1="63";
    LiquidCrystal_I2C lcd(P1.toInt(),16,2);
    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(P4);
    lcd.setCursor(0,1);
    lcd.print(P5);
    if (debug == true) SendData(P4+"; "+P5);
  }    
  else 
    {
      SendData("command is not defined");
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
  ReceiveData="";command="";cmd="";str1="";str2="";str3="";str4="";str5="";str6="";str7="";str8="";str9="";
  byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
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
  }
}
