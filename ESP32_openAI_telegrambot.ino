/* 
ESP32 Use ChatGPT API (GPT-3.5) and Image generator with Telegram Bot
Author : ChungYi Fu (Kaohsiung, Taiwan)  2023-5-5 20:00
https://www.facebook.com/francefu

Tutorial
https://beta.openai.com/docs/guides/completion
https://beta.openai.com/docs/api-reference/completions/create
*/

#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

char wifi_ssid[] = "teacher";
char wifi_pass[] = "87654321";

String telegrambotToken = "";
String telegrambotChatID = "";

String openaiKey = "";  // openAI API Key

//ChatGPT
String role = "You are a helpful assistant.";
String model = "gpt-3.5-turbo";
String system_content = "{\"role\": \"system\", \"content\":\""+ role +"\"}";
String historical_messages = system_content;
//Image generator  --> image:your prompt
String imageSize = "256x256"; // 256x256, 512x512 , 1024x1024

void initWiFi() {
  for (int i=0;i<2;i++) {
    WiFi.begin(wifi_ssid, wifi_pass);

    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("STAIP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");
      break;
    }
  }
}

long messageid_last = 0;
String telegrambot_getUpdates(String token) {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = "";
  JsonObject obj;
  DynamicJsonDocument doc(1024);
  String result;
  long update_id;
  String message;
  long message_id;
  String text;
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  if (client_tcp.connect(myDomain, 443)) {
    while (client_tcp.connected()) {
      getAll = "";
      getBody = "";
      String request = "limit=1&offset=-1&allowed_updates=message";
      client_tcp.println("POST /bot"+token+"/getUpdates HTTP/1.1");
      client_tcp.println("Host: " + String(myDomain));
      client_tcp.println("Content-Length: " + String(request.length()));
      client_tcp.println("Content-Type: application/x-www-form-urlencoded");
      client_tcp.println("Connection: close");
      client_tcp.println();
      client_tcp.print(request);
      int waitTime = 5000;
      long startTime = millis();
      boolean state = false;
      while ((startTime + waitTime) > millis()){
        delay(100);
        while (client_tcp.available()){
            char c = client_tcp.read();
            if (c == '\n') {
              if (getAll.length()==0) state=true;
              getAll = "";
            }
            else if (c != '\r')
              getAll += String(c);
            if (state==true) getBody += String(c);
            startTime = millis();
         }
         if (getBody.length()>0) break;
      }
      deserializeJson(doc, getBody);
      obj = doc.as<JsonObject>();
      message_id = obj["result"][0]["message"]["message_id"].as<String>().toInt();
      if (message_id!=messageid_last) {
      	 if (messageid_last!=0)
      	 	text = obj["result"][0]["message"]["text"].as<String>();
      	 messageid_last = message_id;
      }
      return text;
    }
    return "";
  }
}

String openAI_chat(String message) { 
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above

  String user_content = "{\"role\": \"user\", \"content\":\""+ message+"\"}";
  historical_messages += ", "+user_content;
  String request = "{\"model\":\""+model+"\",\"messages\":[" + historical_messages + "]}";

  if (client_tcp.connect("api.openai.com", 443)) {
    client_tcp.println("POST /v1/chat/completions HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("Authorization: Bearer " + openaiKey);
    client_tcp.println("Content-Type: application/json; charset=utf-8");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    for (int i = 0; i < request.length(); i += 1024) {
      client_tcp.print(request.substring(i, i+1024));
    }
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 20000;   // timeout 20 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) {
          char c = client_tcp.read();
          //Serial.print(String(c));
          if (state==true) 
            getResponse += String(c);
          if (c == '\n')
            Feedback = "";
          else if (c != '\r')
            Feedback += String(c);
          if (Feedback.indexOf("\",\"content\":\"")!=-1)
            state=true; 
          if (Feedback.indexOf("\"},")!=-1)
            state=false;            
          startTime = millis();
       }
       if (getResponse.length()>0) {
          client_tcp.stop();
          getResponse = getResponse.substring(0,getResponse.length()-3);
          String assistant_content = "{\"role\": \"assistant\", \"content\":\""+ getResponse+"\"}";
          historical_messages += ", "+assistant_content;
          Serial.println("");
          return getResponse;
       }
    }
    
    client_tcp.stop();
    Serial.println(Feedback);
    return "error";
  }
  else
    return "Connection failed";  
}

void openAI_chat_reset() {
  historical_messages = system_content;
}

String openAI_image(String message) { 
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above

  String request = "{\"prompt\":\""+ message+"\", \"size\":\""+imageSize+"\", \"n\":1}";
  if (client_tcp.connect("api.openai.com", 443)) {
    client_tcp.println("POST /v1/images/generations HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("Authorization: Bearer " + openaiKey);
    client_tcp.println("Content-Type: application/json; charset=utf-8");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    for (int i = 0; i < request.length(); i += 1024) {
      client_tcp.print(request.substring(i, i+1024));
    }
    
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 20000;   // timeout 20 seconds
    long startTime = millis();
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (String(c)=="\""&&state==true)
            break;           
          if (state==true)
            getResponse += String(c);
          if (c == '\n')
            Feedback = "";
          else if (c != '\r')
            Feedback += String(c);
          if (Feedback.indexOf("\"url\": \"")!=-1)
            state=true;             
          startTime = millis();
       }
       if (getResponse.length()>0) {
          client_tcp.stop();
          Serial.println("");
          return getResponse;
       }
    }
    
    client_tcp.stop();
    Serial.println(Feedback);
    return "error";
  }
  else
    return "Connection failed";  
}

void sendMessageToTelegram_custom(String token, String chatid, String text, String keyboard) {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = "";
  String request = "parse_mode=HTML&chat_id="+chatid+"&text="+text;
  if (keyboard!="") request += "&reply_markup="+keyboard;
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  if (client_tcp.connect(myDomain, 443)) {
    client_tcp.println("POST /bot"+token+"/sendMessage HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println("Connection: close");
    client_tcp.println();
    client_tcp.print(request);
    int waitTime = 5000;
    long startTime = millis();
    boolean state = false;
    while ((startTime + waitTime) > millis()) {
      delay(100);
      while (client_tcp.available())  {
          char c = client_tcp.read();
          if (state==true) getBody += String(c);
          if (c == '\n')  {
            if (getAll.length()==0) state=true;
            getAll = "";
          }
          else if (c != '\r')
            getAll += String(c);
          startTime = millis();
       }
       //Serial.println(getBody);
       if (getBody.length()>0) break;
    }
  }
}

void sendImageToTelegram_custom(String token, String chatid, String prompt, String url) {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = "";
  String request = "chat_id="+chatid+"&caption="+prompt+"&photo="+urlencode(url);
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  if (client_tcp.connect(myDomain, 443)) {
    client_tcp.println("POST /bot"+token+"/sendPhoto HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println("Connection: close");
    client_tcp.println();
    client_tcp.print(request);
    int waitTime = 5000;
    long startTime = millis();
    boolean state = false;
    while ((startTime + waitTime) > millis()) {
      delay(100);
      while (client_tcp.available())  {
          char c = client_tcp.read();
          if (state==true) getBody += String(c);
          if (c == '\n')  {
            if (getAll.length()==0) state=true;
            getAll = "";
          }
          else if (c != '\r')
            getAll += String(c);
          startTime = millis();
       }
       //Serial.println(getBody);
       if (getBody.length()>0) break;
    }
  }
}

//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str) {
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}


void setup()
{
  Serial.begin(115200);
  initWiFi();

}

void loop()
{
  String message = telegrambot_getUpdates(telegrambotToken);
  if (message != "" && message != "/start" && message != "null") {
    String response = "";
    if (message.indexOf("image:")!=-1) {
      String prompt = message.substring(6);
      response = openAI_image(prompt);
      sendImageToTelegram_custom(telegrambotToken, telegrambotChatID, prompt, response);
    }
    else {
      response = openAI_chat(message);
      sendMessageToTelegram_custom(telegrambotToken, telegrambotChatID, response, "");
    }
  }
  delay(100);
}
