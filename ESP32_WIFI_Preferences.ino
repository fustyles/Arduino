//https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/Preferences.h
#include <Preferences.h>
Preferences preferences;

String wifi_ssid;
String wifi_password;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Preferences_wifi_write("myssid", "12345678");
  Preferences_wifi_read();
  Serial.println();
  Serial.printf("wifi_ssid = %s\n", wifi_ssid);
  Serial.printf("wifi_password = %s\n\n", wifi_password);
  
  Preferences_write("hello", "world", "peace");
  Serial.printf("\nmyData = %s", Preferences_read("hello", "world"));
}

void loop() {}

void Preferences_write(const char * name, const char* key, const char* value) {
  preferences.clear();
  preferences.begin(name, false);
  Serial.printf("Put %s = %s\n", key, value);
  preferences.putString(key, value);
  preferences.end();
}

String Preferences_read(const char * name, const char* key) {
  preferences.begin(name, false);
  String myData = preferences.getString(key, "");
  Serial.printf("Get %s = %s\n", key, myData);
  preferences.end();
  return myData;
}

void Preferences_wifi_write(const char* myssid, const char* mypassword) {
  preferences.clear();
  preferences.begin("wifi", false);
  Serial.printf("Put ssid = %s\n", myssid);
  preferences.putString("ssid", myssid);
  Serial.printf("Put password = %s\n", mypassword);
  preferences.putString("password", mypassword);  
  preferences.end();
}

void Preferences_wifi_read() {
  preferences.begin("wifi", false);
  wifi_ssid = preferences.getString("ssid", "");
  Serial.printf("Get ssid = %s\n", wifi_ssid);
  wifi_password = preferences.getString("password", "");
  Serial.printf("Get password = %s\n", wifi_password); 
  preferences.end();
}
