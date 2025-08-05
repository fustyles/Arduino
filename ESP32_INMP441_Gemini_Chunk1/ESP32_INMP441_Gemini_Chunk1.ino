/* 
ESP32 (PSRAM) + INMP441 I2S microphone + Gemini Audio understanding

The ESP32 (PSRAM) is connected to an INMP441 I2S microphone to record audio and upload it to Gemini for understanding the audio content.
The maximum recording time is 20 seconds, but you can control the recording time using the buttons.

Author : ChungYi Fu (Kaohsiung, Taiwan)  2025-8-4 00:20
https://www.facebook.com/francefu

Development Environment
Arduino IDE 1.8.19
Arduino core for the ESP32 1.0.6

Arduino IDE Settings:
ESP32 Wrover Module
Huge APP (3MB No OTA/1MB SPIFFS)

INMP441 I2S:  
SD --> IO13, VDD --> 3V3, GND --> GND, L/R --> GND, WS --> IO15, SCK --> IO2

Button --> IO12

Gemini API Key
https://aistudio.google.com/app/apikey

Gemini Audio understanding
https://ai.google.dev/gemini-api/docs/audio
*/

#include <esp_heap_caps.h>
#include "driver/i2s.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "Base64.h"
#include <ArduinoJson.h>

char wifi_ssid[] = "xxxxx";
char wifi_pass[] = "xxxxx";

String geminiKey = "xxxxx";
//String geminiPrompt = "Audio to Text.";
//String geminiPrompt = "First, convert the audio into text. Based on the text content, determine whether it is related to controlling devices, and respond with JSON data without using Markdown syntax: {\"text\":\"transcribed text content\", \"devices\": [{\"servoAngle\": servo motor control angle value (use the number -1 if unrelated. The maximum servo angle is the number 180, and the minimum is the number 0.)}], \"response\":\"chat response based on the audio content\"}";
String geminiPrompt = "請先將音訊轉成繁體中文文字，根據文字內容判斷是否與控制裝置有關，並以JSON格式資料但不加上Markdown語法回覆: {\"text\":\"音訊轉文字內容\", \"devices\": [{\"servoAngle\":伺服馬達控制的角度值 (若無關則填數字-1。伺服馬達角度最大值為數字180, 最小值為數字0。)}], \"response\":\"依音訊內容聊天回應\"}";

int pinButton = 12;

#define I2S_WS            15
#define I2S_SD            13
#define I2S_SCK           2

#define SAMPLE_RATE       16000
#define SAMPLE_BITS       16
#define CHUNK_SIZE        1024
#define MAX_RECORD_TIME   20    // Maximum recording time in seconds. You can interrupt the recording by pressing the button again. 
int MAX_BUFFER_SIZE = SAMPLE_RATE * MAX_RECORD_TIME * SAMPLE_BITS / 8;

uint8_t* audioData;
uint8_t* wavData;
size_t wavSize = 0;

void initWiFi() {
  for (int i = 0; i < 2; i++) {
    WiFi.begin(wifi_ssid, wifi_pass);
    delay(1000);
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    long int StartTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if ((StartTime + 5000) < millis()) break;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("STAIP address: ");
      Serial.println(WiFi.localIP());
      break;
    }
  }
}

void initI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 1024,
    .use_apll = true
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);  
}

void writeWavHeader(uint8_t* buffer, uint32_t dataSize) {
  uint32_t chunkSize = dataSize + 36;
  uint32_t sampleRate = SAMPLE_RATE;
  uint16_t numChannels = 1;
  uint16_t bitsPerSample = SAMPLE_BITS;
  uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
  uint16_t blockAlign = numChannels * (bitsPerSample / 8);

  memcpy(buffer, "RIFF", 4);
  memcpy(buffer + 4, &chunkSize, 4);
  memcpy(buffer + 8, "WAVEfmt ", 8);
  uint32_t subchunk1Size = 16;
  uint16_t audioFormat = 1;
  memcpy(buffer + 16, &subchunk1Size, 4);
  memcpy(buffer + 20, &audioFormat, 2);
  memcpy(buffer + 22, &numChannels, 2);
  memcpy(buffer + 24, &sampleRate, 4);
  memcpy(buffer + 28, &byteRate, 4);
  memcpy(buffer + 32, &blockAlign, 2);
  memcpy(buffer + 34, &bitsPerSample, 2);
  memcpy(buffer + 36, "data", 4);
  memcpy(buffer + 40, &dataSize, 4);
}

size_t getWavDataLength() {
    const size_t chunkSize = 960;
    size_t count = 0;
    for (size_t i = 0; i < wavSize; i += chunkSize) {
      size_t len = (wavSize - i > chunkSize) ? chunkSize : (wavSize - i);
      char* encodedChunk = (char*)malloc(base64_enc_len(len) + 1);
      if (!encodedChunk) {
        return 0;
      }
      base64_encode(encodedChunk, (char*)(wavData + i), len);
      encodedChunk[base64_enc_len(len)] = '\0';
      count+=strlen(encodedChunk);
      free(encodedChunk);
    }
    return count;
}

String uploadWavDataToGemini(String apikey, String prompt) {
  prompt.replace("\"", "\\\"");
  String requestHead = "{\"contents\": [{\"role\": \"user\", \"parts\": [{\"inline_data\": {\"data\": \"";
  String requestTail = "\", \"mime_type\": \"audio/wav\"}}, {\"text\": \""+prompt+"\"}]}]}";
    
  size_t wavEncodedLength = getWavDataLength();
  size_t contentLength = requestHead.length() + wavEncodedLength + requestTail.length();

  WiFiClientSecure client;  
  client.setInsecure();
  Serial.println("Connect to generativelanguage.googleapis.com");
  if (client.connect("generativelanguage.googleapis.com", 443)) {
    Serial.println("Connection successful");
    client.println("POST /v1beta/models/gemini-2.0-flash:generateContent?key=" + apikey + " HTTP/1.1");
    client.println("Connection: close");
    client.println("Host: generativelanguage.googleapis.com");
    client.println("Content-Type: application/json; charset=utf-8");
    client.println("Content-Length: " + String(contentLength));
    client.println();
    for (int i = 0; i < requestHead.length(); i += 1024) {
      client.print(requestHead.substring(i, i + 1024));
    }

    const size_t chunkSize = 960;
    size_t count = 0;
    for (size_t i = 0; i < wavSize; i += chunkSize) {
      size_t len = (wavSize - i > chunkSize) ? chunkSize : (wavSize - i);
    
      char* encodedChunk = (char*)malloc(base64_enc_len(len) + 1);
      if (!encodedChunk) {
        client.stop();
        return "Memory allocation failed (encodedChunk).";
      }
    
      base64_encode(encodedChunk, (char*)(wavData + i), len);
      encodedChunk[base64_enc_len(len)] = '\0';
    
      client.write((const uint8_t*)encodedChunk, strlen(encodedChunk));

      free(encodedChunk);
    }
  
    for (int i = 0; i < requestTail.length(); i += 1024) {
      client.print(requestTail.substring(i, i + 1024));    
    }        
    
    String getResponse="",Feedback="";
    int waitTime = 20000;
    long startTime = millis();
    boolean state = false;
    boolean headState = false;
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);
      while (client.available()) {
          char c = client.read();
          if (state==true) Feedback += String(c);
          if (c == '\n') {
            if (getResponse.length()==0) {
              if (!headState) {
                client.readStringUntil('\n');
                headState = true;
              }
              state=true;
            }
            getResponse = "";
          }
          else if (c != '\r')
            getResponse += String(c);
          startTime = millis();
       }
       if (Feedback.length()>0) break;
    }
    client.stop(); 
    Serial.println();
    
    JsonObject obj;
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, Feedback);
    obj = doc.as<JsonObject>();
    String getText = obj["candidates"][0]["content"]["parts"][0]["text"].as<String>();
    if (getText == "null")
      getText = obj["error"]["message"].as<String>();
    getText.replace("\n", "");
    getText.replace("```json", "");
    getText.replace("```", "");
    
    return getText;
  }
  else {  
    return "Connected to generativelanguage.googleapis.com failed.";
  }
}

void setup() {
  Serial.begin(115200);

  initWiFi();
  
  initI2S(); 

  pinMode(pinButton, INPUT_PULLUP); 
}

void loop() {
  static bool isRecording = false;
  static unsigned long startMillis = 0;

  if (digitalRead(pinButton) == 1 && !isRecording) {
    delay(1000);

    Serial.println("Start recording...");
    isRecording = true;

    if (!psramFound()) {
      audioData = (uint8_t*)calloc(MAX_BUFFER_SIZE, 1);
    } else {
      audioData = (uint8_t*)heap_caps_calloc(MAX_BUFFER_SIZE, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }

    if (audioData == NULL) {
      Serial.println("Failed to allocate memory for audioData.");
      return;
    }

    startMillis = millis();
    size_t totalBytesRead = 0;

    while (isRecording) {
      size_t bytesToRead = (MAX_BUFFER_SIZE - totalBytesRead > CHUNK_SIZE) ? CHUNK_SIZE : (MAX_BUFFER_SIZE - totalBytesRead);
      size_t bytesRead = 0;

      i2s_read(I2S_NUM_0, audioData + totalBytesRead, bytesToRead, &bytesRead, portMAX_DELAY);
      totalBytesRead += bytesRead;

      if (millis() - startMillis > MAX_RECORD_TIME * 1000 || digitalRead(pinButton) == 1) {
        Serial.println("Recording stopped.");
        isRecording = false;
      }

      if (totalBytesRead >= MAX_BUFFER_SIZE) {
        Serial.println("Reached max buffer size.");
        isRecording = false;
      }
    }

    if (!psramFound()) {
      wavData = (uint8_t*)malloc(44 + totalBytesRead);
    } else {
      wavData = (uint8_t*)heap_caps_malloc(44 + totalBytesRead, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    }

    if (wavData == NULL) {
      Serial.println("Failed to allocate wavData.");
      free(audioData);
      return;
    }

    writeWavHeader(wavData, totalBytesRead);
    memcpy(wavData + 44, audioData, totalBytesRead);
    wavSize = totalBytesRead+44;

    free(audioData);
    Serial.println("WAV data ready.");
    
    //delay(1000);

    String response = uploadWavDataToGemini(geminiKey, geminiPrompt);
    Serial.println(response);

    free(wavData);     
  }
}
