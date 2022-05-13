/*
ESP32 Weather of Taiwan
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-5-12 00:00
https://www.facebook.com/francefu
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

char wifi_ssid[] = "teacher";
char wifi_pass[] = "87654321";

String Weather0012[8] = {"","","","","","","",""};
String Weather1224[8] = {"","","","","","","",""};
String Weather2436[8] = {"","","","","","","",""};

void setup()
{
  Serial.begin(115200);
  initWiFi();
}

void loop()
{
  //宜蘭縣,花蓮縣,臺東縣,澎湖縣,金門縣,連江縣,臺北市,新北市,桃園市,臺中市,臺中市,臺南市,高雄市,基隆市,新竹縣,新竹市,苗栗縣,彰化縣,南投縣,雲林縣,嘉義縣,嘉義市,屏東縣
  opendataWeather("高雄市","rdec-key-123-45678-011121314");
  
  //getWeather(period, index)   period=0,1,2  index=0,1,2,3,4,5,6,7
  Serial.println("位置= "+getWeather(0, 0));
  Serial.println("開始時間= "+getWeather(0, 1));
  Serial.println("結束時間= "+getWeather(0, 2));
  Serial.println("天氣現象= "+getWeather(0, 3));
  Serial.println("降雨機率= "+getWeather(0, 4)+" %");
  Serial.println("最低氣溫= "+getWeather(0, 5)+" °C");
  Serial.println("舒適度= "+getWeather(0, 6));
  Serial.println("最高氣溫= "+getWeather(0, 7)+" °C");            
  Serial.println();
  Serial.println("位置= "+getWeather(1, 0));
  Serial.println("開始時間= "+getWeather(1, 1));
  Serial.println("結束時間= "+getWeather(1, 2));
  Serial.println("天氣現象= "+getWeather(1, 3));
  Serial.println("降雨機率= "+getWeather(1, 4)+" %");
  Serial.println("最低氣溫= "+getWeather(1, 5)+" °C");
  Serial.println("舒適度= "+getWeather(1, 6));
  Serial.println("最高氣溫= "+getWeather(1, 7)+" °C");            
  Serial.println();
  Serial.println("位置= "+getWeather(2, 0));
  Serial.println("開始時間= "+getWeather(2, 1));
  Serial.println("結束時間= "+getWeather(2, 2));
  Serial.println("天氣現象= "+getWeather(2, 3));
  Serial.println("降雨機率= "+getWeather(2, 4)+" %");
  Serial.println("最低氣溫= "+getWeather(2, 5)+" °C");
  Serial.println("舒適度= "+getWeather(2, 6));
  Serial.println("最高氣溫= "+getWeather(2, 7)+" °C");            
  Serial.println();    
  delay(60000);
}

void initWiFi() {
  WiFi.mode(WIFI_AP_STA);

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

void opendataWeather(String location, String Authorization) {
  location = urlencode(location);
  WiFiClientSecure client_tcp;
  String request = "/api/v1/rest/datastore/F-C0032-001?Authorization="+Authorization+"&locationName="+location;
  if (client_tcp.connect("opendata.cwb.gov.tw", 443)) {
    client_tcp.println("GET " + request + " HTTP/1.1");
    client_tcp.println("Host: opendata.cwb.gov.tw");
    client_tcp.println("Connection: close");
    client_tcp.println();
    String getResponse="",Feedback="";
    boolean state = false;
    int waitTime = 10000;
    long startTime = millis();
    char c;
    String temp = "";
    int i = 0;
    while ((startTime + waitTime) > millis()) {
      while (client_tcp.available()) {
        if (state==true) {
          temp = client_tcp.readStringUntil('\r');
          i++;
          if (i%2==0) {
            Feedback += temp;
          }
        }
        else
          c = client_tcp.read();
        if (c == '\n') {
          if (getResponse.length()==0) state=true;
          getResponse = "";
        }
        else if (c != '\r')
          getResponse += String(c);
        startTime = millis();
      }
      if (Feedback.length()!= 0) break;
    }
    client_tcp.stop();
    //Serial.println(Feedback);
    
    JsonObject obj;
    DynamicJsonDocument doc(4096);
    
    Feedback = Feedback.substring(Feedback.indexOf("records")+9,Feedback.length()-1);
    temp = "";
    for (i=0;i<Feedback.length();i++) {
      c = Feedback[i];
      if (c!='\r'&&c!='\n')
      temp += Feedback[i];
    }
    Feedback = temp;
    
    deserializeJson(doc, Feedback);
    obj = doc.as<JsonObject>();

    //00-12
    Weather0012[0] = obj["location"][0]["locationName"].as<String>();
    Weather0012[1] = obj["location"][0]["weatherElement"][0]["time"][0]["startTime"].as<String>();
    Weather0012[2] = obj["location"][0]["weatherElement"][0]["time"][0]["endTime"].as<String>();
    Weather0012[3] = obj["location"][0]["weatherElement"][0]["time"][0]["parameter"]["parameterName"].as<String>();
    Weather0012[4] = obj["location"][0]["weatherElement"][1]["time"][0]["parameter"]["parameterName"].as<String>();
    Weather0012[5] = obj["location"][0]["weatherElement"][2]["time"][0]["parameter"]["parameterName"].as<String>();
    Weather0012[6] = obj["location"][0]["weatherElement"][3]["time"][0]["parameter"]["parameterName"].as<String>();
    Weather0012[7] = obj["location"][0]["weatherElement"][4]["time"][0]["parameter"]["parameterName"].as<String>();
    //12-24
    Weather1224[0] = obj["location"][0]["locationName"].as<String>();
    Weather1224[1] = obj["location"][0]["weatherElement"][0]["time"][1]["startTime"].as<String>();
    Weather1224[2] = obj["location"][0]["weatherElement"][0]["time"][1]["endTime"].as<String>();   
    Weather1224[3] = obj["location"][0]["weatherElement"][0]["time"][1]["parameter"]["parameterName"].as<String>();
    Weather1224[4] = obj["location"][0]["weatherElement"][1]["time"][1]["parameter"]["parameterName"].as<String>();
    Weather1224[5] = obj["location"][0]["weatherElement"][2]["time"][1]["parameter"]["parameterName"].as<String>();
    Weather1224[6] = obj["location"][0]["weatherElement"][3]["time"][1]["parameter"]["parameterName"].as<String>();
    Weather1224[7] = obj["location"][0]["weatherElement"][4]["time"][1]["parameter"]["parameterName"].as<String>();
    //24-36
    Weather2436[0] = obj["location"][0]["locationName"].as<String>();
    Weather2436[1] = obj["location"][0]["weatherElement"][0]["time"][2]["startTime"].as<String>();   
    Weather2436[2] = obj["location"][0]["weatherElement"][0]["time"][2]["endTime"].as<String>();      
    Weather2436[3] = obj["location"][0]["weatherElement"][0]["time"][2]["parameter"]["parameterName"].as<String>();
    Weather2436[4] = obj["location"][0]["weatherElement"][1]["time"][2]["parameter"]["parameterName"].as<String>();
    Weather2436[5] = obj["location"][0]["weatherElement"][2]["time"][2]["parameter"]["parameterName"].as<String>();
    Weather2436[6] = obj["location"][0]["weatherElement"][3]["time"][2]["parameter"]["parameterName"].as<String>();
    Weather2436[7] = obj["location"][0]["weatherElement"][4]["time"][2]["parameter"]["parameterName"].as<String>();
  }
}

String getWeather(int period,int index) {   //period=0,1,2  index=0,1,2,3,4,5,6,7
  if (period==0)
    return Weather0012[index];
  else if (period==1)
    return Weather1224[index];
  else if (period==2)
    return Weather2436[index];
  return "";
}

//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str) {
    String encodedString="";
    char c;
    char code0;
    char code1;
    for (int i =0; i < str.length(); i++) {
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
        encodedString+="%";
        encodedString+=code0;
        encodedString+=code1;
      }
      yield();
    }
    return encodedString;
}
