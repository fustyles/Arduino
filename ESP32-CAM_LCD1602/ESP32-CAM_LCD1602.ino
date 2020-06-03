/*
ESP32-CAM LCD1602
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-6-3 21:00
https://www.facebook.com/francefu

Library
https://github.com/johnrickman/LiquidCrystal_I2C
You must delete other LiquidCrystal_I2C libraries.
*/

byte pinSDA = 2;
byte pinSCL = 13; 
int addr = 0x27;  //0x27 or ox3F

#include "LiquidCrystal_I2C.h"
LiquidCrystal_I2C lcd(addr, 16, 2);
#include <Wire.h>
TwoWire Wire0 = TwoWire(0);

void setup() {
  Wire.begin(pinSDA, pinSCL);
  Wire.setClock(10000);
  
  Serial.begin(115200);
  while (!Serial);
  
  lcd.init();
  lcd.backlight();
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hello, World!");
}

void loop() {
}
