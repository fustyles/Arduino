/* 
ESP32 (PSRAM) + I2S AUDIO OUTPUT

Author : ChungYi Fu (Kaohsiung, Taiwan)  2025-8-12 02:00
https://www.facebook.com/francefu

Development Environment
Arduino IDE 1.8.19
Arduino core for the ESP32 1.0.6

Arduino IDE Settings:
ESP32 Wrover Module
Huge APP (3MB No OTA/1MB SPIFFS)

ESP32-audioI2S v2.0.0
https://github.com/schreibfaul1/ESP32-audioI2S/releases/tag/2.0.6

//Fix these two sections of code in Audio.cpp

if(InBuff.bufferFilled() < maxFrameSize && f_stream && !f_webFileDataComplete){
    static uint8_t cnt_slow = 0;
    cnt_slow ++;
    if(f_tmr_1s) {
        if(cnt_slow > 25 && audio_info) audio_info("slow stream, dropouts are possible");
        f_tmr_1s = false;
        cnt_slow = 0;
        m_f_running = false;
        return;
    }
}

if(f_stream && !availableBytes && !f_webFileAudioComplete){
    loopCnt++;
    if(loopCnt > 200000) {              // wait several seconds
        loopCnt = 0;
        if(audio_info) audio_info("Stream lost");
        m_f_running = false;
        return;
    }
}
*/

#include <WiFi.h>
#include "Audio.h"

// WiFi credentials
char wifi_ssid[] = "teacher";       // Your WiFi SSID
char wifi_pass[] = "12345678"; // Your WiFi Password

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
    String ttsUrl = "https://translate.google.com/translate_tts?ie=UTF-8&q=" 
                    + urlencode(String(count)) +
                    "&tl=zh-TW&client=tw-ob";    

    // Start playing audio from the given URL
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
  String encoded = "";
  char c;
  char code0, code1;
  
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    
    if (isalnum(c)) {
      // Keep alphanumeric characters as-is
      encoded += c;
    } else {
      // Convert special characters to %xx format
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) code1 = (c & 0xf) - 10 + 'A';
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) code0 = c - 10 + 'A';
      encoded += '%';
      encoded += code0;
      encoded += code1;
    }
  }
  return encoded;
}
