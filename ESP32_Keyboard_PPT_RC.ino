/*
ESP32 Keyboard RC for PPT
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-1-25 08:00
https://www.facebook.com/francefu

Library: 
https://github.com/T-vK/ESP32-BLE-Keyboard

Keyboard Modifiers (keyboardpress)
https://www.arduino.cc/en/Reference/KeyboardModifiers

Command Format :  
http://APIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

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
WiFiServer server(80);
WiFiClient client;

#include <HTTPClient.h>
HTTPClient http;

// Enter your WiFi ssid and password
const char* ssid     = "teacher";   //your network SSID
const char* password = "87654321";   //your network password

const char* apssid = "ESP32_PPT";
const char* appassword = "12345678";         //AP password require at least 8 characters.

String lineToken = "";

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
  Serial.println("");
  //Serial.println("command: "+command);
  Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
  Serial.println("");
  
  if (cmd=="your cmd") {
    // You can do anything
    // feedback="<font color=\"red\">Hello World</font>";
  }
  else if (cmd=="ip") {
    feedback="AP IP: "+WiFi.softAPIP().toString();    
    feedback+="<br>";
    feedback+="STA IP: "+WiFi.localIP().toString();
  }  
  else if (cmd=="mac") {
    feedback="STA MAC: "+WiFi.macAddress();
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
    if (WiFi.localIP().toString()!="0.0.0.0") {
      if (lineToken!="")
        lineNotify(lineToken, WiFi.localIP().toString());
    }
    feedback="STAIP: "+WiFi.localIP().toString();
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
  else if (cmd=="keyboardpress") {
    if(bleKeyboard.isConnected()) {
      if (P1!="") bleKeyboard.press(char(P1.toInt()));
      if (P2!="") bleKeyboard.press(char(P2.toInt()));
      if (P3!="") bleKeyboard.press(char(P3.toInt()));
      delay(P4.toInt());
      bleKeyboard.releaseAll();
    }
    else
      feedback="Please connect to ESP32 keyboard";       
  }  
  else if (cmd=="keyboardprint") {
    if(bleKeyboard.isConnected())
      bleKeyboard.print(P1);
    else
      feedback="Please connect to ESP32 keyboard";    
  } 
  else if (cmd=="keyboardwrite") {
    if(bleKeyboard.isConnected())
      bleKeyboard.write(char(P1.toInt()));
    else
      feedback="Please connect to ESP32 keyboard";    
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
  
  bleKeyboard.setName("ESP32 Keyboard");
  bleKeyboard.begin(); 
  Serial.println("Starting Ble Keyboard");

  initWiFi();
}

void loop() {
  getRequest();
}

void initWiFi() {
  WiFi.mode(WIFI_AP_STA);

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
      WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);   //設定SSID顯示客戶端IP         
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
      
      if (lineToken!="")
      lineNotify(lineToken, WiFi.localIP().toString());
      
      break;
    }
  } 

  if (WiFi.status() != WL_CONNECTED) {    //若連線失敗
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);         

    pinMode(2, OUTPUT);
    for (int k=0;k<2;k++) {
      digitalWrite(2,HIGH);
      delay(1000);
      digitalWrite(2,LOW);
      delay(1000);
    }
  } 
    
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());
  
  server.begin(); 
}

void getRequest() {
  command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  receiveState=0,cmdState=1,pState=1,questionState=0,equalState=0,semicolonState=0;

  client = server.available();
  
  if (client) { 
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();             
        
        getCommand(c);
                
        if (c == '\n') {
          if (currentLine.length() == 0) {          
            sendResponse();
            feedback="";
            break;
          } else {
            currentLine = "";
          }
        } 
        else if (c != '\r') {
          currentLine += c;
        }

        if ((currentLine.indexOf("/?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1)) {
          if (command.indexOf("stop")!=-1) {
            client.println();
            client.println();
            client.stop();
          }
          currentLine="";
          feedback="";
          executeCommand();
        }
      }
    }
    delay(1);
    client.stop();
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

//遙控器首頁
static const char PROGMEM PPT_RC[] = R"rawliteral(
<!doctype html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
<style>
.button {
  background-color: gray;
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
  margin: 4px 2px;
  cursor: pointer;
  width: 150px;
  border-radius: 8px;
}
</style>  
</head>
<body align="center">
    <table>
      <tr>
        <td colspan="2" align="center">
        <B>Remote control Powerpoint</B>
        </td>
      </tr>
      <tr>
        <td>
        <input type="button" onclick="sendkeys.src='?keyboardwrite=198';" value="F5" class="button">
        </td>
        <td>
        <input type="button" onclick="sendkeys.src='?keyboardwrite=211';" value="PAGE UP" class="button">
        </td>
      </tr>
      <tr>
        <td>
        <input type="button" onclick="sendkeys.src='?keyboardpress=133;198;;10';" value="SHIFT+F5" class="button">
        </td>
        <td>
        <input type="button" onclick="sendkeys.src='?keyboardwrite=214';" value="PAGE DOWN" class="button">
        </td>
      </tr>
      <tr>
        <td>
        <input type="button" onclick="sendkeys.src='?keyboardwrite=87';" value="W" class="button">
        </td>
        <td>
        <input type="button" onclick="sendkeys.src='?keyboardwrite=177';" value="ESC" class="button">
        </td>
      </tr>
      <tr>
        <td>
        <input type="button" onclick="sendkeys.src='?ip';" value="IP" class="button">
        </td>
        <td>
        <input type="button" onclick="sendkeys.src='?keyboardwrite=66';" value="B" class="button">
        </td>
      </tr>
      <tr>
        <td colspan="2">
        <iframe id="sendkeys" width="250" height="100" style="border:none"></iframe>
        </td>
      </tr>
    </table>
    <script>var sendkeys = document.getElementById('sendkeys');</script>
  </body>
</html>
)rawliteral";

void sendResponse() {
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
    client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
    client.println("Content-Type: text/html; charset=utf-8");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();
    
    String data="";
    if (cmd!="")
      data = feedback;
    else
      data = String((const char *)PPT_RC);
   
    for (int index = 0; index < data.length(); index = index+1024) {
      client.print(data.substring(index, index+1024));
    }
}

String lineNotify(String token, String message) {
  message.replace("%","%25");  
  message.replace(" ","%20");
  message.replace("&","%20");
  message.replace("#","%20");
  //message.replace("\'","%27");
  message.replace("\"","%22");
  message.replace("\n","%0D%0A");
  
  http.begin("http://lineNotify.com/notify.php?token="+token+"&message="+message);
  int httpCode = http.GET();
  if(httpCode > 0) {
      if(httpCode == 200) 
        return http.getString();
      else
        return "";
  } else 
      return "Connection Error!";
}
