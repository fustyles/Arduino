/*
ESP32 UART

Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-1-16 22:00
https://www.facebook.com/francefu

Uart Command Format : 
?cmd=str1;str2;str3;str4;str5;str6;str7;str8;str9

?inputpullup=pin
?pinmode=pin;value
?digitalwrite=pin;value
?analogwrite=pin;value
?digitalread=pin
?touchread=pin
?analogread=pin

Web Serial
https://github.com/fustyles/webduino/blob/master/WebSerial.html
*/

String ReceiveData="", command="",cmd="",str1="",str2="",str3="",str4="",str5="",str6="",str7="",str8="",str9="";

void executecommand() {
  //Serial.println("");
  //Serial.println("command: "+command);
  //Serial.println("cmd= "+cmd+" ,str1= "+str1+" ,str2= "+str2+" ,str3= "+str3+" ,str4= "+str4+" ,str5= "+str5+" ,str6= "+str6+" ,str7= "+str7+" ,str8= "+str8+" ,str9= "+str9);
  
  if (cmd=="yourcmd") {
    //you can do anything
    //SendData(cmd+"="+str1+";"+str2);
  } 
  else if (cmd=="inputpullup") {
    pinMode(str1.toInt(), INPUT_PULLUP);
    SendData(command);
  }  
  else if (cmd=="pinmode") {
    pinMode(str1.toInt(), str2.toInt());
    SendData(command);
  }        
  else if (cmd=="digitalwrite") {
    ledcDetachPin(str1.toInt());
    pinMode(str1.toInt(), OUTPUT);
    digitalWrite(str1.toInt(),str2.toInt());
    SendData(command);
  }   
  else if (cmd=="digitalread") {
    SendData(String(digitalRead(str1.toInt())));
  }   
  else if (cmd=="touchread") {
    SendData(String(touchRead(str1.toInt())));
  }  
  else if (cmd=="analogwrite") {
    ledcAttachPin(str1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,str2.toInt());
    SendData(command);
  }       
  else if (cmd=="analogread") {
    SendData(String(analogRead(str1.toInt())));
  }  
  else {
    SendData("Command is not defined");
  }   
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  getCommand();

  if (ReceiveData.indexOf("?")==0) {
    executecommand();
  }
}

void SendData(String data) {
  Serial.println(data);
}

void getCommand() {
  ReceiveData="";command="";cmd="";str1="";str2="";str3="";str4="";str5="";str6="";str7="";str8="";str9="";
  byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

  if (Serial.available()) {
    while (Serial.available()) {
      char c=Serial.read();
      ReceiveData=ReceiveData+String(c);
      
      if (c=='?') ReceiveState=1;
      if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
      
      if (ReceiveState==1) {
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
      delay(1);
    }  
  }
}
