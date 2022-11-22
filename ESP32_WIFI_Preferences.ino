//https://github.com/espressif/arduino-esp32/blob/master/libraries/Preferences/src/Preferences.h
#include <Preferences.h>
Preferences preferences;

String wifi_ssid = "";
String wifi_password = "";

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  Preferences_write("wifi", "ssid", "test");
  Preferences_write("wifi", "password", "12345678");

  wifi_ssid = Preferences_read("wifi", "ssid");
  wifi_password = Preferences_read("wifi", "password");
  Serial.printf("ssid = %s\n", wifi_ssid);  
  Serial.printf("password = %s\n", wifi_password);  
}

void loop() {
}

void Preferences_write(const char * name, const char* key, const char* value) {
  //preferences.clear();
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
