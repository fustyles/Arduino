/* 
NodeMCU (ESP12E)
Author : ChungYi Fu (Kaohsiung, Taiwan)  2018-10-06 19:30
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
        
      Serial.println();
      Serial.println("STAIP address: ");
      Serial.println(WiFi.localIP());
      Serial.println();     
    }  
    else
       Serial.println("Unable to connect!"); 
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
    
  String data = transJsonData(response);
  Serial.println(data);
    
  String data_tmp="";
  if (data.lastIndexOf("field1,")!=-1) {
    data_tmp = data.substring(data.lastIndexOf("field1,")+7);
    String field1 = data_tmp.substring(0,data_tmp.indexOf(","));
    Serial.println("field1="+field1);
  }
  if (data.lastIndexOf("field2,")!=-1) {
    data_tmp = data.substring(data.lastIndexOf("field2,")+7);
    String field2 = data_tmp.substring(0,data_tmp.indexOf(","));
    Serial.println("field2="+field2);
  }
  if (data.lastIndexOf("field3,")!=-1) {
    data_tmp = data.substring(data.lastIndexOf("field3,")+7);
    String field3 = data_tmp.substring(0,data_tmp.indexOf(","));
    Serial.println("field3="+field3);
  }
  if (data.lastIndexOf("field4,")!=-1) {
    data_tmp = data.substring(data.lastIndexOf("field4,")+7);
    String field4 = data_tmp.substring(0,data_tmp.indexOf(","));
    Serial.println("field4="+field4);
  }
  if (data.lastIndexOf("field5,")!=-1) {
    data_tmp = data.substring(data.lastIndexOf("field5,")+7);
    String field5 = data_tmp.substring(0,data_tmp.indexOf(","));
    Serial.println("field5="+field5);
  }
  if (data.lastIndexOf("field6,")!=-1) {
    data_tmp = data.substring(data.lastIndexOf("field6,")+7);
    String field6 = data_tmp.substring(0,data_tmp.indexOf(","));
    Serial.println("field6="+field6);
  }
  if (data.lastIndexOf("field7,")!=-1) {
    data_tmp = data.substring(data.lastIndexOf("field7,")+7);
    String field7 = data_tmp.substring(0,data_tmp.indexOf(","));
    Serial.println("field7="+field7);
  }
  if (data.lastIndexOf("field8,")!=-1) {
    data_tmp = data.substring(data.lastIndexOf("field8,")+7);
    String field8 = data_tmp.substring(0,data_tmp.indexOf(","));
    Serial.println("field8="+field8); 
  }
  Serial.println();
    
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

String transJsonData(String data)
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
