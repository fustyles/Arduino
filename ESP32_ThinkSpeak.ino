/* 
NodeMCU (ESP32)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-10-06 15:00
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
        digitalWrite(2,HIGH);
        delay(100);
        digitalWrite(2,LOW);
        delay(100);
      }
    }  

    Serial.println();
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}

void loop()
{
  String request ="",response="";
  
  //Update a Channel Feed
  int temperature = random(100);
  int humidity = random(100);
  request = "/update?api_key=UOJGVW3F98OFF5MB&field1="+String(temperature)+"&field2="+String(humidity);
  tcp("api.thingspeak.com",request,80,0);

  //Get a Channel Field
  request = "/channels/463224/feeds.json?results=1";
  response = tcp("api.thingspeak.com",request,80,1);
  //Serial.println(response);
    
  String data = getJsonData(response);
  Serial.println(data);
    
  String field1 = data.substring(data.lastIndexOf("field1,")+7,data.lastIndexOf(",field2"));
  Serial.println("field1="+field1);
  String field2 = data.substring(data.lastIndexOf("field2,")+7,data.lastIndexOf(",field3"));
  Serial.println("field2="+field2);
  String field3 = data.substring(data.lastIndexOf("field3,")+7,data.lastIndexOf(",field4"));
  Serial.println("field3="+field3);
  String field4 = data.substring(data.lastIndexOf("field4,")+7,data.lastIndexOf(",field5"));
  Serial.println("field4="+field4);
  String field5 = data.substring(data.lastIndexOf("field5,")+7,data.lastIndexOf(",field6"));
  Serial.println("field5="+field5);
  String field6 = data.substring(data.lastIndexOf("field6,")+7,data.lastIndexOf(",field7"));
  Serial.println("field6="+field6);
  String field7 = data.substring(data.lastIndexOf("field7,")+7,data.lastIndexOf(",field8"));
  Serial.println("field7="+field7);
  String field8 = data.substring(data.lastIndexOf("field8,")+7,data.length());
  Serial.println("field8="+field8);  
  Serial.println(); 
    
  delay(15000);
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
      client_tcp.stop();
      return Feedback;
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
    data_sub.replace("},{","\n");
    return data_sub;
  }
  else 
    return "";
}
