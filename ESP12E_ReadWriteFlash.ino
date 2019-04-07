/* 
NodeMCU(ESP12E) write WIFI SSID and password to SPI FLASH and read data from SPI FLASH.
Author : ChungYi Fu(Kaohsiung, Taiwan)  2019-4-7 12:00
https://www.facebook.com/francefu
*/

#include <ESP8266WiFi.h> 
#include <ESP.h>

const int len = 64;
char ssid[len]    = "xxxxx";
char password[len]    = "xxxxx";

const char* apssid = "ESP12E";
const char* appassword = "12345678";   //AP password require at least 8 characters.

char buff_ssid[len];
char buff_password[len];
const uint32_t addressStart = 0x3FA000; 
const uint32_t addressEnd   = 0x3FAFFF;

void setup() {
  Serial.begin(115200);

  /* Test
  flashWrite(ssid, password);  // Write correct SSID and password to SPI FLASH
  strcpy(ssid,"test");  // Set nonexistent SSID
  */
  
  connectWIFI(ssid, password);
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nRead settings from SPI FLASH");
    flashRead();
    connectWIFI(buff_ssid, buff_password);
  }
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
    flashWrite(id, pwd);  // Write SSID and password to SPI FLASH
  }
  else {
    Serial.println("\nConnection Failed");
    WiFi.softAP(apssid, appassword);  
  }
}

void flashWrite(char id[len], char pwd[len]) {
  flashErase();
  strcpy(buff_ssid, id);
  strcpy(buff_password, pwd);
  
  uint32_t flashAddress;
    
  flashAddress = addressStart + 0*len;
  if (ESP.flashWrite(flashAddress,(uint32_t*)buff_ssid, sizeof(buff_ssid)))
    Serial.printf("address: %p write \"%s\" [ok]\n", flashAddress, buff_ssid);
  else 
    Serial.printf("address: %p write \"%s\" [error]\n", flashAddress, buff_ssid);

  flashAddress = addressStart + 1*len;
  if (ESP.flashWrite(flashAddress,(uint32_t*)buff_password, sizeof(buff_password)))
    Serial.printf("address: %p write \"%s\" [ok]\n", flashAddress, buff_password);
  else 
    Serial.printf("address: %p write \"%s\" [error]\n", flashAddress, buff_password);
}

void flashRead() {
  uint32_t flashAddress;
  
  flashAddress = addressStart + 0*len;
  memset(buff_ssid, 0, sizeof(buff_ssid));
  if (ESP.flashRead(flashAddress,(uint32_t*)buff_ssid, sizeof(buff_ssid)))
    Serial.printf("[ssid] %s \n", buff_ssid);
  else 
    Serial.printf("[ssid] error \n"); 
  
  flashAddress = addressStart + 1*len;
  memset(buff_password, 0, sizeof(buff_password));
  if (ESP.flashRead(flashAddress,(uint32_t*)buff_password, sizeof(buff_password)))
    Serial.printf("[password] %s \n", buff_password);
  else 
    Serial.printf("[ssid] error \n"); 
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
void *memset(void *str, int c, size_t n)
*/
