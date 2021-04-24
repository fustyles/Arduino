/*
https://www.thaieasyelec.com/article-wiki/embedded-electronics-application/espino32camintroduction
Library： https://github.com/ThaiEasyElec/ESPIno32CAM


You must modify the file "ESPino32CAM.h" of the library.

// Config Camera Pin (ESPIno32CAM)
#define PWDN_GPIO_NUM     -1 
#define RESET_GPIO_NUM     4
#define XCLK_GPIO_NUM      13
#define SIOD_GPIO_NUM     21
#define SIOC_GPIO_NUM     22
#define Y9_GPIO_NUM       34
#define Y8_GPIO_NUM       35
#define Y7_GPIO_NUM       32
#define Y6_GPIO_NUM       25
#define Y5_GPIO_NUM       27
#define Y4_GPIO_NUM       12
#define Y3_GPIO_NUM       14
#define Y2_GPIO_NUM       26
#define VSYNC_GPIO_NUM    36
#define HREF_GPIO_NUM     39
#define PCLK_GPIO_NUM     33

Replace to ...

// Config Camera Pin (ESP32-CAM)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
*/

//輸入Wi-Fi帳密
const char* ssid     = "*****";   //Wi-Fi帳號
const char* password = "*****";   //Wi-Fi密碼

String token = "*****:*****";   // Create your bot and get the token -> https://telegram.me/fatherbot
String chat_id = "*****";   // Get chat_id -> https://telegram.me/chatid_echo_bot   //Line Notify Token

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "ESPino32CAM.h"
#include "ESPino32CAM_QRCode.h"
ESPino32CAM cam;
ESPino32QRCode qr;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //關閉電壓不穩時重啟電源設定
  
  Serial.begin(115200);
  if (cam.init() != ESP_OK) {
    Serial.println("Fail");
    while (10);
  }
  qr.init(&cam);

  sensor_t *s = cam.sensor();
  s->set_framesize(s, FRAMESIZE_VGA);
  s->set_whitebal(s,true);
  
  WiFi.mode(WIFI_STA);

  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    if ((StartTime+10000) < millis()) break;
  } 

  Serial.println("");
  Serial.println("STAIP address: ");
  Serial.println(WiFi.localIP());
    
  Serial.println("");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reset");
    
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    ledcWrite(3,10);
    delay(200);
    ledcWrite(3,0);
    delay(200);    
    ledcDetachPin(3);
        
    delay(1000);
    ESP.restart();  //若未連上Wi-Fi閃燈兩次後重啟
  }
  else {
    ledcAttachPin(4, 3);
    ledcSetup(3, 5000, 8);
    for (int i=0;i<5;i++) {  //若連上Wi-Fi閃燈五次
      ledcWrite(3,10);
      delay(200);
      ledcWrite(3,0);
      delay(200);    
    }
    ledcDetachPin(3);      
  }
}

void loop() {
  camera_fb_t *fb = cam.capture(); 
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  
  dl_matrix3du_t *image_rgb;
  if(cam.jpg2rgb(fb,&image_rgb)) {
    cam.clearMemory(fb);
    cam.printDebug("\r\nQR Read:");
    qrResoult res = qr.recognition(image_rgb);
    if(res.status) {
      cam.printDebug("");
      cam.printfDebug("Version: %d", res.version);
      cam.printfDebug("ECC level: %c",res.eccLevel);
      cam.printfDebug("Mask: %d", res.mask);
      cam.printDebug("Data type: "+ qr.dataType(res.dataType));
      cam.printfDebug("Length: %d",res.length);
      cam.printDebug("Payload: "+res.payload);
      sendMessage2Telegram(token, chat_id, res.payload);
    }
    else
      cam.printDebug("FAIL");
  }
  cam.clearMemory(image_rgb);  
  
  delay(3000);
}

String sendMessage2Telegram(String token, String chat_id, String text) {
  const char* myDomain = "api.telegram.org";
  String getAll="", getBody = "";
  
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client_tcp;
  //client_tcp.setInsecure();   //version 1.0.6
  if (client_tcp.connect(myDomain, 443)) {
    Serial.println("Connection successful");

    String message = "chat_id="+chat_id+"&text="+text;
    client_tcp.println("POST /bot"+token+"/sendMessage HTTP/1.1");
    client_tcp.println("Host: " + String(myDomain));
    client_tcp.println("Content-Length: " + String(message.length()));
    client_tcp.println("Content-Type: application/x-www-form-urlencoded");
    client_tcp.println();
    client_tcp.print(message);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTime = millis();
    boolean state = false;
    
    while ((startTime + waitTime) > millis())
    {
      Serial.print(".");
      delay(100);      
      while (client_tcp.available()) 
      {
          char c = client_tcp.read();
          if (state==true) getBody += String(c);        
          if (c == '\n') 
          {
            if (getAll.length()==0) state=true; 
            getAll = "";
          } 
          else if (c != '\r')
            getAll += String(c);
          startTime = millis();
       }
       if (getBody.length()>0) break;
    }
    client_tcp.stop();
    Serial.println(getBody);
  }
  else {
    getBody="Connected to api.telegram.org failed.";
    Serial.println("Connected to api.telegram.org failed.");
  }
  
  return getBody;
}
