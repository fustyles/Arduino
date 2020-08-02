/*
Pixel:Bit (UNO)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-8-2 16:00
https://www.facebook.com/francefu

Serial baud rate : 9600

?car=F       //前進
?car=B       //後退
?car=L       //左轉
?car=R       //右轉
?car=FL      //左前進
?car=FR      //右前進
?car=BL      //左後退
?car=BR      //右後退
?speedL=255  //左輪轉速
?speedR=255  //右輪轉速
*/

//Webbit settings
byte pingL1 = 9;
byte pingL2 = 10;
byte pingR1 = 6;
byte pingR2 = 3;
byte speedL = 255;
byte speedR = 255;
float speedRatio = 0.8;

String ReceiveData="", command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";  

void executeCommand() {
  if (cmd=="yourcmd") {
      //you can do anything
  }  
  else if (cmd=="car") {   // ?car=state
      pinMode(pingL1, OUTPUT);
      pinMode(pingL2, OUTPUT);
      pinMode(pingR1, OUTPUT);
      pinMode(pingR2, OUTPUT);
      digitalWrite(pingL1, 0);
      digitalWrite(pingL2, 0);
      digitalWrite(pingR1, 0);
      digitalWrite(pingR2, 0);
    
      if (P1=="S") {
        analogWrite(pingL1,0);
        analogWrite(pingL2,0);
        analogWrite(pingR1,0);
        analogWrite(pingR2,0);
      }
      else if  (P1=="F") {
        analogWrite(pingL1,speedL);
        analogWrite(pingL2,0);
        analogWrite(pingR1,0);
        analogWrite(pingR2,speedR);          
      }
      else if  (P1=="B") {
        analogWrite(pingL1,0);
        analogWrite(pingL2,speedL);
        analogWrite(pingR1,speedR);
        analogWrite(pingR2,0);     
      }
      else if  (P1=="L") {
        analogWrite(pingL1,0);
        analogWrite(pingL2,speedL);
        analogWrite(pingR1,0);
        analogWrite(pingR2,speedR);       
      }
      else if  (P1=="R") {
        analogWrite(pingL1,speedL);
        analogWrite(pingL2,0);
        analogWrite(pingR1,speedR);
        analogWrite(pingR2,0);       
      }

      else if  (P1=="FL") {
        analogWrite(pingL1,speedL*speedRatio);
        analogWrite(pingL2,0);
        analogWrite(pingR1,0);
        analogWrite(pingR2,speedR);          
      }
      else if  (P1=="BL") {
        analogWrite(pingL1,0);
        analogWrite(pingL2,speedL*speedRatio);
        analogWrite(pingR1,speedR);
        analogWrite(pingR2,0);     
      }
      else if  (P1=="FR") {
        analogWrite(pingL1,speedL);
        analogWrite(pingL2,0);
        analogWrite(pingR1,0);
        analogWrite(pingR2,speedR*speedRatio);          
      }
      else if  (P1=="BR") {
        analogWrite(pingL1,0);
        analogWrite(pingL2,speedL);
        analogWrite(pingR1,speedR*speedRatio);
        analogWrite(pingR2,0);     
      }     
    }
    else if (cmd=="speedL") {
      speedL = P1.toInt();
    }
    else if (cmd=="speedR") {
      speedR = P1.toInt();
    }
}

void setup()
{
  Serial.begin(9600);
  Serial.setTimeout(10);
}

void loop() 
{
  getCommand();
  if (ReceiveData.indexOf("?")==0) {
    executeCommand();
  }
}

void getCommand()
{
  ReceiveData="";command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
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
        if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) P1=P1+String(c);
        if ((cmdState==0)&&(strState==2)&&(c!=';')) P2=P2+String(c);
        if ((cmdState==0)&&(strState==3)&&(c!=';')) P3=P3+String(c);
        if ((cmdState==0)&&(strState==4)&&(c!=';')) P4=P4+String(c);
        if ((cmdState==0)&&(strState==5)&&(c!=';')) P5=P5+String(c);
        if ((cmdState==0)&&(strState==6)&&(c!=';')) P6=P6+String(c);
        if ((cmdState==0)&&(strState==7)&&(c!=';')) P7=P7+String(c);
        if ((cmdState==0)&&(strState==8)&&(c!=';')) P8=P8+String(c);
        if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) P9=P9+String(c);
        
        if (c=='?') questionstate=1;
        if (c=='=') equalstate=1;
        if ((strState>=9)&&(c==';')) semicolonstate=1;
      }
      delay(1);
    }  
    //Serial.println(ReceiveData);
  }
}
