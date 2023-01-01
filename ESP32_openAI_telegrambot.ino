//Generated Date: Sun, 01 Jan 2023 06:03:35 GMT

#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

char wifi_ssid[] = "teacher";
char wifi_pass[] = "87654321";
String telegrambotToken = "";
String telegrambotChatID = "";
String openaiKey = "";
int max_tokens = 256;

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

String openAI_text(String token, int max_tokens, String words) {
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();
  words = "{\"model\":\"text-davinci-003\",\"prompt\":\"" + words + "\",\"temperature\":0,\"max_tokens\":" + String(max_tokens) + ",\"frequency_penalty\":0,\"presence_penalty\":0.6,\"top_p\":1.0,\"n\":1}";
  if (client_tcp.connect("api.openai.com", 443)) {
    client_tcp.println("POST /v1/completions HTTP/1.1");
    client_tcp.println("Connection: close");
    client_tcp.println("Host: api.openai.com");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Type: application/json; charset=utf-8");
    client_tcp.println("Content-Length: " + String(words.length()));
    client_tcp.println();
    client_tcp.println(words);
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 60000;
    long startTime = millis();
    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (state==true) {
            Feedback += String(c);
            if (Feedback.indexOf("\"text\":\"\\n\\n")!=-1)
               Feedback = "";
            if (Feedback.indexOf("\"text\":\"?\\n\\n")!=-1)
               Feedback = "";
            if (Feedback.indexOf("\",\"index\"")!=-1) {
              client_tcp.stop();
              Serial.println();
              return Feedback.substring(0,Feedback.length()-9);
            }
            if (Feedback.indexOf("\"}}")!=-1) {
              client_tcp.stop();
              Serial.println();
              return Feedback.substring(0,Feedback.length()-3);
            }
          }
          if (c == '\n') {
            if (getResponse.length()==0) state=true;
            getResponse = "";
          }
          else if (c != '\r')
            getResponse += String(c);
          startTime = millis();
       }
       if (getResponse.length()>0) break;
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
       Serial.println(getBody);
       if (getBody.length()>0) break;
    }
  }
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
    Serial.println(message);
    String response = (openAI_text(openaiKey, max_tokens, message));
    sendMessageToTelegram_custom(telegrambotToken,telegrambotChatID,response,"");
  }
  delay(100);
}
