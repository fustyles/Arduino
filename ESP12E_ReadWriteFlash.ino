/* 
NodeMCU(ESP12E) write WIFI SSID and password to SPI FLASH and read data from SPI FLASH.
Author : ChungYi Fu(Kaohsiung, Taiwan)  2019-4-24 16:00
https://www.facebook.com/francefu
*/

#include <ESP8266WiFi.h> 
#include <ESP.h>

const int len = 64;    // flashWrite, flashRead -> i = 0 to 63

char ssid[len]    = "xxxxx";
char password[len]    = "xxxxx";
const char* apssid = "ESP12E";
const char* appassword = "12345678";   //AP password require at least 8 characters.

const uint32_t addressStart = 0x3FA000; 
const uint32_t addressEnd   = 0x3FAFFF;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Write WIFI SSID and password to SPI FLASH
  flashErase();
  flashWrite(ssid, 0);  
  flashWrite(password, 1);

  // Read WIFI SSID and password from SPI FLASH
  char buff_ssid[len], buff_password[len];
  strcpy(buff_ssid, flashRead(0));
  strcpy(buff_password, flashRead(1));
  Serial.printf("ssid: \"%s\"\npassword: \"%s\"\n", buff_ssid, buff_password);

  if ((buff_ssid[0]>=32)&&(buff_ssid[0]<=126))
    connectWIFI(buff_ssid, buff_password);
}

void loop() {
    delay(500);
}

void connectWIFI(char id[len], char pwd[len]) {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(id, pwd);
  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(id);
  
  long int StartTime=millis();
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");    
    if ((StartTime+10000) < millis()) break;
  } 

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());    
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
  }
  else {
    Serial.println("\nConnection Failed");
    WiFi.softAP(apssid, appassword);  
  }
  
  server.begin();
}

void flashWrite(char data[len], int i) {      // i = 0 to 63
  uint32_t flashAddress = addressStart + i*len;
  char buff_write[len];
  strcpy(buff_write, data);
  if (ESP.flashWrite(flashAddress,(uint32_t*)buff_write, sizeof(buff_write)-1))
    Serial.printf("address: %p write \"%s\" [ok]\n", flashAddress, buff_write);
  else 
    Serial.printf("address: %p write \"%s\" [error]\n", flashAddress, buff_write);
}

char* flashRead(int i) {      // i = 0 to 63
  uint32_t flashAddress = addressStart + i*len;
  static char buff_read[len];
  if (ESP.flashRead(flashAddress,(uint32_t*)buff_read, sizeof(buff_read)-1)) {
    //Serial.printf("data: \"%s\"\n", buff_read);
    return buff_read;
  } else  
    return "";  
}

void flashErase() {
  if (ESP.flashEraseSector(addressStart / 4096))
    Serial.println("\nErase [ok]");
  else
    Serial.println("\nErase [error]");
}

/*
bool flashEraseSector(uint32_t sector);
bool flashWrite(uint32_t offset, uint32_t *data, size_t size);
bool flashRead(uint32_t offset, uint32_t *data, size_t size);
*/
