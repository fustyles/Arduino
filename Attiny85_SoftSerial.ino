/*
  Software serial multiple serial test
 
 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.
 
 The circuit: 
 * RX is digital pin 2 (connect to TX of other device)
 * TX is digital pin 4 (connect to RX of other device)
 
 created back in the mists of time
 modified 9 Apr 2012
 by Tom Igoe
 based on Mikal Hart's example
 
 This example code is in the public domain.
 
 <SoftSerial> adapted from <SoftwareSerial> for <TinyPinChange> library which allows sharing the Pin Change Interrupt Vector.
 Single difference with <SoftwareSerial>: add #include <TinyPinChange.h>  at the top of your sketch.
 RC Navy (2012): http://p.loussouarn.free.fr
 
 */
#include <SoftSerial.h>     /* Allows Pin Change Interrupt Vector Sharing */
#include <TinyPinChange.h>  /* Ne pas oublier d'inclure la librairie <TinyPinChange> qui est utilisee par la librairie <RcSeq> */

SoftSerial mySerial(2, 4); // RX, TX

void setup() {
  mySerial.begin(19200);
}

void loop() {
  String cmd="",P1="",P2="";
  byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
  if (mySerial.available()) {
    while (mySerial.available()) {
      char c=mySerial.read();
      
      if (c=='?') ReceiveState=1;
      if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
      
      if (ReceiveState==1) {
        if (c=='=') cmdState=0;
        if (c==';') strState++;
        if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
        if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) P1=P1+String(c);
        if ((cmdState==0)&&(strState>=2)&&((c!=';')||(semicolonstate==1))) P2=P2+String(c);
        if (c=='?') questionstate=1;
        if (c=='=') equalstate=1;
        if ((strState>=2)&&(c==';')) semicolonstate=1;
      }
    }  
  }

  if (cmd=="dw") {
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(), P2.toInt());  
  }   
  else if (cmd=="aw") {
    pinMode(P1.toInt(), OUTPUT);
    analogWrite(P1.toInt(), P2.toInt());
  }
  else if (cmd=="dr") {
    pinMode(P1.toInt(), INPUT);
    mySerial.println(String(digitalRead(P1.toInt())));
  }   
  else if (cmd=="ar") {
    pinMode(P1.toInt(), INPUT);
    mySerial.println(String(analogRead(P1.toInt())));
  }  
}
