/* 
ESP32-CAM reads out the MLX90615 or MLX90614 infrared thermometer
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-6-1 19:00
https://www.facebook.com/francefu

SDA: IO2
SCL: IO13

I2C Library 
https://github.com/felias-fogg/SlowSoftI2CMaster

MLX90615 Datasheet
https://www.melexis.com/-/media/files/documents/datasheets/mlx90615-datasheet-melexis.pdf

MLX90614 Datasheet
https://www.melexis.com/-/media/files/documents/datasheets/mlx90614-datasheet-melexis.pdf

Refer to the code.
https://github.com/felias-fogg/SoftI2CMaster/blob/master/examples/MLX90614Soft/MLX90614Soft.ino
*/

#include <SlowSoftI2CMaster.h>
SlowSoftI2CMaster si = SlowSoftI2CMaster(2, 13, true);  // SlowSoftI2CMaster(uint8_t sda, uint8_t scl, bool pullup)

void setup(){
  Serial.begin(9600);
  si.i2c_init();
}

void loop(){
  //MLX90615
  float temperature = getMLX9061X(0, 0.00, 0x5B ,0x27);  //MLX90615 object
  //float temperature = getMLX9061X(0, 0.00, 0x5B ,0x26);  //MLX90615 ambient
  
  //MLX90614
  //float temperature = getMLX9061X(0, 0.00, 0x5A ,0x07);  //MLX90614 object1
  //float temperature = getMLX9061X(0, 0.00, 0x5A ,0x06);  //MLX90614 ambient 
  //float temperature = getMLX9061X(0, 0.00, 0x5A ,0x08);  //MLX90614 object2
     
  Serial.println(temperature);
  delay(1000);
}

//scale: 0(Celcius), 1(Fahrenheit), 2(Kelvin)
float getMLX9061X(byte scale, float compensation, uint8_t addr ,uint8_t obj) {
  int dev = addr<<1;
  int data_low = 0;
  int data_high = 0;
  int pec = 0;
  
  si.i2c_start(dev+I2C_WRITE);
  si.i2c_write(obj); 
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
