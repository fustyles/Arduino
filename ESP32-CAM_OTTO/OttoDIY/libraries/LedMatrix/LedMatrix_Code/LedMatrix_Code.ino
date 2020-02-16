#include <LedMatrix.h>
 
LedMatrix ledmatrix(11, 13, 12);

void setup() {           
  Serial.begin(115200);  
  
  ledmatrix.writeFull(0x3FFFFFFF);  
  delay(1000);
  
  ledmatrix.clearMatrix();
  delay(1000);
  
}

void loop() {
  ledmatrix.setLed(1, 1);
  ledmatrix.setLed(2, 2);
  ledmatrix.setLed(3, 3);
  ledmatrix.setLed(4, 4);
  ledmatrix.setLed(5, 5);
  
  
  printLong(ledmatrix.readFull());
}

void printLong(unsigned long toprint) {
  unsigned long i;
  for(i = 0; i < 32; i++) {
    if(toprint & (1L << (31 - i))) Serial.print("1");
    else Serial.print("0");
  } 
  Serial.println("");
}
