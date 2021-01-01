/*
Arduino Leonardo Keyes IR Remote Control for PPT
Author    ChungYi Fu (Kaohsiung, Taiwan)  2021-01-01 21:00 
https://www.facebook.com/francefu

Keyboard Modifiers (keyboardpress)
https://www.arduino.cc/en/Reference/KeyboardModifiers
 
PPT Remote Control (keyboard press = keycode1;keycode2;presstime)
keyboard write = 198          "F5"
keyboard write = 211          "PAGE UP"
keyboard press = 133;198;10  "SHIFT+F5"
keyboard write = 214          "PAGE DOWN"
keyboard write = 87           "W"
keyboard write = 177          "ESC"
keyboard write = 66           "B"

Game (keyboard press = keycode1;keycode2;presstime)
keyboard press = 215;;100     "KEY_RIGHT_ARROW"
keyboard press = 216;;100     "KEY_LEFT_ARROW"
keyboard press = 217;;100     "KEY_DOWN_ARROW"
keyboard press = 218;;200     "KEY_UP_ARROW"
keyboard press = 215;218;200  "KEY_RIGHT_ARROW + KEY_UP_ARROW"
keyboard press = 216;218;200  "KEY_LEFT_ARROW + KEY_UP_ARROW"
*/

#include <Keyboard.h>
#include <IRremote.h>        //引用紅外線函式庫

int RECV_PIN =11;            //紅外線腳位
IRrecv irrecv(RECV_PIN);     //定義IRrec物件接收紅外線訊號
decode_results results;      //解碼結果放在decode_results結構的result變數內

void setup()
{
  Serial.begin(9600);
  while (!Serial){}  
  Serial.println("Ready");
  irrecv.blink13(true);     //收到訊號時，腳位13的LED會閃爍
  irrecv.enableIRIn();      //啟動紅外線解碼  
}

void loop() 
{
/*
  0xFF629D    FORWARD
  0xFF22DD    LEFT
  0xFF02FD    OK
  0xFFC23D    RIGHT
  0xFFA857    REVERSE
  0xFF6897    1
  0xFF9867    2
  0xFFB04F    3
  0xFF30CF    4
  0xFF18E7    5
  0xFF7A85    6
  0xFF10EF    7
  0xFF38C7    8
  0xFF5AA5    9
  0xFF42BD    *
  0xFF4AB5    0
  0xFF52AD    #
  0xFFFFFFFF  REPEAT
*/
  
  if (irrecv.decode(&results)) {            //如果解碼成功
    //Serial.println(results.value, HEX);   //顯示結果轉換成十六進制數值
     
    if (results.value==0xFF22DD) {  //F5 (LEFT)
        Keyboard.write(char(198));
        Serial.println("F5");
    } 
    else if (results.value==0xFF629D) {  //PAGE UP (FORWARD)
        Keyboard.write(char(211));
        Serial.println("PAGE UP");
    }    
    else if (results.value==0xFFC23D) {  //SHIFT+F5 (RIGHT)
        Keyboard.press(char(133));
        Keyboard.press(char(198));
        delay(10);
        Keyboard.releaseAll();
        Serial.println("SHIFT+F5");
    }  
    else if (results.value==0xFFA857) {  //PAGE DOWN (REVERSE)
        Keyboard.write(char(214));
        Serial.println("PAGE DOWN");
    }  
    else if (results.value==0xFF6897) {  //ESC (1)
        Keyboard.write(char(177));
        Serial.println("ESC");
    } 
    else if (results.value==0xFF9867) {  //W (2)
        Keyboard.write(char(87));
        Serial.println("W");
    }   
    else if (results.value==0xFFB04F) {  //B (3)
        Keyboard.write(char(66));
        Serial.println("B");
    }
    else
        Serial.println("Undefined");
    
    delay(500);    
    irrecv.resume(); //繼續收下一組紅外線訊號 
  }
}
