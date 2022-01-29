/*
ESP32 MQTT
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-1-30 00:00
https://www.facebook.com/francefu

Library: 
https://www.arduino.cc/reference/en/libraries/pubsubclient/

Command Format :  
?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

?ip
?mac
?restart
?resetwifi=ssid;password
?inputpullup=pin
?pinmode=pin;value
?digitalwrite=pin;value
?analogwrite=pin;value
?digitalread=pin
?analogread=pin
?touchread=pin  
*/

#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "teacher";
const char* password = "87654321";

const char* mqtt_server = "broker.emqx.io";
const unsigned int mqtt_port = 1883;
#define MQTT_USER               ""
#define MQTT_PASSWORD           ""
#define MQTT_PUBLISH_TOPIC    "yourname/send"
#define MQTT_SUBSCRIBE_TOPIC    "yourname/get"
    
WiFiClient espClient;
PubSubClient client(espClient);

//自訂指令參數值
String command="";
String cmd="";
String P1="";
String P2="";
String P3="";
String P4="";
String P5="";
String P6="";
String P7="";
String P8="";
String P9="";

//自訂指令拆解狀態值
byte receiveState=0;
byte cmdState=1;
byte pState=1;
byte questionState=0;
byte equalState=0;
byte semicolonState=0;

String feedback = "";

void executeCommand() {
  //Serial.println("");
  //Serial.println("command: "+command);
  //Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
  //Serial.println("");
  
  if (cmd=="your cmd") {
    // You can do anything
    // feedback="Hello World";
  }
  else if (cmd=="ip") {
    feedback=WiFi.localIP().toString();
  }  
  else if (cmd=="mac") {
    feedback=WiFi.macAddress();
  }  
  else if (cmd=="restart") {
    ESP.restart();
  }
  else if (cmd=="resetwifi") {
    WiFi.begin(P1.c_str(), P2.c_str());
    Serial.print("Connecting to ");
    Serial.println(P1);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Serial.println("");
    Serial.println("STAIP: "+WiFi.localIP().toString());
    feedback=WiFi.localIP().toString();
  }     
  else if (cmd=="inputpullup") {
    pinMode(P1.toInt(), INPUT_PULLUP);
  }  
  else if (cmd=="pinmode") {
    if (P2.toInt()==1)
      pinMode(P1.toInt(), OUTPUT);
    else
      pinMode(P1.toInt(), INPUT);
  }        
  else if (cmd=="digitalwrite") {
    ledcDetachPin(P1.toInt());
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(), P2.toInt());
  }   
  else if (cmd=="digitalread") {
    feedback=String(digitalRead(P1.toInt()));
  }
  else if (cmd=="analogwrite") {
    ledcAttachPin(P1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,P2.toInt());
  }       
  else if (cmd=="analogread") {
    feedback=String(analogRead(P1.toInt()));
  }
  else if (cmd=="touchread") {
    feedback=String(touchRead(P1.toInt()));
  }  
  else {
    feedback="Command is not defined";
  } 

  Serial.println("feedback: "+feedback);
  Serial.println("");
  sendText(feedback); 
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());
  
  initWiFi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;
    
  Serial.print("[");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    char c = payload[i];
    getCommand(payload[i]);
    Serial.print(c);
  }
  Serial.println();
  executeCommand();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(MQTT_PUBLISH_TOPIC, "hello world");
      // ... and resubscribe
      client.subscribe(MQTT_SUBSCRIBE_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendText(String text) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(MQTT_PUBLISH_TOPIC, text.c_str());
      // ... and resubscribe
      client.subscribe(MQTT_SUBSCRIBE_TOPIC);
    } else {
      Serial.print("Publish failed");
    }
}

void initWiFi() {
  WiFi.mode(WIFI_STA);
  
  for (int i=0;i<2;i++) {
    WiFi.begin(ssid, password);
  
    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
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
  
      pinMode(2, OUTPUT);
      for (int j=0;j<5;j++) {
        digitalWrite(2,HIGH);
        delay(100);
        digitalWrite(2,LOW);
        delay(100);
      }
      break;
    }
  } 

  if (WiFi.status() != WL_CONNECTED) {    //若連線失敗
    pinMode(2, OUTPUT);
    for (int k=0;k<2;k++) {
      digitalWrite(2,HIGH);
      delay(1000);
      digitalWrite(2,LOW);
      delay(1000);
    }
  } 
}

void getCommand(char c) {
  if (c=='?') receiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) receiveState=0;
  
  if (receiveState==1) {
    command=command+String(c);
    
    if (c=='=') cmdState=0;
    if (c==';') pState++;
  
    if ((cmdState==1)&&((c!='?')||(questionState==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(pState==1)&&((c!='=')||(equalState==1))) P1=P1+String(c);
    if ((cmdState==0)&&(pState==2)&&(c!=';')) P2=P2+String(c);
    if ((cmdState==0)&&(pState==3)&&(c!=';')) P3=P3+String(c);
    if ((cmdState==0)&&(pState==4)&&(c!=';')) P4=P4+String(c);
    if ((cmdState==0)&&(pState==5)&&(c!=';')) P5=P5+String(c);
    if ((cmdState==0)&&(pState==6)&&(c!=';')) P6=P6+String(c);
    if ((cmdState==0)&&(pState==7)&&(c!=';')) P7=P7+String(c);
    if ((cmdState==0)&&(pState==8)&&(c!=';')) P8=P8+String(c);
    if ((cmdState==0)&&(pState>=9)&&((c!=';')||(semicolonState==1))) P9=P9+String(c);
    
    if (c=='?') questionState=1;
    if (c=='=') equalState=1;
    if ((pState>=9)&&(c==';')) semicolonState=1;
  }
}
