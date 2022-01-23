/*
ESP32 Keyboard RC for PPT
Author : ChungYi Fu (Kaohsiung, Taiwan)  2022-1-23 21:00
https://www.facebook.com/francefu

Library: 
https://github.com/T-vK/ESP32-BLE-Keyboard
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
String Command="";
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
byte ReceiveState=0;
byte cmdState=1;
byte strState=1;
byte questionstate=0;
byte equalstate=0;
byte semicolonstate=0;

String Feedback = "";

void ExecuteCommand() {
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
  Serial.println("");
  
  if (cmd=="your cmd") {
    // You can do anything
    // Feedback="<font color=\"red\">Hello World</font>";
  }
  else if (cmd=="ip") {
    Feedback="AP IP: "+WiFi.softAPIP().toString();    
    Feedback+="<br>";
    Feedback+="STA IP: "+WiFi.localIP().toString();
  }  
  else if (cmd=="mac") {
    Feedback="STA MAC: "+WiFi.macAddress();
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
    /*
    if (WiFi.localIP().toString()!="0.0.0.0") 
    {
      cmd="ifttt";
      P1="eventname";
      P2="key";
      P3=WiFi.localIP().toString();
      ExecuteCommand();
    }
    */
    Feedback="STAIP: "+WiFi.localIP().toString();
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
    Feedback=String(digitalRead(P1.toInt()));
  }
  else if (cmd=="analogwrite") {
    ledcAttachPin(P1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,P2.toInt());
  }       
  else if (cmd=="analogread")
  {
    Feedback=String(analogRead(P1.toInt()));
  }
  else if (cmd=="touchread") {
    Feedback=String(touchRead(P1.toInt()));
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
      Feedback="Please connect to ESP32 BLE";       
  }  
  else if (cmd=="keyboardprint") {
    if(bleKeyboard.isConnected()) {
      bleKeyboard.print(P1);
    }
    else
      Feedback="Please connect to ESP32 BLE";    
  } 
  else if (cmd=="keyboardwrite") {
    if(bleKeyboard.isConnected()) {
      bleKeyboard.write(char(P1.toInt()));
    }
    else
      Feedback="Please connect to ESP32 BLE";    
  }
  else {
    Feedback="Command is not defined";
  }  

  /*
      Serial.println("Sending 'Hello world'...");
      bleKeyboard.print("Hello world");
  
      delay(1000);
  
      Serial.println("Sending Enter key...");
      bleKeyboard.write(KEY_RETURN);
  
      delay(1000);
  
      Serial.println("Sending Play/Pause media key...");
      bleKeyboard.write(KEY_MEDIA_PLAY_PAUSE);
  
      delay(1000);
  
     //
     // Below is an example of pressing multiple keyboard modifiers 
     // which by default is commented out.
      /*
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
  
  bleKeyboard.begin(); 
  Serial.println("Starting BLE");

  WiFi.mode(WIFI_AP_STA);

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  WiFi.begin(ssid, password);

  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if ((StartTime+10000) < millis()) break;
  } 

  if (WiFi.status() == WL_CONNECTED) {
    pinMode(2, OUTPUT);
    for (int i=0;i<5;i++) {
      digitalWrite(2,HIGH);
      delay(100);
      digitalWrite(2,LOW);
      delay(100);
    }
  }  

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
    if (lineToken!="")
      LineNotify(lineToken, WiFi.localIP().toString());
  }
  else
    WiFi.softAP(apssid, appassword);
    
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP()); 
     
  server.begin();   
}

void loop() {
  getRequest();
}

void getRequest() {
  Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

  client = server.available();
  
  if (client) { 
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();             
        
        getCommand(c);
                
        if (c == '\n') {
          if (currentLine.length() == 0) {          
            mainPage();
            Feedback="";
            break;
          } else {
            currentLine = "";
          }
        } 
        else if (c != '\r') {
          currentLine += c;
        }

        if ((currentLine.indexOf("/?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1)) {
          if (Command.indexOf("stop")!=-1) {
            client.println();
            client.println();
            client.stop();
          }
          currentLine="";
          Feedback="";
          ExecuteCommand();
        }
      }
    }
    delay(1);
    client.stop();
  }
}

void getCommand(char c)
{
  if (c=='?') ReceiveState=1;
  if ((c==' ')||(c=='\r')||(c=='\n')) ReceiveState=0;
  
  if (ReceiveState==1) {
    Command=Command+String(c);
    
    if (c=='=') cmdState=0;
    if (c==';') strState++;
  
    if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) P1=P1+String(c);
    if ((cmdState==0)&&(strState==2)&&(c!=';')) P2=P2+String(c);
    if ((cmdState==0)&&(strState==3)&&(c!=';')) P3=P3+String(c);
    if ((cmdState==0)&&(strState==4)&&(c!=';')) P4=P4+String(c);
    if ((cmdState==0)&&(strState==5)&&(c!=';')) P5=P5+String(c);
    if ((cmdState==0)&&(strState==6)&&(c!=';')) P6=P6+String(c);
    if ((cmdState==0)&&(strState==7)&&(c!=';')) P7=P7+String(c);
    if ((cmdState==0)&&(strState==8)&&(c!=';')) P8=P8+String(c);
    if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) P9=P9+String(c);
    
    if (c=='?') questionstate=1;
    if (c=='=') equalstate=1;
    if ((strState>=9)&&(c==';')) semicolonstate=1;
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
  </body>
</html>
)rawliteral";

void mainPage() {
    client.println("HTTP/1.1 200 OK");
    client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
    client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
    client.println("Content-Type: text/html; charset=utf-8");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();
    
    String Data="";
    if (cmd!="")
      Data = Feedback;
    else
      Data = String((const char *)PPT_RC);
   
    for (int Index = 0; Index < Data.length(); Index = Index+1000) {
      client.print(Data.substring(Index, Index+1000));
    }
}

String LineNotify(String token, String message) {
  message.replace("%","%25");  
  message.replace(" ","%20");
  message.replace("&","%20");
  message.replace("#","%20");
  //message.replace("\'","%27");
  message.replace("\"","%22");
  message.replace("\n","%0D%0A");
  
  http.begin("http://linenotify.com/notify.php?token="+token+"&message="+message);
  int httpCode = http.GET();
  if(httpCode > 0) {
      if(httpCode == 200) 
        return http.getString();
      else
        return "";
  } else 
      return "Connection Error!";
}
