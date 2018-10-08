/* 
NodeMCU (ESP12E)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-10-08 19:00
*/

#include <ESP8266WiFi.h>     //ESP12E

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
    String IP = getExternalIP();
    Serial.println("External IP is " + IP);
    Serial.println();
    
    // IFTTT LINE
    String domain="maker.ifttt.com";
    String event="xxxxx";
    String key="xxxxxxxxxxxxxxxxxxxxxx";
    String request = "/trigger/" + event + "/with/key/" + key + "?value1=" + IP + "&value2=" + WiFi.localIP().toString();
    Serial.println(tcp(domain,request,80,0));  
    Serial.println();
  }  
  else
     Serial.println("Unable to connect!"); 
}

void loop() 
{
}

String getExternalIP()
{
  String domain="www.rus.net.tw";
  String request = "/myip.php";
  String response = tcp(domain,request,80,1);
  //Serial.println(response);
  
  int s = response.indexOf("Your IP is");
  if (s!=-1)  {
    String data_sub = response.substring(s+25,s+50);
    return data_sub.substring(0,data_sub.indexOf("<BR>"));
  }
  else 
    return "";
}

String tcp(String domain,String request,int port,byte wait)
{
    WiFiClient client_tcp;
    
    if (client_tcp.connect(domain, port)) 
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
