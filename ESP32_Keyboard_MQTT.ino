/*
ESp32 Keyboard by using MQTT
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-10-27 00:00
https://www.facebook.com/francefu

Library: 
https://github.com/T-vK/ESp32-BLE-Keyboard

Keyboard Modifiers (keyboardpress)
https://www.arduino.cc/en/Reference/KeyboardModifiers

Command Format :  
http://APIP/?cmd=p1;p2;p3;p4;p5;p6;p7;p8;p9
http://STAIP/?cmd=p1;p2;p3;p4;p5;p6;p7;p8;p9

Default APIP： 192.168.4.1

http://192.168.xxx.xxx/?ip
http://192.168.xxx.xxx/?mac
http://192.168.xxx.xxx/?restart
http://192.168.xxx.xxx/?resetwifi=ssid;password
http://192.168.xxx.xxx/?inputpullup=pin
http://192.168.xxx.xxx/?pinmode=pin;value
http://192.168.xxx.xxx/?digitalwrite=pin;value
http://192.168.xxx.xxx/?analogwrite=pin;value
http://192.168.xxx.xxx/?digitalread=pin
http://192.168.xxx.xxx/?analogread=pin
http://192.168.xxx.xxx/?touchread=pin  
http://192.168.xxx.xxx/?keyboardpress=keycode1;keycode2;keycode3;presstime
http://192.168.xxx.xxx/?keyboardprint=characters
http://192.168.xxx.xxx/?keyboardwrite=keycode
 
Remote Control for PPT  (keyboard press = keycode1, keycode2, keycode3, pressTime)
?keyboardwrite=198              "F5"
?keyboardwrite=211              "PAGE UP"
?keyboardwrite=133;198;;10      "SHIFT+F5"      
?keyboardwrite=214              "PAGE DOWN"
?keyboardwrite=87               "W"
?keyboardwrite=177              "ESC"
?keyboardwrite=66               "B"

Remote Control for Game  (keyboard press = keycode1, keycode2, keycode3, pressTime)
?keyboardpress=215;;;100       "KEY_RIGHT_ARROW"
?keyboardpress=216;;;100       "KEY_LEFT_ARROW"
?keyboardpress=217;;;100       "KEY_DOWN_ARROW"
?keyboardpress=218;;;200       "KEY_UP_ARROW"
?keyboardpress=215;218;;200    "KEY_RIGHT_ARROW + KEY_UP_ARROW"
?keyboardpress=216;218;;200    "KEY_LEFT_ARROW + KEY_UP_ARROW"

Keycode Constants
KEY_LEFT_CTRL
KEY_LEFT_SHIFT
KEY_LEFT_ALT
KEY_LEFT_GUI
KEY_RIGHT_CTRL
KEY_RIGHT_SHIFT
KEY_RIGHT_ALT
KEY_RIGHT_GUI (= Apple “CMD” key)
KEY_UP_ARROW
KEY_DOWN_ARROW
KEY_LEFT_ARROW
KEY_RIGHT_ARROW
KEY_BACKSPACE
KEY_TAB
KEY_RETURN
KEY_ESC
KEY_INSERT
KEY_DELETE
KEY_PAGE_UP
KEY_PAGE_DOWN
KEY_HOME
KEY_END
KEY_CAPS_LOCK
KEY_F1 to KEY_F24
*/

#include <BleKeyboard.h>
BleKeyboard bleKeyboard;

#include <WiFi.h>

// Enter your WiFi ssid and password
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

//自訂指令參數值
String command="";
String cmd="";
String p1="";
String p2="";
String p3="";
String p4="";
String p5="";
String p6="";
String p7="";
String p8="";
String p9="";

//自訂指令拆解狀態值
byte receiveState=0;
byte cmdState=1;
byte pState=1;
byte questionState=0;
byte equalState=0;
byte semicolonState=0;

String feedback = "";

#include <PubSubClient.h>
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_PUBLISH_TOPIC    "yourtopic/send"
#define MQTT_SUBSCRIBE_TOPIC    "yourtopic/get"

const char* mqtt_server = "broker.emqx.io";
const unsigned int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
String mqtt_data = "";

void mqtt_sendText(String topic, String text) {
    String clientId = "";
    if (mqtt_client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      mqtt_client.publish(topic.c_str(), text.c_str());
    }
}

void reconnect() {
  while (!mqtt_client.connected()) {
    String mqtt_clientId = "";
    if (mqtt_client.connect(mqtt_clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      mqtt_client.subscribe(MQTT_SUBSCRIBE_TOPIC);
    } else {
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  mqtt_data = "";
  for (int ci = 0; ci < length; ci++) {
    char c = payload[ci];
    mqtt_data+=c;
  }
  if (String(topic)=="fsm/game"&&mqtt_data!="[]") {
    command="";cmd="";p1="";p2="";p3="";p4="";p5="";p6="";p7="";p8="";p9="";
    receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;
    Serial.println(mqtt_data);    
    for (int i=0;i<mqtt_data.length();i++) {
      char c = mqtt_data[i];
      getCommand(c);
    }
    executeCommand();
  }
}

void executeCommand() {
  Serial.println("");
  //Serial.println("command: "+command);
  Serial.println("cmd= "+cmd+" ,p1= "+p1+" ,p2= "+p2+" ,p3= "+p3+" ,p4= "+p4+" ,p5= "+p5+" ,p6= "+p6+" ,p7= "+p7+" ,p8= "+p8+" ,p9= "+p9);
  Serial.println("");
  
  if (cmd=="restart") {
    ESP.restart();
  }    
  else if (cmd=="resetwifi") {
    WiFi.begin(p1.c_str(), p2.c_str());
    Serial.print("Connecting to ");
    Serial.println(p1);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Serial.println("");
    Serial.println("STAIP: "+WiFi.localIP().toString());
    feedback="STAIP: "+WiFi.localIP().toString();
  }    
  else if (cmd=="inputpullup") {
    pinMode(p1.toInt(), INPUT_PULLUP);
  }  
  else if (cmd=="pinmode") {
    if (p2.toInt()==1)
      pinMode(p1.toInt(), OUTPUT);
    else
      pinMode(p1.toInt(), INPUT);
  }        
  else if (cmd=="digitalwrite") {
    ledcDetachPin(p1.toInt());
    pinMode(p1.toInt(), OUTPUT);
    digitalWrite(p1.toInt(), p2.toInt());
  }   
  else if (cmd=="digitalread") {
    feedback=String(digitalRead(p1.toInt()));
  }
  else if (cmd=="analogwrite") {
    ledcAttachPin(p1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,p2.toInt());
  }       
  else if (cmd=="analogread") {
    feedback=String(analogRead(p1.toInt()));
  }
  else if (cmd=="touchread") {
    feedback=String(touchRead(p1.toInt()));
  }  
  else if (cmd=="keyboardpress") {
    if(bleKeyboard.isConnected()) {
      if (p1!="") bleKeyboard.press(char(p1.toInt()));
      if (p2!="") bleKeyboard.press(char(p2.toInt()));
      if (p3!="") bleKeyboard.press(char(p3.toInt()));
      delay(p4.toInt());
      bleKeyboard.releaseAll();
    }
    else
      feedback="Please connect to ESp32 keyboard";       
  }  
  else if (cmd=="keyboardprint") {
    if(bleKeyboard.isConnected())
      bleKeyboard.print(p1);
    else
      feedback="Please connect to ESp32 keyboard";    
  } 
  else if (cmd=="keyboardwrite") {
    if(bleKeyboard.isConnected())
      bleKeyboard.write(char(p1.toInt()));
    else
      feedback="Please connect to ESp32 keyboard";    
  }
  else {
    feedback="Command is not defined";
  }  

  /*
      Serial.println("Sending 'Hello world'...");
      bleKeyboard.print("Hello world");
  
  
      Serial.println("Sending Enter key...");
      bleKeyboard.write(KEY_RETURN);
  
  
      Serial.println("Sending Ctrl+Alt+Delete...");
      bleKeyboard.press(KEY_LEFT_CTRL);
      bleKeyboard.press(KEY_LEFT_ALT);
      bleKeyboard.press(KEY_DELETE);
      delay(100);
      bleKeyboard.releaseAll();
   */
}

void setup() {
  Serial.begin(115200);
  
  bleKeyboard.setName("ESp32 Keyboard");
  bleKeyboard.begin(); 
  Serial.println("Starting Ble Keyboard");

  initWiFi();

  randomSeed(micros());
  mqtt_client.setServer(mqtt_server,mqtt_port);
  mqtt_client.setCallback(callback);
  //mqtt_client.setBufferSize(1024);  
}

void loop() {
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();
}

void initWiFi() {
  WiFi.mode(WIFI_STA);

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

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
    if ((cmdState==0)&&(pState==1)&&((c!='=')||(equalState==1))) p1=p1+String(c);
    if ((cmdState==0)&&(pState==2)&&(c!=';')) p2=p2+String(c);
    if ((cmdState==0)&&(pState==3)&&(c!=';')) p3=p3+String(c);
    if ((cmdState==0)&&(pState==4)&&(c!=';')) p4=p4+String(c);
    if ((cmdState==0)&&(pState==5)&&(c!=';')) p5=p5+String(c);
    if ((cmdState==0)&&(pState==6)&&(c!=';')) p6=p6+String(c);
    if ((cmdState==0)&&(pState==7)&&(c!=';')) p7=p7+String(c);
    if ((cmdState==0)&&(pState==8)&&(c!=';')) p8=p8+String(c);
    if ((cmdState==0)&&(pState>=9)&&((c!=';')||(semicolonState==1))) p9=p9+String(c);
    
    if (c=='?') questionState=1;
    if (c=='=') equalState=1;
    if ((pState>=9)&&(c==';')) semicolonState=1;
  }
}
