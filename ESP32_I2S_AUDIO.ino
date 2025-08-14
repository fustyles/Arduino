/* 
ESP32 (PSRAM) + I2S AUDIO OUTPUT

Author : ChungYi Fu (Kaohsiung, Taiwan)  2025-8-14 13:00
https://www.facebook.com/francefu

Development Environment
Arduino IDE 1.8.19
Arduino core for the ESP32 1.0.6

Arduino IDE Settings:
ESP32 Wrover Module
Huge APP (3MB No OTA/1MB SPIFFS)

MAX98357A I2S: 
LRC --> IO26, BCLK --> IO27, DIN --> IO25, GAIN --> GND, GND --> GND, VIN --> 5V

ESP32-audioI2S v2.0.0
https://github.com/fustyles/Arduino/blob/master/ESP32_INMP441_MAX98357A_Gemini/ESP32-audioI2S-fix.zip
*/

#include <WiFi.h>
#include "Audio.h"

// WiFi credentials
char wifi_ssid[] = "xxxxx";       // Your WiFi SSID
char wifi_pass[] = "xxxxx"; // Your WiFi Password

// Google TTS Language Codes: https://developers.google.com/workspace/admin/directory/v1/languages
String googlettsLanguageCode = "zh-TW"; 

Audio audio_play;

// I2S pins for MAX98357A connection
#define I2S_DOUT      25  // Data Out (DIN on MAX98357A)
#define I2S_BCLK      27  // Bit Clock
#define I2S_LRC       26  // Left/Right Clock (Word Select)

// Counter for test text-to-speech
int count = 100;

// Initialize WiFi connection
void initWiFi() {
  for (int i = 0; i < 2; i++) { // Try connecting twice
    WiFi.begin(wifi_ssid, wifi_pass);
    delay(1000);
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    long int StartTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      // Timeout after 5 seconds
      if ((StartTime + 5000) < millis()) break;
    }

    // If connected, print IP address and exit loop
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("STAIP address: ");
      Serial.println(WiFi.localIP());
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);

  initWiFi();

  // Configure I2S pins for MAX98357A
  audio_play.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  
  // Set volume level (0–21)
  audio_play.setVolume(21);
}

void loop() {
  // Keep processing audio while it's running
  while (audio_play.isRunning()) {
    audio_play.loop(); 
  }
  
  // When not playing audio, request new Google TTS audio
  if (!audio_play.isRunning()) {
    count++;
    String ttsUrl = "https://translate.google.com/translate_tts?ie=UTF-8&q=" + urlencode(String(count)) + "&tl=" + googlettsLanguageCode + "&client=tw-ob";    

    // Start playing audio from the given URL
    if (!audio_play.connecttohost(ttsUrl.c_str()))
      audio_play.connecttohost(ttsUrl.c_str());  
  }
}

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}

// Function to URL-encode a string for HTTP requests
String urlencode(String str) {
  const char *msg = str.c_str();
  const char *hex = "0123456789ABCDEF";
  String encodedMsg = "";
  while (*msg != '\0') {
    if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9') || *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~') {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[(unsigned char)*msg >> 4];
      encodedMsg += hex[*msg & 0xf];
    }
    msg++;
  }
  return encodedMsg;
}

