/* 
WebBit(BPI-bit) (ESP32) MyBlockly
Author : ChungYi Fu (Kaohsiung, Taiwan)  2019-12-21 13:00
https://www.facebook.com/francefu

Library
https://github.com/nhatuan84/esp32-lcd
https://github.com/Makuna/NeoPixelBus
https://github.com/asukiaaa/MPU9250_asukiaaa

Command Format :  
http://APIP/?cmd=p1;p2;p3;p4;p5;p6;p7;p8;p9
http://STAIP/?cmd=p1;p2;p3;p4;p5;p6;p7;p8;p9

Default APIP： 192.168.4.1
http://192.168.4.1/?ip
http://192.168.4.1/?mac
http://192.168.4.1/?restart
http://192.168.4.1/?resetwifi=ssid;password
http://192.168.4.1/?inputpullup=gpio
http://192.168.4.1/?pinmode=gpio;value
http://192.168.4.1/?digitalwrite=gpio;value
http://192.168.4.1/?analogwrite=gpio;value
http://192.168.4.1/?digitalread=gpio
http://192.168.4.1/?analogread=gpio
http://192.168.4.1/?touchread=gpio
http://192.168.4.1/?tcp=domain;port;request;wait
--> wait = 0 or 1  (waiting for response)
--> request = /xxxx/xxxx
http://192.168.4.1/?ifttt=event;key;value1;value2;value3
http://192.168.4.1/?thingspeakupdate=key;field1;field2;field3;field4;field5;field6;field7;field8
http://192.168.4.1/?thingspeakread=request
--> request = /channels/xxxxx/fields/1.json?results=1
http://192.168.4.1/?linenotify=token;request
--> request = message=xxxxx
--> request = message=xxxxx&stickerPackageId=xxxxx&stickerId=xxxxx
http://192.168.4.1/?car=gpioL1;gpioL2;gpioR1;gpioR2;L_speed;R_speed;Delay;state
http://192.168.4.1/?i2cLcd=address;gpioSDA;gpioSCL;text1;text2

[WebBit]
http://192.168.4.1/?brightness=value (0~1)
http://192.168.4.1/?led=number;rrggbb   (number:0~24)
http://192.168.4.1/?matrixled=rrggbbrrggbb......rrggbbrrggbb   (0~24)
http://192.168.4.1/?buzzer=frequency;delay
http://192.168.4.1/?buttonA
http://192.168.4.1/?buttonB
http://192.168.4.1/?buttonAB
http://192.168.4.1/?temperature
http://192.168.4.1/?lumL
http://192.168.4.1/?lumR
http://192.168.4.1/?MPU9250
http://192.168.4.1/?accel
http://192.168.4.1/?gyro
http://192.168.4.1/?mag
    
STAIP：
Query：http://192.168.4.1/?ip
Link：http://192.168.4.1/?resetwifi=ssid;password

If you don't need to get response from WebBit(BPI-bit) and want to execute commands quickly, 
you can append a parameter value "stop" at the end of command.
For example:
http://192.168.4.1/?digitalwrite=gpio;value;stop
http://192.168.4.1/?restart=stop

Control Page (http)
Source
https://github.com/fustyles/webduino/blob/master/ESP8266_MyFirmata.html
Page
https://fustyles.github.io/webduino/ESP8266_MyFirmata.html
*/

#include <MPU9250_asukiaaa.h>
MPU9250_asukiaaa mySensor;
float aX, aY, aZ, aSqrt, gX, gY, gZ, mDirection, mX, mY, mZ;

#include <NeoPixelBus.h>
const uint16_t PixelCount = 25; // this example assumes 4 pixels, making it smaller will cause a failure
const uint8_t PixelPin = 4;  // make sure to set this to the correct pin, ignored for Esp8266
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

//Library: https://github.com/nhatuan84/esp32-lcd
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>

// Enter your WiFi ssid and password
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

const char* apssid = "MyFirmata WebBit";
const char* appassword = "12345678";         //AP password require at least 8 characters.

WiFiServer server(80);

String Feedback="", Command="",cmd="",p1="",p2="",p3="",p4="",p5="",p6="",p7="",p8="",p9="";
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
float brightness = 0.5;

void ExecuteCommand()
{
  Serial.println("");
  //Serial.println("Command: "+Command);
  Serial.println("cmd= "+cmd+" ,p1= "+p1+" ,p2= "+p2+" ,p3= "+p3+" ,p4= "+p4+" ,p5= "+p5+" ,p6= "+p6+" ,p7= "+p7+" ,p8= "+p8+" ,p9= "+p9);
  Serial.println("");
  
  if (cmd=="your cmd") {
    // You can do anything
    // Feedback="{\"data\":\"Sensor data"\"}";
  }
  else if (cmd=="ip") {
    Feedback="{\"data\":\""+WiFi.softAPIP().toString()+"\"},{\"data\":\""+WiFi.localIP().toString()+"\"}";
  }  
  else if (cmd=="mac") {
    Feedback="{\"data\":\""+WiFi.macAddress()+"\"}";
  }  
  else if (cmd=="restart") {
    ESP.restart();
  }    
  else if (cmd=="resetwifi") {
    WiFi.begin(p1.c_str(), p2.c_str());
    Serial.print("Connecting to ");
    Serial.println(p1);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Serial.println("");
    Serial.println("STAIP: "+WiFi.localIP().toString());
    Feedback="{\"data\":\""+WiFi.softAPIP().toString()+"\"},{\"data\":\""+WiFi.localIP().toString()+"\"}";
    /*
    if (WiFi.status() == WL_CONNECTED) {
      cmd="linenotify";
      p1 = "yourToken";
      p2 = "message="+WiFi.localIP().toString();
      ExecuteCommand();
    }
    */
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
    Feedback="{\"data\":\""+String(digitalRead(p1.toInt()))+"\"}";
  }
  else if (cmd=="analogwrite") {
    ledcAttachPin(p1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,p2.toInt());
  }       
  else if (cmd=="analogread") {
    Feedback="{\"data\":\""+String(analogRead(p1.toInt()))+"\"}";
  }
  else if (cmd=="touchread") {
    Feedback="{\"data\":\""+String(touchRead(p1.toInt()))+"\"}";
  }    
  else if (cmd=="tcp") {
    String domain=p1;
    int port=p2.toInt();
    String request=p3;
    int wait=p4.toInt();      // wait = 0 or 1

    if ((port==443)||(domain.indexOf("https")==0)||(domain.indexOf("HTTPS")==0))
      Feedback="{\"data\":\""+tcp_https(domain,request,port,wait)+"\"}";
    else
      Feedback="{\"data\":\""+tcp_http(domain,request,port,wait)+"\"}";  
  }
  else if (cmd=="ifttt") {
    String domain="maker.ifttt.com";
    String request = "/trigger/" + p1 + "/with/key/" + p2;
    request += "?value1="+p3+"&value2="+p4+"&value3="+p5;
    Feedback="{\"data\":\""+tcp_https(domain,request,443,0)+"\"}";
  }
  else if (cmd=="thingspeakupdate") {
    String domain="api.thingspeak.com";
    String request = "/update?api_key=" + p1;
    request += "&field1="+p2+"&field2="+p3+"&field3="+p4+"&field4="+p5+"&field5="+p6+"&field6="+p7+"&field7="+p8+"&field8="+p9;
    Feedback="{\"data\":\""+tcp_https(domain,request,443,0)+"\"}";
  }    
  else if (cmd=="thingspeakread") {
    String domain="api.thingspeak.com";
    String request = p1;
    Feedback=tcp_https(domain,request,443,1);
    int s=Feedback.indexOf("feeds");
    Feedback=Feedback.substring(s+8);
    int e=Feedback.indexOf("]");
    Feedback=Feedback.substring(0,e);
    Feedback.replace("},{",";");
    Feedback.replace("\":\"",",");
    Feedback.replace("\":",",");
    Feedback.replace("\",\"",","); 
    Feedback.replace("\"","");
    Feedback.replace("{","");
    Feedback.replace("}","");
    Feedback.replace("[","");
    Feedback.replace("]","");
    Feedback.replace(",\"",",");
    Feedback.replace("\":",",");
    Feedback="{\"data\":\""+Feedback+"\"}";
  } 
  else if (cmd=="linenotify") {
    String token = p1;
    String request = p2;
    Feedback=LineNotify(token,request,1);
    if (Feedback.indexOf("status")!=-1) {
      int s=Feedback.indexOf("{");
      Feedback=Feedback.substring(s);
      int e=Feedback.indexOf("}");
      Feedback=Feedback.substring(0,e);
      Feedback.replace("\"","");
      Feedback.replace("{","");
      Feedback.replace("}","");
    }
    Feedback="{\"data\":\""+Feedback+"\"}";
  } 
  else if (cmd=="car") {
    ledcAttachPin(p1.toInt(), 1);
    ledcSetup(1, 5000, 8);
    ledcWrite(1,0);
    ledcAttachPin(p2.toInt(), 2);
    ledcSetup(2, 5000, 8);
    ledcWrite(2,0);  
    ledcAttachPin(p3.toInt(), 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,0); 
    ledcAttachPin(p4.toInt(), 4);
    ledcSetup(4, 5000, 8);
    ledcWrite(4,0);
    delay(10);
    
    if (p8=="S") {
      //
    }
    else if  (p8=="F") {
      ledcAttachPin(p1.toInt(), 1);
      ledcSetup(1, 5000, 8);
      ledcWrite(1,p5.toInt());
      ledcAttachPin(p4.toInt(), 4);
      ledcSetup(4, 5000, 8);
      ledcWrite(4,p6.toInt());
      if ((p7!="")&&(p7!="0")) {
        delay(p7.toInt());
        ledcAttachPin(p1.toInt(), 1);
        ledcSetup(1, 5000, 8);
        ledcWrite(1,0);
        ledcAttachPin(p4.toInt(), 4);
        ledcSetup(4, 5000, 8);
        ledcWrite(4,0);          
      }     
    }
    else if  (p8=="B") {
      ledcAttachPin(p2.toInt(), 2);
      ledcSetup(2, 5000, 8);
      ledcWrite(2,p5.toInt());  
      ledcAttachPin(p3.toInt(), 3);
      ledcSetup(3, 5000, 8);
      ledcWrite(3,p6.toInt());  
      if ((p7!="")&&(p7!="0")) {
        delay(p7.toInt());
        ledcAttachPin(p2.toInt(), 2);
        ledcSetup(2, 5000, 8);
        ledcWrite(2,0); 
        ledcAttachPin(p3.toInt(), 3);
        ledcSetup(3, 5000, 8);
        ledcWrite(3,0); 
      }     
    }
    else if  (p8=="L") {
      ledcAttachPin(p2.toInt(), 2);
      ledcSetup(2, 5000, 8);
      ledcWrite(2,p5.toInt());  
      ledcAttachPin(p4.toInt(), 4);
      ledcSetup(4, 5000, 8);
      ledcWrite(4,p6.toInt());   
      if ((p7!="")&&(p7!="0")) {
        delay(p7.toInt());
        ledcAttachPin(p2.toInt(), 2);
        ledcSetup(2, 5000, 8);
        ledcWrite(2,0); 
        ledcAttachPin(p4.toInt(), 4);
        ledcSetup(4, 5000, 8);
        ledcWrite(4,0);          
      }
    }
    else if  (p8=="R") {
      ledcAttachPin(p1.toInt(), 1);
      ledcSetup(1, 5000, 8);
      ledcWrite(1,p5.toInt());
      ledcAttachPin(p3.toInt(), 3);
      ledcSetup(3, 5000, 8);
      ledcWrite(3,p6.toInt());  
      if ((p7!="")&&(p7!="0")) {
        delay(p7.toInt());
        ledcAttachPin(p1.toInt(), 1);
        ledcSetup(1, 5000, 8);
        ledcWrite(1,0);
        ledcAttachPin(p3.toInt(), 3);
        ledcSetup(3, 5000, 8);
        ledcWrite(3,0); 
      }        
    }
  } 
  else if (cmd=="i2cLcd") {
    LiquidCrystal_I2C lcd(p1.toInt(),16,2);
    lcd.begin(p2.toInt(), p3.toInt());
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(p4);
    lcd.setCursor(0,1);
    lcd.print(p5);
  }
  else if (cmd=="brightness") {
    brightness = p1.toFloat();
    if (brightness>1) brightness=1;
    if (brightness<0) brightness=0.5;
  }   
  else if (cmd=="buttonA") {
    pinMode(35, INPUT);
    Feedback="{\"data\":\""+String(digitalRead(35))+"\"}";
  }  
  else if (cmd=="buttonB") {
    pinMode(27, INPUT);
    Feedback="{\"data\":\""+String(digitalRead(27))+"\"}";
  }   
  else if (cmd=="buttonAB") {
    pinMode(27, INPUT);
    pinMode(35, INPUT);
    if (digitalRead(27)==0&&digitalRead(35)==0)
      Feedback="{\"data\":\"1\"}";
    else
      Feedback="{\"data\":\"0\"}";
  }  
  else if (cmd=="temperature") {
    pinMode(34, INPUT);
    double Temp_Value = analogRead(34);
    double voltageValue = (Temp_Value / 4095) * 3.3;
    double Rt = ((3.3 - voltageValue) * 5.1) / voltageValue;
    double temperature = ((298.15 * 3950) / (3950 + 298.15 * log(Rt / 10))) - 273.15;
    Feedback="{\"data\":\""+String(temperature)+"\"}";
  }    
  else if (cmd=="lumL") {
    pinMode(36, INPUT);    
    Feedback="{\"data\":\""+String(analogRead(36))+"\"}";
  }  
  else if (cmd=="lumR") {
    pinMode(39, INPUT);    
    Feedback="{\"data\":\""+String(analogRead(39))+"\"}";
  }     
  else if (cmd=="led") {
    p2.toLowerCase();    
    int R = (HextoRGB(p2[0])*16+HextoRGB(p2[1]))*brightness;
    int G = (HextoRGB(p2[2])*16+HextoRGB(p2[3]))*brightness;
    int B = (HextoRGB(p2[4])*16+HextoRGB(p2[5]))*brightness;
    strip.SetPixelColor(p1.toInt(), RgbColor(R, G, B));
    strip.Show();
  }  
  else if (cmd=="matrixled") {
    p1.toLowerCase();    
    int R,G,B;
    for (int i=0;i<p1.length()/6;i++) {
      R = (HextoRGB(p1[i*6])*16+HextoRGB(p1[i*6+1]))*brightness;
      G = (HextoRGB(p1[i*6+2])*16+HextoRGB(p1[i*6+3]))*brightness;
      B = (HextoRGB(p1[i*6+4])*16+HextoRGB(p1[i*6+5]))*brightness;
      strip.SetPixelColor(i, RgbColor(R, G, B));
    }
    strip.Show();
  }  
else if (cmd=="buzzer") {
    int freq = 2000;
    int channel = 0;
    int resolution = 8;
    ledcSetup(channel, freq, resolution);
    ledcAttachPin(25, channel);
    String f="",d="",split=",";
    int s1=0;
    p1+=",";
    p2+=",";
    for (int i=0;i<p1.length();i++) {
      if (p1[i]==split[0]) {
        f=p1.substring(s1,i);
        s1=i+1;
        for (int j=0;j<p2.length();j++) {
          if (p2[j]==split[0]) {
            d=p2.substring(0,j);
            ledcWriteTone(channel, f.toInt());
            delay(d.toInt());
            p2=p2.substring(j+1);
            break;
          }
        }
      }
    }
    ledcWriteTone(channel, 0); 
  }
  else if (cmd=="accel") {
    mySensor.accelUpdate();
    aX = mySensor.accelX();
    aY = mySensor.accelY();
    aZ = mySensor.accelZ();
    aSqrt = mySensor.accelSqrt(); 
    Feedback="{\"data\":\""+String(aX)+"\"},{\"data\":\""+String(aY)+"\"},{\"data\":\""+String(aZ)+"\"},{\"data\":\""+String(aSqrt)+"\"}";
  } 
  else if (cmd=="gyro") {
    mySensor.gyroUpdate();
    gX = mySensor.gyroX();
    gY = mySensor.gyroY();
    gZ = mySensor.gyroZ();  
    p1.toLowerCase();
    Feedback="{\"data\":\""+String(gX)+"\"},{\"data\":\""+String(gY)+"\"},{\"data\":\""+String(gZ)+"\"}";
  }
  else if (cmd=="mag") {
    mySensor.magUpdate();
    mX = mySensor.magX();
    mY = mySensor.magY();
    mZ = mySensor.magZ(); 
    mDirection = mySensor.magHorizDirection(); 
    Feedback="{\"data\":\""+String(mX)+"\"},{\"data\":\""+String(mY)+"\"},{\"data\":\""+String(mZ)+"\"},{\"data\":\""+String(mDirection)+"\"}";    
  }  
  else if (cmd=="MPU9250") {
    mySensor.accelUpdate();
    aX = mySensor.accelX();
    aY = mySensor.accelY();
    aZ = mySensor.accelZ();
    aSqrt = mySensor.accelSqrt(); 
    mySensor.gyroUpdate();
    gX = mySensor.gyroX();
    gY = mySensor.gyroY();
    gZ = mySensor.gyroZ();  
    mySensor.magUpdate();
    mX = mySensor.magX();
    mY = mySensor.magY();
    mZ = mySensor.magZ(); 
    mDirection = mySensor.magHorizDirection(); 
    Feedback="{\"data\":\""+String(aX)+"\"},{\"data\":\""+String(aY)+"\"},{\"data\":\""+String(aZ)+"\"},{\"data\":\""+String(aSqrt)+"\"},{\"data\":\""+String(gX)+"\"},{\"data\":\""+String(gY)+"\"},{\"data\":\""+String(gZ)+"\"},{\"data\":\""+String(mX)+"\"},{\"data\":\""+String(mY)+"\"},{\"data\":\""+String(mZ)+"\"},{\"data\":\""+String(mDirection)+"\"}";
  }
  else {
    Feedback="{\"data\":\"Command is not defined\"}";
  }
  if (Feedback=="") Feedback="{\"data\":\""+Command+"\"}";  
}

int HextoRGB(char val) {
  String hex ="0123456789abcdef";
  return hex.indexOf(val);
}

void setup()
{
  #define LED_POWER 2
  pinMode(LED_POWER, OUTPUT);
  digitalWrite(LED_POWER, HIGH);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  while (!Serial); // wait for serial attach
  
  Wire.begin(21, 22);
  mySensor.setWire(&Wire);
  mySensor.beginAccel();
  mySensor.beginGyro();
  mySensor.beginMag();
  /*
  // You can set your own offset for mag values
    maxXOffset = - (magXMax + magXMin) / 2
               = - (70 - 30) / 2
               = - 40 / 2
               = -20
    magYOffset = - (magYMax + magYMin) / 2
               = - (110 + 10) / 2
               = - 120 / 2
               = -60  
    mySensor.magXOffset = -50;
    mySensor.magYOffset = -55;
    mySensor.magZOffset = -10;  
  */

  strip.Begin();
  
  WiFi.mode(WIFI_AP_STA);

  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

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

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());

  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);
    /*
    cmd="linenotify";
    p1 = "yourToken";
    p2 = "message="+WiFi.localIP().toString();
    ExecuteCommand();
    */
    for (int i=0;i<5;i++) {
      strip.SetPixelColor(0, RgbColor(100, 100, 100));
      strip.Show();
      delay(300);
      strip.SetPixelColor(0, RgbColor(0, 0, 0));
      strip.Show();
      delay(300);      
    }
  }
  else {
    WiFi.softAP(apssid, appassword);
    for (int i=0;i<3;i++) {
      strip.SetPixelColor(0, RgbColor(100, 100, 100));
      strip.Show();
      delay(1000);
      strip.SetPixelColor(0, RgbColor(0, 0, 0));
      strip.Show();
      delay(1000);      
    }    
  }

  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());    
  server.begin(); 
}

void loop()
{
Command="";cmd="";p1="";p2="";p3="";p4="";p5="";p6="";p7="";p8="";p9="";
ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

 WiFiClient client = server.available();

  if (client) 
  { 
    String currentLine = "";

    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();             
        
        getCommand(c);
                
        if (c == '\n') 
        {
          if (currentLine.length() == 0) 
          {          
            
            client.println("HTTP/1.1 200 OK");
            client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
            client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
            client.println("Content-Type: application/json;charset=utf-8");
            client.println("Access-Control-Allow-Origin: *");
            client.println("Connection: close");
            client.println();
            if (Feedback=="")
              client.println("[{\"data\":\"WebBit\"}]");
            else
              client.println("["+Feedback+"]");
            client.println();
            
            Feedback="";
            break;
          } else {
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {
          currentLine += c;
        }

        if ((currentLine.indexOf("/?")!=-1)&&(currentLine.indexOf(" HTTP")!=-1))
        {
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
  
  if (ReceiveState==1)
  {
    Command=Command+String(c);
    
    if (c=='=') cmdState=0;
    if (c==';') strState++;
  
    if ((cmdState==1)&&((c!='?')||(questionstate==1))) cmd=cmd+String(c);
    if ((cmdState==0)&&(strState==1)&&((c!='=')||(equalstate==1))) p1=p1+String(c);
    if ((cmdState==0)&&(strState==2)&&(c!=';')) p2=p2+String(c);
    if ((cmdState==0)&&(strState==3)&&(c!=';')) p3=p3+String(c);
    if ((cmdState==0)&&(strState==4)&&(c!=';')) p4=p4+String(c);
    if ((cmdState==0)&&(strState==5)&&(c!=';')) p5=p5+String(c);
    if ((cmdState==0)&&(strState==6)&&(c!=';')) p6=p6+String(c);
    if ((cmdState==0)&&(strState==7)&&(c!=';')) p7=p7+String(c);
    if ((cmdState==0)&&(strState==8)&&(c!=';')) p8=p8+String(c);
    if ((cmdState==0)&&(strState>=9)&&((c!=';')||(semicolonstate==1))) p9=p9+String(c);
    
    if (c=='?') questionstate=1;
    if (c=='=') equalstate=1;
    if ((strState>=9)&&(c==';')) semicolonstate=1;
  }
}

String tcp_http(String domain,String request,int port,byte wait)
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
            if (state==true) Feedback += String(c);            
            if (c == '\n') 
            {
              if (getResponse.length()==0) state=true; 
              getResponse = "";
            } 
            else if (c != '\r')
              getResponse += String(c);
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

String tcp_https(String domain,String request,int port,byte wait)
{
    WiFiClientSecure client_tcp;
    client_tcp.setInsecure();   //run version 1.0.5 or above
    
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
            if (state==true) Feedback += String(c);            
            if (c == '\n') 
            {
              if (getResponse.length()==0) state=true; 
              getResponse = "";
            } 
            else if (c != '\r')
              getResponse += String(c);
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

String LineNotify(String token, String request, byte wait)
{
  request.replace("%","%25");    
  request.replace(" ","%20");
  request.replace("&","%20");
  request.replace("#","%20");
  //request.replace("\'","%27");
  request.replace("\"","%22");
  request.replace("\n","%0D%0A");
  request.replace("%3Cbr%3E","%0D%0A");
  request.replace("%3Cbr/%3E","%0D%0A");
  request.replace("%3Cbr%20/%3E","%0D%0A");
  request.replace("%3CBR%3E","%0D%0A");
  request.replace("%3CBR/%3E","%0D%0A");
  request.replace("%3CBR%20/%3E","%0D%0A"); 
  request.replace("%20stickerPackageId","&stickerPackageId");
  request.replace("%20stickerId","&stickerId");    
  
  WiFiClientSecure client_tcp;
  client_tcp.setInsecure();   //run version 1.0.5 or above
    
  if (client_tcp.connect("notify-api.line.me", 443)) 
  {
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("User-Agent: ESp8266/1.0");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println("Content-Length: " + String(request.length()));
    client_tcp.println();
    client_tcp.println(request);
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
          if (state==true) Feedback += String(c);          
          if (c == '\n') 
          {
            if (getResponse.length()==0) state=true; 
            getResponse = "";
          } 
          else if (c != '\r')
            getResponse += String(c);
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
