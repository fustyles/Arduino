/* 
LinkIt7697 (or ESP32) reads out the MLX90614 infrared thermometer
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-3-30 23:30
https://www.facebook.com/francefu

Library: https://github.com/felias-fogg/SlowSoftI2CMaster

MLX90614
SDA: pin 9
SCL: pin 8
*/

#include <SlowSoftI2CMaster.h>
SlowSoftI2CMaster si = SlowSoftI2CMaster(9, 8, true);

void setup(){
  Serial.begin(9600);
  si.i2c_init();
}

void loop(){
  float temperature = getMLX90615(0, 0.00);  //scale: 0(Celcius), 1(Fahrenheit), 2(Kelvin)
  Serial.println(temperature);
  delay(1000); // wait a second before printing again
}

float getMLX90615(int scale, float compensation) {
  int dev = 0x5B<<1;
  int data_low = 0;
  int data_high = 0;
  int pec = 0;
  
  si.i2c_start(dev+I2C_WRITE);
  si.i2c_write(0x27);
  // read
  si.i2c_rep_start(dev+I2C_READ);
  data_low = si.i2c_read(false); //Read 1 byte and then send ack
  data_high = si.i2c_read(false); //Read 1 byte and then send ack
  pec = si.i2c_read(true);
  si.i2c_stop();
  
  //This converts high and low bytes together and processes temperature, MSB is a error bit and is ignored for temps
  double val = 0x0000; // zero out the data
  // This masks off the error bit of the high byte, then moves it left 8 bits and adds the low byte.
  val = (double)(((data_high & 0x007F) << 8) + data_low);
  double myfactor = 0.02; // 0.02 degrees per LSB (measurement resolution of the MLX90614)
  float kelvin = (val * myfactor)-0.01;
  
  float celcius = kelvin - 273.15;
  float fahrenheit = (celcius*1.8) + 32;
  
  if (scale==0)
    return (celcius + compensation);
  else if (scale==1)
    return (fahrenheit + compensation);
  else if (scale==2)
    return (kelvin + compensation);    
}
