//https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/Preferences.h
#include <Preferences.h>
Preferences preferences;

String wifi_ssid;
String wifi_password;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Preferences_write("test", "12345678");
  
  Preferences_read();
  Serial.println();
  Serial.printf("wifi_ssid = %s\n", wifi_ssid);
  Serial.printf("wifi_password = %s\n", wifi_password);      
}

void loop() {}

void Preferences_write(const char* myssid, const char* mypassword) {
  preferences.clear();
  preferences.begin("wifi", false);
  Serial.printf("Put ssid = %s\n", myssid);
  preferences.putString("ssid", myssid);
  Serial.printf("Put password = %s\n", mypassword);
  preferences.putString("password", mypassword);  
  preferences.end();
}

void Preferences_read() {
  preferences.begin("wifi", false);
  wifi_ssid = preferences.getString("ssid", "");
  Serial.printf("Get ssid = %s\n", wifi_ssid);
  wifi_password = preferences.getString("password", "");
  Serial.printf("Get password = %s\n", wifi_password); 
  preferences.end();
}
