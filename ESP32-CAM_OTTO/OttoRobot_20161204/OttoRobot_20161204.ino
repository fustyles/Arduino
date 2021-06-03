/*
OTTO ROBOT 20161204
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-2-17 19:00
https://www.facebook.com/francefu

Libraries: https://github.com/fustyles/Arduino/tree/master/ESP32-CAM_OTTO/OttoDIY/libraries

ESP32-CAM RX - Arduino NANO TX
ESP32-CAM TX - Arduino NANO RX

servo 2 : Arduino NANO pin 2
servo 3 : Arduino NANO pin 3
servo 4 : Arduino NANO pin 4
servo 5 : Arduino NANO pin 5
Buzzer : Arduino NANO pin 10
*/

//----------------------------------------------------------------
//-- Easy Otto´s sketch controlled by BlueControl Android app. Each button has different functionallity
//-- The app send a char each time a button is pressed.
//-- "E" button enters obstacle avoidance mode
//-- Otto basic firmware v2 adapted from Zowi (ottobot.org)https://github.com/OttoDIY/OttoDIY
//-- CC BY SA
//-- 04 December 2016
//-- Adapted Pablo García pabloeweb@gmail.com 01 March 2017
//-----------------------------------------------------------------

#include <Servo.h> 
#include <Oscillator.h>
#include <US.h>
#include <Otto.h>
Otto Otto;  //This is Otto!
byte dato;  //To store the char sent by the app
int speedx;

//---------------------------------------------------------
//-- First step: Make sure the pins for servos are in the right position
/*
         --------------- 
        |     O   O     |
        |---------------|
YR 3==> |               | <== YL 2
         --------------- 
            ||     ||
RR 5==>   -----   ------  <== RL 4
         |-----   ------|
*/

#define PIN_YL 2 //servo[2]
#define PIN_YR 3 //servo[3]
#define PIN_RL 4 //servo[4]
#define PIN_RR 5 //servo[5]
  
/*SOUNDS******************
 * S_connection  S_disconnection  S_buttonPushed S_mode1 S_mode2 S_mode3 S_surprise S_OhOoh  S_OhOoh2  S_cuddly 
 * S_sleeping  S_happy S_superHappy S_happy_short S_sad S_confused S_fart1 S_fart2  S_fart3 
 */
/*MOVEMENTS LIST**************
 * dir=1---> FORWARD/LEFT
 * dir=-1---> BACKWARD/RIGTH
 * T : amount of movement. HIGHER VALUE SLOWER MOVEMENT usually 1000 (from 600 to 1400)
 * h: height of mov. around 20
     jump(steps=1, int T = 2000);
     walk(steps, T, dir);
     turn(steps, T, dir);
     bend (steps, T, dir); //usually steps =1, T=2000
     shakeLeg (steps, T, dir);
     updown(steps, T, HEIGHT);
     swing(steps, T, HEIGHT);
     tiptoeSwing(steps, T, HEIGHT);
     jitter(steps, T, HEIGHT); (small T)
     ascendingTurn(steps, T, HEIGHT);
     moonwalker(steps, T, HEIGHT,dir);
     crusaito(steps, T, HEIGHT,dir);
     flapping(steps, T, HEIGHT,dir);
/*GESTURES LIST***************
OttoHappy OttoSuperHappy  OttoSad   OttoSleeping  OttoFart  OttoConfused OttoLove  OttoAngry   
OttoFretful OttoMagic  OttoWave  OttoVictory  OttoFail
*/

bool obstacleDetected = false;
int T=1000;

void setup(){
  Serial.begin(9600);
  //Set the servo pins
  Otto.init(PIN_YL,PIN_YR,PIN_RL,PIN_RR,true);
  Otto.sing(S_connection); //Otto wake up!
  Otto.home();
  delay(50);
  Otto.sing(S_happy); // a happy Otto :)
}

void loop() {
  String cmd="";
  
  if (Serial.available()) {
    while (Serial.available()) {
      char c = Serial.read();
      if (c!='\r'&&c!='\n') cmd=cmd+String(c);
      delay(1);
    }
  }

  if (cmd!="") {
    Serial.println(cmd);
    if (cmd=="B") {
      Otto.home();
      digitalWrite(12, !digitalRead(12));
    }
    else if (cmd=="A") {
      obstacleMode();
    }        
    else if (cmd=="U") {
      Otto.walk(2,T,1); //2 steps FORWARD
    }
    else if (cmd=="D") {
      Otto.walk(2,T,-1); //2 steps FORWARD
    }
    else if (cmd=="C") {
      Otto.playGesture(OttoFretful);
      Otto.home();
      Otto.sing(S_sleeping);
      delay(1000);
      T=1000;
    }
    else if (cmd=="L") {
      Otto.turn(2,T,1);//2 steps turning RIGHT                
      delay(50);
    }
    else if (cmd=="R") {
      Otto.turn(2,T,-1);//2 steps turning RIGHT                
      delay(50);
    }
    else if (cmd=="a") {
      Otto.shakeLeg(1,T,1);
      Otto.home();
    }
    else if (cmd=="b") {
      Otto.shakeLeg(1,T,-1);
      Otto.home();
    }
    else if (cmd=="c") {
      Otto.flapping(1,T,30,1);
      Otto.home();
    }
    else if (cmd=="d") {
      Otto.flapping(1,T,30,-1);
      Otto.home();
    }
    else if (cmd=="e") {
      Otto.bend(1,T,1);
      Otto.home();
    }
    else if (cmd=="f") {
      Otto.bend(1,T,-1);
      Otto.home();
    }
    else if (cmd=="g") {
      Otto.moonwalker(1,T,30,1);
      Otto.home();
    }
    else if (cmd=="h") {
      Otto.moonwalker(1,T,30,-1);
      Otto.home();
    }
    else if (cmd=="i") {
      Otto.tiptoeSwing(1,T,30);
      Otto.home();
    }
    else if (cmd=="j") {
      Otto.jitter(1,T,30); 
      Otto.home();
    }
    else if (cmd=="k") {
      Otto.swing(1,T,30);
      Otto.home();
    }
    else if (cmd=="l") {
      Otto.updown(1,T,30);
      Otto.home();
    }
    else if (cmd=="m") {
      Otto.sing(S_surprise); 
      Otto.home();
    }
    else if (cmd=="n") {
      Otto.sing(S_sleeping);
      Otto.home();
    }
    else if (cmd=="o") {
      Otto.sing(S_happy); 
      Otto.home();
    }
    else if (cmd=="p") {
      Otto.sing(S_cuddly); 
      Otto.home();
    }
    else if (cmd=="q") {
      Otto.sing(S_sad); 
      Otto.home();
    }
    else if (cmd=="r") {
      Otto.sing(S_confused); 
      Otto.home();
    }
    else if (cmd=="s") {
      Dance1();
      Otto.home();
    }
    else if (cmd=="t") {
      Dance2();
      Otto.home();
    }
    else if (cmd=="u") {
      Dance3();
      Otto.home();
    }
    else if (cmd=="v") {
      Dance4();
      Otto.home();
    }
    else if (cmd=="w") {
      T=T-100;
      if(T<100){T=1000;}
      Otto.home();
    }
    else if (cmd=="x") {
      T=T+100;
      if(T>3000){T=1000;}
      Otto.home();
    }
    else if (cmd=="myface") {
      Otto._tone(note_C6,50,200);
      Otto._tone(note_B5,50,200);
      Otto._tone(note_A5,50,200);
      Otto._tone(note_G5,50,200);
      Otto.updown(1,T,30);
      Otto.home();
      Otto._tone(note_E5,50,200);
      Otto._tone(note_G5,50,200);
      Otto._tone(note_E5,50,200);
      Otto._tone(note_G5,50,200);
      Otto._tone(note_A5,50,200); 
      Otto.jitter(1,T,30);
      Otto.home();
    }    
  }
}

void Dance1(){
  int x=1;
  while(!Serial.available()) {
    obstacleDetector(); //check for obstacle
    if(obstacleDetected) { 
       Otto.sing(S_cuddly); 
       Otto.home();
       break;
    } else {
      Otto._tone(note_E5,50,30);
      Otto._tone(note_E6,55,25);
      Otto._tone(note_A6,60,10);
       Otto.moonwalker(1, 300, 30, x);
      Otto._tone(note_E5,50,30);
      Otto._tone(note_E6,55,25);
      Otto._tone(note_A6,60,10);
      Otto.moonwalker(1, 300, 30, x);  
      x=-1*x;
      obstacleDetector(); 
    }
  }
}

void Dance2(){
  int x=1;
  while(!Serial.available()) {
   obstacleDetector(); //check for obstacle
    if(obstacleDetected) { 
     Otto.sing(S_sad); 
     Otto.home();
     break;
    } else{
      Otto.moonwalker(1, T, 30, x); 
      Otto.shakeLeg(1,T,x);
      x=-1*x;
      obstacleDetector(); 
    }
  }
}

void Dance3(){
  int x=1;
  while(!Serial.available()) {
    obstacleDetector(); //check for obstacle
    if(obstacleDetected) { 
       Otto.sing(S_fart1); 
       Otto.home();
       break;
    } else {
      Otto.jitter(1,T,30);
      Otto.ascendingTurn(1,T,30);
      Otto.crusaito(1,T,30,x);
      Otto.crusaito(1,T,30,-1*x);
      Otto.tiptoeSwing(1,T,30);
      Otto.flapping(1,T,30,x);
      x=-1*x;
      obstacleDetector(); 
    }
  }
}

void Dance4(){
  int x=1;
  while(!Serial.available()) {
   obstacleDetector(); //check for obstacle
    if(obstacleDetected) { 
       Otto.sing(S_surprise); 
       Otto.home();
       break;
    } else {
      Otto.moonwalker(1, T, 30, x); 
      Otto.shakeLeg(1,T,x);
      x=-1*x;
      obstacleDetector(); 
    }
  }
}
///////////////////////////////////////////////////////////////////
//-- Function to avoid obstacles until another key is pressed in the app
void obstacleMode(){
  Otto.sing(S_OhOoh);  
  while(!Serial.available()) {
    obstacleDetector(); //check for obstacle
    if(obstacleDetected){ 
      Otto.sing(S_surprise); 
      Otto.playGesture(OttoFretful); 
      Otto.sing(S_fart3); 
      Otto.walk(2,T,-1); 
      Otto.turn(3,T,-1);                
      delay(50); 
      obstacleDetector(); 
    } else { 
      Otto.walk(2,T,1); 
      obstacleDetector(); 
    }
  }
  Otto.home();
}
///////////////////////////////////////////////////////////////////
//-- Function to read distance sensor & to actualize obstacleDetected variable
void obstacleDetector(){
  int distance = Otto.getDistance();
  if(distance<15) {
    obstacleDetected = true;
  } else {
    obstacleDetected = false;
  }
}
