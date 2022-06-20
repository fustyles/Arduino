/*
ESP32 Google spreadsheet query by SQL statement
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-6-20 08:00
https://www.facebook.com/francefu
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <ArduinoJson.h>
String spreadsheetQueryData = "{\"values\":[]}";

char wifi_ssid[] = "teacher";
char wifi_pass[] = "87654321";

void setup()
{
  Serial.begin(115200);
  initWiFi();

  //Spreadsheet_query(sql, spreadsheetId, spreadsheetName)
  spreadsheetQueryData = Spreadsheet_query("select B,C", "1EjRU-dWFYq3pq8uXjh_LYwEbI-hff2jQC1N0GQ6qGYw", "工作表1");   
  Serial.println(Spreadsheet_getcell(0, 0));  //B1
  Serial.println(Spreadsheet_getcell(0, 1));  //C1
  Serial.println(Spreadsheet_getcell(1, 0));  //B2
  Serial.println(Spreadsheet_getcell(1, 1));  //C2  
  Serial.println(Spreadsheet_getcell_count("row"));  //資料陣列橫列數
  Serial.println(Spreadsheet_getcell_count("col"));  //資料陣列縱行數
}

void loop()
{

}

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

String Spreadsheet_query(String sql, String mySpreadsheetid, String mySpreadsheetname) {
  sql = urlencode(sql);
  mySpreadsheetname = urlencode(mySpreadsheetname);
  const char* myDomain = "docs.google.com";
  String getAll="", getBody = "", getData = "";
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;   //run version 1.0.5 or above
  client_tcp.setInsecure();
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");
    String url = "https://docs.google.com/spreadsheets/d/"+mySpreadsheetid+"/gviz/tq?tqx=out:json&sheet="+mySpreadsheetname+"&tq="+sql;
    client_tcp.println("GET "+url+" HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Type: application/json");
    client_tcp.println();
    int waitTime = 10000;
    long startTime = millis();
    boolean state = false;
    boolean start = false;

    while ((startTime + waitTime) > millis()) {
      Serial.print(".");
      delay(100);
      while (client_tcp.available()) {
          char c = client_tcp.read();
          if (getBody.indexOf("\"rows\":[")!=-1) start = true;
          if (getData.indexOf("],")!=-1) start = false;
          if (state==true&&c!='\n'&&c!='\r') getBody += String(c);
          if (start==true&&c!='\n'&&c!='\r') getData += String(c);
          if (c == '\n') {
            if (getAll.length()==0) state=true;
            getAll = "";
          } else if (c != '\r')
            getAll += String(c);
          startTime = millis();
       }
       if (getBody.length()>0) break;
    }
    Serial.println("");
    if (getBody.indexOf("error")!=-1||getBody=="")
      return "{\"values\":[]}";
    getData = "{\"values\":[" + getData.substring(0, getData.length()-2) + "]}";
    return getData;
  } else {
    Serial.println("Connected to " + String(myDomain) + " failed.");
    return "{\"values\":[]}";
  }
}

String urlencode(String str)
{
  String encodedString="";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i =0; i < str.length(); i++){
    c=str.charAt(i);
    if (c == ' ') {
      encodedString+= '+';
    } else if (isalnum(c)){
      encodedString+=c;
    } else {
      code1=(c & 0xf)+'0';
      if ((c & 0xf) >9) {
          code1=(c & 0xf) - 10 + 'A';
      }
      c=(c>>4)&0xf;
      code0=c+'0';
      if (c > 9) {
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

String Spreadsheet_getcell(int row, int col) {
    if (spreadsheetQueryData!="") {
      JsonObject obj;
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, spreadsheetQueryData);
      obj = doc.as<JsonObject>();
      if ((obj["values"].size()<row+1)||(obj["values"][0]["c"].size()<col+1))
        return "";
      return obj["values"][row]["c"][col]["v"].as<String>();
    }
    else
      return "";
}

int Spreadsheet_getcell_count(String option) {
    if (spreadsheetQueryData!="") {
      JsonObject obj;
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, spreadsheetQueryData);
      obj = doc.as<JsonObject>();
      if (option=="row")
        return obj["values"].size();
      else if (option=="col")
        return obj["values"][0]["c"].size();
    }
    return 0;
}
