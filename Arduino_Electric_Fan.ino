#include <Servo.h>
Servo myservo;

int angle=90;          //風向初始角度
int degree=5;          //單位時間風向旋轉角度

int servoPin=4;        //伺服馬達腳位
int motorPin1=5;       //馬達驅動IC腳位
int motorPin2=6;       //馬達驅動IC腳位
int speedButtonPin=7;   //風速四段變速切換按鈕腳位(吊扇)
int rotateButtonPin=8;  //風向旋轉切換按鈕腳位

int pressCount=0;      //風速切換按鈕點選次數
int rotateState=-1;    //風向旋轉狀態 1=旋轉, -1=暫停

void setup() {
  pinMode(speedButtonPin, INPUT);
  pinMode(rotateButtonPin, INPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);    
  
  myservo.attach(servoPin);
  myservo.write(angle);
  
  Serial.begin(9600);
}

void loop() {
  //讀取風速切換按鈕狀態
  if (digitalRead(speedButtonPin)==1)
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
  
  //讀取風向旋轉切換按鈕狀態
  if (digitalRead(rotateButtonPin)==1)
  {
    rotateState*=(-1);     //改變風向旋轉狀態設定值
    delay(200);
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
    delay(200);
  }
}

