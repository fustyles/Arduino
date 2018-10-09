/* 
NodeMCU (ESP32)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-10-09 21:00
*/

#include <WiFi.h>     //ESP32

const char* ssid     = "xxxxx";   //your network SSID
const char* password = "xxxxx";   //your network password

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  delay(1000);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.println();
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+10000) < millis()) break;
  } 

  if (WiFi.localIP().toString()!="0.0.0.0")
  {   
    // IFTTT - Webhooks + Google Sheet
    String cmd = "on";
    //String cmd = "Hello\%20World";
    Serial.println("Write Data = " + cmd);
    String domain="maker.ifttt.com";
    String event="spreadsheet";
    String key="dwGDpfZMocgp179E5zpZwu";
    String request = "/trigger/" + event + "/with/key/" + key + "?value1=" + cmd;
    Serial.println(tcp(domain,request,80,0));  
    Serial.println();

    delay(5000);

    // Get data from Google Sheet
    domain="spreadsheets.google.com";
    key="1bG9wDbyY0Rx1D3ayZN-vwq8ijCc3qOyTdPi92fEs8wQ";
    request = "/feeds/cells/" + key + "/1/public/values?alt=json-in-script&callback=doData";
    String response = tcp(domain,request,80,1);  
    //Serial.println(response);
    String A1 = getGoogleSheetField(response,1,1);
    Serial.println("Get Data = " + A1);
  }  
  else
     Serial.println("Unable to connect!"); 
}

void loop() 
{
}

String getGoogleSheetField(String data, int row, int col)
{
  String s = "\"row\":\"" + String(row) + "\",\"col\":\"" + String(col) + "\"";
  String data_sub = data.substring(data.indexOf(s));
  s = "\$t";
  data_sub = data_sub.substring(data_sub.indexOf(s)+5);
  s = "\"";
  data_sub = data_sub.substring(0,data_sub.indexOf(s));
  return data_sub;
}

String tcp(String domain,String request,int port,byte wait)
{
    WiFiClient client_tcp;
    
    if (client_tcp.connect(domain.c_str(), port)) 
    {
      Serial.println("GET " + request);
      client_tcp.println("GET " + request + " HTTP/1.1");
      client_tcp.println("Host: " + domain);
      client_tcp.println("Connection: close");
      client_tcp.println();

      String getResponse="",Feedback="";
      boolean state = false;
      int waitTime = 3000;   // timeout 3 seconds
      long startTime = millis();
      while ((startTime + waitTime) > millis())
      {
        while (client_tcp.available()) 
        {
            char c = client_tcp.read();
            if (c == '\n') 
            {
              if (getResponse.length()==0) state=true; 
              getResponse = "";
            } 
            else if (c != '\r')
              getResponse += String(c);
            if (state==true) Feedback += String(c);
            if (wait==1)
              startTime = millis();
         }
         if (wait==0)
          if ((state==true)&&(Feedback.length()!= 0)) break;
      }
      client_tcp.stop();
      return Feedback;
    }
    else
      return "Connection failed";  
}
