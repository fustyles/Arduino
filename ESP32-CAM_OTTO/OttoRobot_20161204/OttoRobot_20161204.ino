/*
OTTO ROBOT (SoftwareSerial)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-2-16 14:00
https://www.facebook.com/francefu

Libraries: https://github.com/fustyles/Arduino/tree/master/ESP32-CAM_OTTO/OttoDIY/libraries

ESP32-CAM RX - Arduino NANNO pin 6
ESP32-CAM TX - Arduino NANNO pin 7

servo 2 : Arduino NANNO pin 2
servo 3 : Arduino NANNO pin 3
servo 4 : Arduino NANNO pin 4
servo 5 : Arduino NANNO pin 5
Buzzer : Arduino NANNO pin 10
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

#include <SoftwareSerial.h>
// SoftwareSerial (RX, TX)
#define PIN_TX 6//connect ESP32-CAM RX pin here - pin 6
#define PIN_RX 7 //connect ESP32-CAM TX pin here - pin 7
SoftwareSerial ESP32Serial ( PIN_TX , PIN_RX );

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
OttoFretful OttoMagic  OttoWave  OttoVictory  OttoFail*/
///////////////////////////////////////////////////////////////////
//-- Global Variables -------------------------------------------//
///////////////////////////////////////////////////////////////////
bool obstacleDetected = false;
int T=1000;
///////////////////////////////////////////////////////////////////
//-- Setup ------------------------------------------------------//
///////////////////////////////////////////////////////////////////
void setup(){
  ESP32Serial.begin(9600); //setup your ESP32-CAM to match this baudrate (https://github.com/OttoDIY/OttoDIY)
  Serial.begin(9600);
  //Set the servo pins
  Otto.init(PIN_YL,PIN_YR,PIN_RL,PIN_RR,true);
  Otto.sing(S_connection); //Otto wake up!
  Otto.home();
  delay(50);
  Otto.sing(S_happy); // a happy Otto :)
}
///////////////////////////////////////////////////////////////////
//-- Principal Loop ---------------------------------------------//
///////////////////////////////////////////////////////////////////
void loop() {
  if (ESP32Serial.available()) //If something received over bluetooth, store the char

   dato= ESP32Serial.read();
   
   //check the char received
   switch(dato) {//if we receive a...
    case 66: // "B" Received
    {
      Otto.home();
      digitalWrite(12, !digitalRead(12));
      break;          
    }
    case 65: // "A" Received----OBSTACLE MODE ON!!!! (until a button is pressed in the app)------
    {
      obstacleMode();
      break;          
    }        
    case 85: //"U": Up arrow received
    {
      Otto.walk(2,T,1); //2 steps FORWARD
      break;
    }
    case 68: //"D": Down arrow received
    {
      Otto.walk(2,T,-1); //2 steps FORWARD
      break;
    }
    case 67: //"C": Center button received
    {
      Otto.playGesture(OttoFretful);
      Otto.home();
      Otto.sing(S_sleeping);
      delay(1000);
      T=1000;
      break;
    }
    case 76: //"L": Left arrow received
    {
      Otto.turn(2,T,1);//2 steps turning RIGHT                
      delay(50);
      break;
    }
    case 82: //"R": Right arrow received
    {
      Otto.turn(2,T,-1);//2 steps turning RIGHT                
      delay(50);
      break;
    }
    case 97: // "a" Received  shake
    {
      Otto.shakeLeg(1,T,1);
      Otto.home();
      break;
    }
    case 98: // "b" Received
    {
      Otto.shakeLeg(1,T,-1);
      Otto.home();
      break;           
    }
    case 99: // "c" Received
    {
      Otto.flapping(1,T,30,1);
      Otto.home();
      break;       
    }
    case 100: // "d" Received
    {
      Otto.flapping(1,T,30,-1);
      Otto.home();
      break;          
    }
    case 101: // "e" Received-
    {
      Otto.bend(1,T,1);
      Otto.home();
      break;     
    }
    case 102: // "f" Received-
    {
      Otto.bend(1,T,-1);
      Otto.home();
      break;     
    }
    case 103: // "g" Received-
    {
      Otto.moonwalker(1,T,30,1);
      Otto.home();
      break;     
    }
    case 104: // "h" Received-
    {
      Otto.moonwalker(1,T,30,-1);
      Otto.home();
      break;     
    }
    case 105: // "i" Received-
    {
      Otto.tiptoeSwing(1,T,30);
      Otto.home();
      break;     
    }
    case 106: // "j" Received-
    {
      Otto.jitter(1,T,30); 
      Otto.home();
      break;     
    }
    case 107: // "k" Received-
    {
      Otto.swing(1,T,30);
      Otto.home();
      break;     
    }
    case 108: // "l" Received-
    {
      Otto.updown(1,T,30);
      Otto.home();
      break;     
    }
    case 109: // "m" Received-
    {
      Otto.sing(S_surprise); 
      Otto.home();
      break;     
    }
    case 110: // "n" Received-
    {
      Otto.sing(S_sleeping);
      Otto.home();
      break;     
    }
    case 111: // "o" Received-
    {
      Otto.sing(S_happy); 
      Otto.home();
      break;     
     }
    case 112: // "p" Received-
    {
      Otto.sing(S_cuddly); 
      Otto.home();
      break;     
    }
    case 113: // "q" Received-
    {
      Otto.sing(S_sad); 
      Otto.home();
      break;     
    }
    case 114: // "r" Received-
    {
      Otto.sing(S_confused); 
      Otto.home();
      break;     
    }
    case 115: // "s" Received- Dance1
    {
      Dance1();
      Otto.home();
      break;     
    }
    case 116: // "t" Received-
    {
      Dance2();
      Otto.home();
      break;     
    }
    case 117: // "u" Received-
    {
      Dance3();
      Otto.home();
      break;     
    }        
    case 118: // "v" Received-
    {
      Dance4();
      Otto.home();
      break;     
    }
    case 119: // "w" Received-
    {
      T=T-100;
      if(T<100){T=1000;}
      Otto.home();
      break;     
    } 
    case 120: // "x" Received-
    {
      T=T+100;
      if(T>3000){T=1000;}
      Otto.home();
      break;  
    } 
  }
  dato=0; //clears the incoming data until it receives the next button from app
}

void Dance1(){
  int x=1;
  while(!ESP32Serial.available()) {
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
  while(!ESP32Serial.available()) {
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
  while(!ESP32Serial.available()) {
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
  while(!ESP32Serial.available()) {
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
  while(!ESP32Serial.available()) {
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
