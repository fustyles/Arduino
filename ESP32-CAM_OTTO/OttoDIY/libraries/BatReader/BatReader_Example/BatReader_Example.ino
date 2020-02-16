#include "BatReader.h"

BatReader batreader;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

}

void loop() {
  Serial.print("Battery Voltage: ");
  Serial.println(batreader.readBatVoltage());
  Serial.print("Battery Percentage: ");
  Serial.print(batreader.readBatPercent());
  Serial.println("%\n\r");
  delay(1000);
}
