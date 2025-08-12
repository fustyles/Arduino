/* 
Gemini AIoT Voice Assistant
ESP32 (PSRAM) + INMP441 I2S microphone + MAX98357A I2S (DAC) + Gemini Audio understanding

The ESP32 (PSRAM) is connected to an INMP441 I2S microphone to record audio and upload it to Gemini for understanding the audio content.
Automatically detects sound input and starts recording. If there is no sound for three seconds, recording stops. 
Play the Gemini’s reply through a speaker by converting it to an MP3 file using Google TTS.

Author : ChungYi Fu (Kaohsiung, Taiwan)  2025-8-13 02:00
https://www.facebook.com/francefu

Development Environment
Arduino IDE 1.8.19
Arduino core for the ESP32 1.0.6

Arduino IDE Settings:
ESP32 Wrover Module
Huge APP (3MB No OTA/1MB SPIFFS)

INMP441 I2S:  (I2S_NUM_1)
SD --> IO13, VDD --> 3V3, GND --> GND, L/R --> GND, WS --> IO15, SCK --> IO2

MAX98357A I2S:  (I2S_NUM_0) 
LRC --> IO26, BCLK --> IO27, DIN --> IO25, GAIN --> GND, GND --> GND, VIN --> 5V

ESP32-audioI2S v2.0.0 (old version)
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
    if(loopCnt > 10000) {
        loopCnt = 0;
        if(audio_info) audio_info("Stream lost");
        m_f_running = false;
        return;
    }
}

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
#include "Audio.h"

// WiFi credentials
char wifi_ssid[] = "xxxxx";
char wifi_pass[] = "xxxxx";

// Gemini API Key and prompt
String geminiKey = "xxxxx";
//String geminiPrompt = "Audio to Text.";
String geminiPrompt = "First, convert the audio into text. Based on the text content, determine whether it is related to controlling devices, and respond with JSON data without using Markdown syntax: {\"text\":\"transcribed text content\", \"devices\": [{\"servoAngle\": servo motor control angle value (use the number -1 if unrelated. The maximum servo angle is the number 180, and the minimum is the number 0.)}], \"response\":\"chat response based on the audio content\"}";
//String geminiPrompt = "請先將音訊轉成繁體中文文字，根據文字內容判斷是否與控制裝置有關，並以JSON格式資料但不加上Markdown語法回覆: {\"text\":\"音訊轉文字內容\", \"devices\": [{\"servoAngle\":伺服馬達控制的角度值 (若無關則填數字-1。伺服馬達角度最大值為數字180, 最小值為數字0。)}], \"response\":\"依音訊內容聊天回應\"}";

Audio audio_play;

// I2S pins for MAX98357A connection
#define I2S_DOUT      25  // Data Out (DIN on MAX98357A)
#define I2S_BCLK      27  // Bit Clock
#define I2S_LRC       26  // Left/Right Clock (Word Select)

String speakText = "";  // Text to speech

// I2S microphone pin definitions
#define I2S_WS            15
#define I2S_SD            13
#define I2S_SCK           2

// Define recording parameters and thresholds
#define SAMPLE_RATE       16000   // Sampling rate in Hz (samples per second)
#define SAMPLE_BITS       16      // Bits per sample (audio resolution)
#define CHANNEL_NUM       1       // Mono channel input
#define MAX_RECORD_TIME   20      // Maximum recording time in seconds
#define CHUNK_SIZE        1024    // Size of each audio read chunk in bytes
#define THRESHOLD_RMS     3000    // RMS threshold to trigger recording (sound detection)
#define SILENCE_TIMEOUT   3000    // Stop recording if silence persists (in ms)
#define PREBUFFER_SECONDS 1       // Amount of audio (in seconds) to retain before sound trigger

// Calculate buffer sizes based on sampling parameters
int maxBufferSize = SAMPLE_RATE * MAX_RECORD_TIME * SAMPLE_BITS / 8;  // Total size for recording buffer (in bytes)
int preBufferSize = SAMPLE_RATE * PREBUFFER_SECONDS * SAMPLE_BITS / 8;  // Size of pre-trigger buffer (in bytes)
uint8_t* preBuffer;          // Pointer to pre-buffer memory
int preBufferOffset = 0;     // Offset to track how much data is filled in pre-buffer

// Audio buffers
uint8_t* audioData;
uint8_t* wavData;
size_t wavSize = 0;

// Recording state tracking
bool isRecording = false;           // Flag to indicate if recording is active
unsigned long startMillis = 0;      // Timestamp when recording started
unsigned long lastLoudMillis = 0;   // Timestamp of last detected loud sound
size_t totalBytesRead = 0;          // Total number of bytes recorded so far

// Initialize WiFi connection
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

// Initialize I2S microphone
void initI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // Required: Set ESP32 as I2S master and receiver to capture data from microphone
    .sample_rate = SAMPLE_RATE,                          // Required: Must match the microphone's supported sampling rate
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,        // Required: INMP441 outputs 16-bit audio samples
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,         // Required: INMP441 uses only the left audio channel for mono output
    .communication_format = I2S_COMM_FORMAT_I2S,         // Required: Standard I2S communication format used by INMP441
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,            // Optional: Interrupt priority level (level 1 is standard)
    .dma_buf_count = 4,                                  // Optional but recommended: Number of DMA buffers used for streaming
    .dma_buf_len = 1024,                                 // Optional but recommended: Length of each DMA buffer in samples
    .use_apll = true                                     // Optional: Enable APLL for higher clock precision (depends on ESP32 board)
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_1, &pin_config);  
}

// Write WAV file header to buffer
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

// Calculate the Base64 encoded length of the full WAV file
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

// Upload WAV data to Gemini and receive transcribed response
String uploadWavDataToGemini(String apikey, String prompt) {
  prompt.replace("\"", "\\\"");
  String requestHead = "{\"contents\": [{\"role\": \"user\", \"parts\": [{\"inline_data\": {\"data\": \"";
  String requestTail = "\", \"mime_type\": \"audio/wav\"}}, {\"text\": \""+prompt+"\"}]}]}";
    
  size_t wavEncodedLength = getWavDataLength();
  size_t contentLength = requestHead.length() + wavEncodedLength + requestTail.length();

  WiFiClientSecure client;  
  client.setInsecure();
  //Serial.println("Connect to generativelanguage.googleapis.com");
  if (client.connect("generativelanguage.googleapis.com", 443)) {
    //Serial.println("Connection successful");
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

// Calculate Root Mean Square (RMS) value from audio samples (used to detect loudness)
int calculateRMS(int16_t* samples, int count) {
  long sum = 0;
  for (int i = 0; i < count; i++) {
    sum += samples[i] * samples[i];
  }
  return sqrt((float)sum / count);
}

// Update the pre-buffer with recent audio data to retain last 1 second before trigger
void updatePreBuffer(uint8_t* data, size_t len) {
  // If new data is too long, just keep the last preBufferSize bytes
  if (len >= preBufferSize) {    
    memcpy(preBuffer, data + (len - preBufferSize), preBufferSize);
    preBufferOffset = preBufferSize;
    return;
  }

  // If space available, append to end
  if (preBufferOffset + len <= preBufferSize) {  
    memcpy(preBuffer + preBufferOffset, data, len);
    preBufferOffset += len;
  } else {   
    // Shift old data to make room (rolling buffer)
    int shiftSize = len;
    memmove(preBuffer, preBuffer + shiftSize, preBufferSize - shiftSize);
    memcpy(preBuffer + (preBufferSize - shiftSize), data, shiftSize);
    preBufferOffset = preBufferSize;
  } 
}


void getResponseData(String jsonResponse) {
  Serial.println(jsonResponse);
  if (jsonResponse.indexOf("{")==-1) return;
    
  JsonObject jsonObject;
  DynamicJsonDocument jsonObject_doc(2048);
  JsonArray deviceArray;
  DynamicJsonDocument deviceArray_doc(1024);
  JsonObject deviceItem;
  DynamicJsonDocument deviceItem_doc(1024);  
  
  deserializeJson(jsonObject_doc, jsonResponse);
  jsonObject = jsonObject_doc.as<JsonObject>();
  Serial.println("User: "+jsonObject["text"].as<String>());
  Serial.println("Gemini: "+jsonObject["response"].as<String>());
  speakText = jsonObject["response"].as<String>();
  deviceArray = jsonObject["devices"].as<JsonArray>();
  for (int i = 0; i <= deviceArray.size() - 1; i++) {
    deviceItem = deviceArray[i];
    if (deviceItem["servoAngle"].as<String>()) {
      Serial.println("Servo angle: "+deviceItem["servoAngle"].as<String>());
    }
  }
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

void setup() {
  Serial.begin(115200);

  initWiFi();
  
  initI2S();

  // Configure I2S pins for MAX98357A
  audio_play.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  
  // Set volume level (0–21)
  audio_play.setVolume(21);    

  // Allocate memory for pre-buffer (PSRAM preferred)
  if (!psramFound()) {
    preBuffer = (uint8_t*)malloc(preBufferSize);
  } else {
    preBuffer = (uint8_t*)heap_caps_malloc(preBufferSize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  }   
}

// Main loop: Continuously monitor audio input and trigger recording when sound is detected
void loop() {
  // Keep processing audio while it's running
  while (audio_play.isRunning()) {
    audio_play.loop(); 
  }
  
  // When not playing audio, request new Google TTS audio
  if (!audio_play.isRunning()) {
      
    if (speakText!="") {    
      String ttsUrl = "https://translate.google.com/translate_tts?ie=UTF-8&q=" 
                      + urlencode(speakText) +
                      "&tl=zh-TW&client=tw-ob";    
      speakText = "";
        
      // Clear the I2S DMA buffer to remove any residual audio data
      i2s_zero_dma_buffer(I2S_NUM_1);
      // Restart the I2S peripheral to resume audio capture
      i2s_start(I2S_NUM_1);
            
      // Start playing audio from the given URL
      audio_play.connecttohost(ttsUrl.c_str()); 
        
    } else {
      
        size_t bytesRead = 0;
        uint8_t sampleBuffer[CHUNK_SIZE];
      
        // Read a chunk of audio data from I2S
        i2s_read(I2S_NUM_1, sampleBuffer, CHUNK_SIZE, &bytesRead, portMAX_DELAY);
        // Calculate loudness of the current audio sample (RMS)
        int rms = calculateRMS((int16_t*)sampleBuffer, bytesRead / 2);
        // Continuously update pre-buffer with recent audio
        updatePreBuffer(sampleBuffer, bytesRead);
      
        // ---- Triggered Recording Start ----
        if (!isRecording) {
          if (rms > THRESHOLD_RMS) {  // Detect loud sound to trigger recording
            Serial.println("Detected sound, start recording.");
      
            // Allocate audio buffer for full recording duration
            if (!psramFound()) {
              audioData = (uint8_t*)calloc(maxBufferSize, 1);
            } else {
              audioData = (uint8_t*)heap_caps_calloc(maxBufferSize, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            }
            if (!audioData) {
              Serial.println("Failed to allocate audioData.");
              return;
            }
      
            // Copy pre-buffered audio before trigger into recording buffer
            memcpy(audioData, preBuffer, preBufferOffset);
            totalBytesRead = preBufferOffset;
      
            // Add current sample to audio buffer
            if (totalBytesRead + bytesRead <= maxBufferSize) {
              memcpy(audioData + totalBytesRead, sampleBuffer, bytesRead);
              totalBytesRead += bytesRead;
            }
      
            isRecording = true;
            startMillis = millis();         // Start timing the recording
            lastLoudMillis = millis();      // Mark latest sound detected
          }
      
        // ---- During Recording ----
        } else {
          // Add current audio chunk to recording buffer
          if (totalBytesRead + bytesRead <= maxBufferSize) {
            memcpy(audioData + totalBytesRead, sampleBuffer, bytesRead);
            totalBytesRead += bytesRead;
          }
      
          // Update last loud moment if sound continues
          if (rms > THRESHOLD_RMS) {
            lastLoudMillis = millis();
          }
      
          // Check end condition: timeout, silence, or buffer full
          bool timeoutReached = (millis() - startMillis > MAX_RECORD_TIME * 1000);
          bool silenceTooLong = (millis() - lastLoudMillis > SILENCE_TIMEOUT);
          bool bufferFull = (totalBytesRead >= maxBufferSize);
      
          if (timeoutReached || silenceTooLong || bufferFull) {
            Serial.println("Stopping recording.");
      
            // Allocate memory for WAV data
            if (!psramFound()) {
              wavData = (uint8_t*)malloc(44 + totalBytesRead);
            } else {
              wavData = (uint8_t*)heap_caps_malloc(44 + totalBytesRead, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            }
      
            // If allocation successful, finalize WAV and upload
            if (wavData) {
              writeWavHeader(wavData, totalBytesRead);  // Write WAV header
              memcpy(wavData + 44, audioData, totalBytesRead); // Append audio
              wavSize = totalBytesRead + 44;
              //Serial.printf("WAV complete, total = %d bytes (including prebuffer)\n", wavSize);
          
              String response = uploadWavDataToGemini(geminiKey, geminiPrompt);
              getResponseData(response);
      
              free(wavData);  // Free WAV buffer
            } else {
              Serial.println("Failed to allocate wavData.");
            }
      
            // Cleanup and reset state
            free(audioData);
            isRecording = false;
            totalBytesRead = 0;
            preBufferOffset = 0;
          }
        }
      
        delay(5);  // Short delay to reduce CPU load

      
    }
  }
}


