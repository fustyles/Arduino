void ShowLED(int p);     //自定函式

//麥克風感測器由左而右 A0,G,+,D0
//調整麥克風感測器可變電阻值使類比訊號值約500左右

int m1=A0;   //麥克風A0腳位
int m2=12;    //麥克風D0腳位
double t;    //紀錄連續擊掌間隔時間
int timelimit=800;  //連續擊掌逾時間隔時間
int n1=0;    //第一次擊掌狀態
int n2=0;    //第二次擊掌狀態
int n3=0;    //第三次擊掌狀態
int bin1=0;    //紀錄第一次擊掌結果
int bin2=0;    //紀錄第二次擊掌結果
int bin3=0;    //紀錄第三次擊掌結果
int lim1=520;  //產生聲音下限值
int lim2=700;  //產生聲音大小聲區分值 
int LED[]={2,3,4,5,6,7,8,9};  //控制8個LED燈腳位
int LEDs[]={10,11,12};  //擊掌指示燈

void setup() { 
  for (int i=0;i<sizeof(LED)/sizeof(LED[0]);i++)   
    pinMode(LED[i], OUTPUT); 

  pinMode(m1,INPUT);      //麥克風A0輸入模式
  pinMode(m2,INPUT);      //麥克風D0輸入模式
  Serial.begin(9600);        //鮑率
}

void loop() {
 
int s1=digitalRead(m2);     //讀取D0數位訊號
int s2=analogRead(m1);     //讀取A0類比訊號
                
 if (s1==0)          //若偵測到聲音產生低電位訊號
  {
    if (s2>=lim1)   //偵測到聲音高於下限值，表示由敲擊產生
    { 
      if (n1==0)   //尚未擊掌過
      {
        n1=1;      //第一次擊掌狀態
        Serial.print("1->");
        Serial.println(s2);
        digitalWrite(LEDs[0],HIGH);
        if (s2>=lim2)
          bin1=1;    //音量>=基準值，第一位數字設為1
        else
          bin1=0;    //音量<基準值，第一位數字設為0
        n2=0;
        n3=0;
        t=0;
        delay(200); 
      }
      //隔一秒內偵測到第二次擊掌
      else if (n1==1&&n2==0&&t>=200&&t<=timelimit)  
      {
          n2=1;     //第二次擊掌狀態
          Serial.print("2->");
          Serial.println(s2);
          digitalWrite(LEDs[1],HIGH);
          if (s2>=lim2)
            bin2=1;
          else
            bin2=0;  
          n3=0;
          t=0;
          delay(200);
      }
      //隔一秒內偵測到第三次擊掌
      else if (n2==1&&n3==0&&t>=200&&t<=timelimit)  
      {
          n3=1;     //第三次擊掌狀態
          Serial.print("3->");
          Serial.println(s2);
          digitalWrite(LEDs[2],HIGH);
          if (s2>=lim2)
            bin3=1;
          else
            bin3=0;  
          t=0;
      }
    }
  }
  if (t>timelimit)   //二進制數字轉成十進制數字顯示
    {
      if (n1==1&&n2==1&&n3==1)
        ShowLED(bin1*4+bin2*2+bin3);  
      
      n1=0;
      n2=0;
      n3=0;
      t=0;
      
      bin1=0;
      bin2=0;
      bin3=0;

      digitalWrite(LEDs[0],LOW);
      digitalWrite(LEDs[1],LOW);
      digitalWrite(LEDs[2],LOW);
      delay(100);
    }   
  if (n1==1||n2==1)   //偵測到第一次或第二次擊掌後開始重新計時
  {
    delay(1);
    t=t+1;
  } 
}

void ShowLED(int p)
{
  digitalWrite(LED[p],HIGH);
}
