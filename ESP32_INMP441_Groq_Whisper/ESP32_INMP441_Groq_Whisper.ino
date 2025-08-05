/* 
ESP32 (PSRAM) + INMP441 I2S microphone + Groq Whisper(STT)

Author : ChungYi Fu (Kaohsiung, Taiwan)  2025-8-3 20:30
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

Groq API Key
https://console.groq.com/keys
*/

#include <esp_heap_caps.h>
#include "driver/i2s.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

char wifi_ssid[] = "xxxxx";
char wifi_pass[] = "xxxx";

String groqKey = "xxxx";

int pinButton = 12;

#define I2S_WS            15
#define I2S_SD            13
#define I2S_SCK           2

#define SAMPLE_RATE       16000
#define SAMPLE_BITS       16

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

String uploadWavDataToGroqWhisper(String apikey, int seconds) {
  uint8_t* audioData; 
  int BUFFER_BYTE_SIZE = SAMPLE_RATE * seconds * SAMPLE_BITS / 8;
         
  if (!psramFound()) {
    Serial.println("PSRAM not found, fallback to normal calloc for audioData.");
    audioData = (uint8_t*)calloc(BUFFER_BYTE_SIZE, 1);
  } else {
    Serial.println("audioData allocated in PSRAM.");    
    audioData = (uint8_t*)heap_caps_calloc(BUFFER_BYTE_SIZE, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  }

  if (audioData == NULL) {
    Serial.println("audioData allocation failed!");
    Serial.printf("Remaining internal heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Remaining PSRAM heap: %d bytes\n", ESP.getFreePsram());
    return "audioData allocation failed!";
  } 
  
  Serial.println("Start recording...");
  size_t bytes_read = 0;

  esp_err_t result = i2s_read(I2S_NUM_0, audioData, BUFFER_BYTE_SIZE, &bytes_read, portMAX_DELAY);
  if (result != ESP_OK) {
    Serial.println("i2s_read failed!");
    free(audioData);
    return "i2s_read failed!";
  }

  if (bytes_read == 0) {
    Serial.println("The recording length is abnormal, skipping WAV production.");
    free(audioData);
    return "The recording length is abnormal, skipping WAV production.";
  }

  if (bytes_read < 512) {
    Serial.println("The recording is too short and may not be readable.");
  }

  uint8_t* wavData;
  if (!psramFound()) {
    Serial.println("PSRAM not found, fallback to normal malloc for wavData.");
    wavData = (uint8_t*)malloc(44 + bytes_read);
  } else {
    Serial.println("wavData allocated in PSRAM.");    
    wavData = (uint8_t*)heap_caps_malloc(44 + bytes_read, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  }
  if (wavData == NULL) {
    Serial.println("wavData alloc failed!");
    free(audioData);
    return "wavData alloc failed!";
  }

  writeWavHeader(wavData, bytes_read);
  memcpy(wavData + 44, audioData, bytes_read);
  size_t wavSize = 44 + bytes_read;
    
  free(audioData);

  String requestHead = "--Taiwan\r\nContent-Disposition: form-data; name=\"model\"\r\n\r\nwhisper-large-v3-turbo\r\n--Taiwan\r\nContent-Disposition: form-data; name=\"response_format\"\r\n\r\nverbose_json\r\n--Taiwan\r\nContent-Disposition: form-data; name=\"file\"; filename=\"user.wav\"\r\nContent-Type: audio/wav\r\n\r\n";
  String requestTail = "\r\n--Taiwan--\r\n";
  size_t totalLen = requestHead.length() + wavSize + requestTail.length();

  WiFiClientSecure client;  
  client.setInsecure();
  
  Serial.println("Connect to api.groq.com");
  if (client.connect("api.groq.com", 443)) {
    Serial.println("Connection successful");
    client.println("POST /openai/v1/audio/transcriptions HTTP/1.1");
    client.println("Connection: close");
    client.println("Host: api.groq.com");
    client.println("Authorization: Bearer " + apikey);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=Taiwan");
    client.println();
    client.print(requestHead);

    for (size_t n = 0; n < wavSize; n += 1024) {
      size_t len = (n + 1024 < wavSize) ? 1024 : (wavSize - n);
      client.write(wavData + n, len);
    }

    client.print(requestTail);
    free(wavData);
  
    String getResponse="",Feedback="";
    int waitTime = 20000;
    long startTime = millis();
    boolean state = false;
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);
      while (client.available()) {
          char c = client.read();
          if (state==true) Feedback += String(c);
          if (c == '\n') {
            if (getResponse.length()==0) state=true;
            getResponse = "";
          }
          else if (c != '\r')
            getResponse += String(c);
          startTime = millis();
       }
       if (Feedback.length()>0) break;
    }
    client.stop();
    JsonObject obj;
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, Feedback);
    obj = doc.as<JsonObject>();
    String getText = obj["text"].as<String>();
    if (getText == "null")
      getText = obj["error"]["message"].as<String>();
    getText.replace("\n", "");
    
    return getText;
  }
  else {
    return "Connected to api.groq.com failed.";
  }
}

void setup() {
  Serial.begin(115200);

  initWiFi();
  
  initI2S(); 

  pinMode(pinButton, INPUT_PULLUP); 
}

void loop() {
  if (digitalRead(pinButton)==1) {
    String response = uploadWavDataToGroqWhisper(groqKey, 5);    //Recording for 5 seconds
    Serial.println(response); 
  }
}




