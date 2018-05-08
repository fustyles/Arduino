/*
Arduino Uno
3V DC Motor
Motor Driver Board
SG90 Mini Servo
*/

#include <Servo.h>
Servo myservo;

int servoPin=4;        //伺服馬達腳位
int motorPin1=5;       //馬達驅動IC腳位
int motorPin2=6;       //馬達驅動IC腳位
int speedButtonPin=7;   //風速四段變速切換按鈕腳位
int rotateButtonPin=8;  //風向旋轉切換按鈕腳位

int angle=90;          //風向初始角度
int degree=5;          //單位時間風向旋轉角度
int pressCount=0;      //風速切換按鈕點選次數
int rotateState=-1;    //風向旋轉狀態 1(旋轉), -1(停止)
int rotateInterval=200;   //風向旋轉速度 (ms)

void setup() 
{
  Serial.begin(9600);
  
  pinMode(speedButtonPin, INPUT_PULLUP);
  pinMode(rotateButtonPin, INPUT_PULLUP);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);    
  
  myservo.attach(servoPin);
  myservo.write(angle);
}

void loop() 
{
  //讀取風速切換按鈕狀態
  if (digitalRead(speedButtonPin)==0)
  {
    pressCount=(pressCount+1)%4;     //風速四段切換
    //Serial.println(pressCount);
    
    switch (pressCount) 
    {
      case 0:
        analogWrite(motorPin1,0);
        analogWrite(motorPin2,0);
        break;
      case 1:
        analogWrite(motorPin1,100);  //0~255
        analogWrite(motorPin2,0);
        break;
      case 2:
        analogWrite(motorPin1,120);  //0~255
        analogWrite(motorPin2,0);
        break;
      case 3:
        analogWrite(motorPin1,140);  //0~255
        analogWrite(motorPin2,0);
        break;        
      default:
        analogWrite(motorPin1,0);
        analogWrite(motorPin2,0);        
        break;
    }
    delay(500);
  }
  
  //讀取風向旋轉切換按鈕狀態
  if (digitalRead(rotateButtonPin)==0)
  {
    rotateState*=(-1);     //風向旋轉狀態切換
    delay(500);
  }
  
  if (rotateState==1)      //風向來回轉動
  {
    angle+=degree;
    if ((angle<5)||(angle>175))
    {
      degree*=(-1);
      angle+=degree*2;
    }
    myservo.write(angle);
    
    delay(rotateInterval);
  }
}
