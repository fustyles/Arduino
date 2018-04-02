#include <Servo.h>
Servo myservo;

int angle=90;
int degree=5;

int motorPin1=5;
int motorPin2=6;
int speedButton = 7;
int rotateButton = 8;

int pressCount=0;
int rotateState=-1;

void setup() {
  pinMode(speedButton, INPUT);
  pinMode(rotateButton, INPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);    
  
  myservo.attach(9);
  myservo.write(angle);
  
  Serial.begin(9600);
}

void loop() {
  if (digitalRead(speedButton)==1)
  {
    pressCount++;
    if (pressCount%4==0) 
    {
      analogWrite(motorPin1,0);
      analogWrite(motorPin2,0);
      pressCount=0;
    }
    else if (pressCount%4==1) 
    {
      analogWrite(motorPin1,100);
      analogWrite(motorPin2,0);
    }
    else if (pressCount%4==2) 
    {
      analogWrite(motorPin1,120);
      analogWrite(motorPin2,0);
    }
    else if (pressCount%4==3) 
    {
      analogWrite(motorPin1,140);
      analogWrite(motorPin2,0);
    }
    delay(200);
  }
  
  if (digitalRead(rotateButton)==1)
  {
    rotateState *= (-1);
    delay(200);
  }
  
  if (rotateState==1)
  {
    angle+=degree;
    if ((angle<5)||(angle>175))
    {
      degree=degree*(-1);
      angle=angle+degree*2;
    }
    myservo.write(angle);
    delay(200);
  }
}

