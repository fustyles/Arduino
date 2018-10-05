/* 
NodeMCU (ESP12E)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-10-05 22:00
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
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+10000) < millis()) break;
    } 
  
    if (WiFi.localIP().toString()!="0.0.0.0")
    {
      pinMode(2, OUTPUT);
      for (int i=0;i<5;i++)
      {
        digitalWrite(2,LOW);
        delay(100);
        digitalWrite(2,HIGH);
        delay(100);
      }
    }  

    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("");
}

void loop()
{
  String request ="",response="",data="",field1="",field2="";
  
  //Update a Channel Feed
  int temperature = random(100);
  int humidity = random(100);
  request = "/update?api_key=UOJGVW3F98OFF5MB&field1="+String(temperature)+"&field2="+String(humidity);
  tcp("api.thingspeak.com",request,80,0);

  //Get a Channel Field
  request = "/channels/463224/feeds.json?results=1";
  response = tcp("api.thingspeak.com",request,80,1);
  //Serial.println(response);
    
  data = getJsonData(response);
  Serial.println(data);
    
  field1 = data.substring(data.indexOf("field1")+7,data.indexOf(",field2"));
  Serial.println("field1="+field1);
    
  field2 = data.substring(data.indexOf("field2")+7,data.indexOf(",field3"));
  Serial.println("field2="+field2);
    
  delay(15000);
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
      long StartTime = millis();
      while ((StartTime+4000) > millis())
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
         }
         if (wait==0)
          if ((state==true)&&(Feedback.length()!= 0)) break;
      }
      return Feedback;
      client_tcp.stop();
    }
    else
      return "Connection failed";  
}

String getJsonData(String data)
{
  int s = data.indexOf("feeds");
  int e = data.indexOf("}]}");
  if ((s!=-1)&&(e!=-1))  {
    String data_sub = data.substring(s+9,e);
    data_sub.replace("\":","\",");
    data_sub.replace("\"","");
    data_sub.replace("},{",",");
    return data_sub;
  }
  else 
    return "";
}
