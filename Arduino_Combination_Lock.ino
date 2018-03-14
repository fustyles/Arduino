#include <Servo.h> 
Servo myservo;     //建立Servo物件控制門鎖
int ServorPIN=3;   //Servo輸出腳位

int btn0=4;    //重設按鈕腳位
int btn1=5;    //密碼數字1按鈕腳位
int btn2=6;    //密碼數字2按鈕腳位
int btn3=7;    //密碼數字3按鈕腳位
int ledR=9;    //指示燈Red腳位
int ledG=10;   //指示燈Green腳位
int ledB=11;   //指示燈Blue腳位
double t;    //暫存連續輸入密碼間隔時間
int stateInitial=0;   //輸入密碼狀態 1-初始化設定密碼狀態，2-解鎖輸入密碼狀態
int Timelimit=1000;  //連續輸入密碼逾時時間(ms)
int n1=0;    //第一個密碼正在輸入狀態
int n2=0;    //第二個密碼正在輸入狀態
int n3=0;    //第三個密碼正在輸入狀態
String pwd1="";  //紀錄初始化密碼第一個數字
String pwd2="";  //紀錄初始化密碼第二個數字
String pwd3="";  //紀錄初始化密碼第三個數字
String inp1="";  //紀錄輸入解鎖密碼第一個數字
String inp2="";  //紀錄輸入解鎖密碼第二個數字
String inp3="";  //紀錄輸入解鎖密碼第三個數字

void setup() {
  Serial.begin(9600);         //鮑率
  
  myservo.attach(ServorPIN);   //設定Servo輸出腳位
  myservo.write(0);         //寫入Servo角度位置         
  delay(2000);              //等待Servo旋轉至指定角度         

  pinMode(btn0, INPUT);
  pinMode(btn1, INPUT); 
  pinMode(btn2, INPUT); 
  pinMode(btn3, INPUT); 

  pinMode(ledR, OUTPUT);    
  pinMode(ledG, OUTPUT);   
  pinMode(ledB, OUTPUT);  
  
  analogWrite(ledR,0);
  analogWrite(ledG,0);
  analogWrite(ledB,0);  
}

void loop() {
  int s0 = digitalRead(btn0);      //讀取重設鍵電位訊號 HIGH=1(電位≥2.5V)，LOW=0(電位<2.5V)
  //Serial.println(s0);
  int s1 = digitalRead(btn1);      //讀取密碼1電位訊號 HIGH=1(電位≥2.5V)，LOW=0(電位<2.5V)
  //Serial.println(s1);
  int s2 = digitalRead(btn2);      //讀取密碼2電位訊號 HIGH=1(電位≥2.5V)，LOW=0(電位<2.5V)
  //Serial.println(s2);
  int s3 = digitalRead(btn3);      //讀取密碼3電位訊號 HIGH=1(電位≥2.5V)，LOW=0(電位<2.5V)
  //Serial.println(s3);
  
  if (s0==HIGH)             //重設鍵按下
  {
    n1=0;n2=0;n3=0;t=0;pwd1="";pwd2="";pwd3="";
    //亮藍燈
    analogWrite(ledR,0);
    analogWrite(ledG,0);
    analogWrite(ledB,255); 

    stateInitial=1;   //1-初始化密碼設定狀態，2-開鎖輸入狀態
    myservo.write(0);         //Servo旋轉至解鎖位置       
  }

  if ((s1==HIGH||s2==HIGH||s3==HIGH)&&(stateInitial==1||stateInitial==2))          //若偵測到密碼按鍵按下
  {
      //綠燈閃爍一次
      analogWrite(ledR,0);
      analogWrite(ledG,255);
      analogWrite(ledB,0); 
      delay(10);
      analogWrite(ledR,0);
      analogWrite(ledG,0);
      analogWrite(ledB,0); 
      
      if (n1==0)   //尚未輸入任何密碼
      {
        n1=1;      //輸入第一個密碼狀態
        n2=0;
        n3=0;
        t=0;
        if (stateInitial==1)
        {
          if (s1==HIGH)  pwd1="1";
          if (s2==HIGH)  pwd1="2";
          if (s3==HIGH)  pwd1="3";
        }
        else if (stateInitial==2)
        {
          if (s1==HIGH)  inp1="1";
          if (s2==HIGH)  inp1="2";
          if (s3==HIGH)  inp1="3";
        }
      }
      else if (n1==1&&n2==0&&t>=100&&t<=Timelimit)  //隔Timelimit時間內偵測到輸入第二個密碼
      {
        n2=1;     //輸入第二個密碼狀態
        n3=0;
        t=0;
        if (stateInitial==1)
        {
          if (s1==HIGH)  pwd2="1";
          if (s2==HIGH)  pwd2="2";
          if (s3==HIGH)  pwd2="3";
        }
        else if (stateInitial==2)
        {
          if (s1==HIGH)  inp2="1";
          if (s2==HIGH)  inp2="2";
          if (s3==HIGH)  inp2="3";
        }
      }
      else if (n2==1&&n3==0&&t>=100&&t<=Timelimit)  //隔Timelimit時間內偵測到輸入第三個密碼
      {
        n3=1;     //輸入第三個密碼狀態
        t=0;
        if (stateInitial==1)
        {
          if (s1==HIGH)  pwd3="1";
          if (s2==HIGH)  pwd3="2";
          if (s3==HIGH)  pwd3="3";
        }
        else if (stateInitial==2)
        {
          if (s1==HIGH)  inp3="1";
          if (s2==HIGH)  inp3="2";
          if (s3==HIGH)  inp3="3";
        }
      }
  }
  if (t>Timelimit&&n3==1)   //時間範圍內偵測到輸入三個密碼
  {
    if (stateInitial==1)
    {
      //密碼已設定，門鎖上鎖
      myservo.write(90);    

      //亮紅燈                
      analogWrite(ledR,255);
      analogWrite(ledG,0);
      analogWrite(ledB,0); 

      stateInitial=2;
    }
    else if (stateInitial==2)
    {
      if (pwd1==inp1&&pwd2==inp2&&pwd3==inp3)
      {
        //密碼正確，亮白燈
        analogWrite(ledR,255);
        analogWrite(ledG,255);
        analogWrite(ledB,255); 

        //門鎖解鎖
        myservo.write(0);         //寫入Servo角度位置        
      }
      else
      {
        //密碼錯誤，亮紅燈
        analogWrite(ledR,255);
        analogWrite(ledG,0);
        analogWrite(ledB,0);         

        inp1="";inp2="";inp3="";
      }
    }
    n1=0;n2=0;n3=0;t=0;
  }  
  else if (t>Timelimit&&n3==0)   //時間範圍內偵測密碼不足三位
  {
    if (stateInitial==1)
    {
      //密碼不足三位，亮藍燈
      analogWrite(ledR,0);
      analogWrite(ledG,0);
      analogWrite(ledB,255); 

      pwd1="";pwd2="";pwd3="";inp1="";inp2="";inp3="";
    }
    else if (stateInitial==2)
    {
      //密碼錯誤，亮紅燈
      analogWrite(ledR,255);
      analogWrite(ledG,0);
      analogWrite(ledB,0); 

      inp1="";inp2="";inp3="";      
    }
    n1=0;n2=0;n3=0;t=0;
  }

  if (n1==1||n2==1)   //偵測到第一次或第二次輸入密碼後開始重新計時
  {
    delay(1);
    t=t+1;
  }  
}
