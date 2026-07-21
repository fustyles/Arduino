/*
Author : ChungYi Fu (Kaohsiung, Taiwan)  2026-07-20 20:00
https://www.facebook.com/francefu

=====================================================================
 ESP32-CAM Snapshot Capture + Browser-side Object Detection (TensorFlow.js)
=====================================================================
 Description:
 The ESP32-CAM hosts a single web page that lets the user configure
 and control the camera (resolution, flash, brightness, contrast,
 mirroring, rotation) and issue simple query-string commands
 (e.g. ?cmd=P1;P2;...; ?digitalwrite=; ?flash=; etc.).
 Object detection is performed client-side: the browser periodically
 requests a still image from the camera, then runs it through a
 TensorFlow.js Graph Model to detect objects and draw bounding boxes.

 Supported Model(s):
 Any TensorFlow.js Graph Model with YOLO-style output
 (post-processing includes non-max suppression). The model path is
 user-configurable in the web UI; it defaults to a YOLOv8n web model,
 but a Graph Model exported from Google Teachable Machine (or other
 compatible YOLO-format models such as YOLOv9n/YOLOv11n) can also be
 used by changing the "Model Path" field.

Home page:
http://APIP
http://STAIP

Custom command format:
http://APIP?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

Default AP-side IP: 192.168.4.1
http://192.168.xxx.xxx?ip
http://192.168.xxx.xxx?mac
http://192.168.xxx.xxx?restart
http://192.168.xxx.xxx?digitalwrite=pin;value
http://192.168.xxx.xxx?analogwrite=pin;value
http://192.168.xxx.xxx?flash=value        //value = 0~255, flash LED brightness
http://192.168.xxx.xxx?getstill                 //capture a still image
http://192.168.xxx.xxx?framesize=size     //size = 10 UXGA|9 SXGA|8 XGA|7 SVGA|6 VGA|5 CIF|4 QVGA|3 HQVGA|0 QQVGA, changes the frame resolution
http://192.168.xxx.xxx?quality=value    // value = 10 to 63
http://192.168.xxx.xxx?brightness=value    // value = -2 to 2
http://192.168.xxx.xxx?contrast=value    // value = -2 to 2 
http://192.168.xxx.xxx?serial=P1;P2;P3;P4;P5;P6;P7;P8;P9

Query the client's IP:
Query IP: http://192.168.4.1/?ip
Reset network: http://192.168.4.1/?resetwifi=ssid;password
*/

//Enter Wi-Fi credentials
const char* ssid     = "xxxxxxxxxx";   //Wi-Fi SSID
const char* password = "xxxxxxxxxx";   //Wi-Fi password

//Enter AP-mode connection credentials
const char* apssid = "ESP32-CAM";
const char* appassword = "12345678";    //AP-side password must be at least 8 characters

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"         //camera driver
#include "soc/soc.h"            //used to prevent reboot on unstable power
#include "soc/rtc_cntl_reg.h"   //used to prevent reboot on unstable power

String Feedback="";   //message returned to the client
//command parameter values
String Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
//command parsing state values
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled

//AI-Thinker ESP32-CAM module pin definitions
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

WiFiServer server(80);

void ExecuteCommand()
{
  //Serial.println("");
  //Serial.println("Command: "+Command);
  if (cmd!="getstill") {
    Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
    Serial.println("");
  }

  //Custom command section
  if (cmd=="your cmd") {
    // You can do anything.
    // Feedback="<font color=\"red\">Hello World</font>";
  }
  else if (cmd=="ip") {
    Feedback="AP IP: "+WiFi.softAPIP().toString();    
    Feedback+=", ";
    Feedback+="STA IP: "+WiFi.localIP().toString();
  }  
  else if (cmd=="mac") {
    Feedback="STA MAC: "+WiFi.macAddress();
  }  
  else if (cmd=="resetwifi") {
    WiFi.begin(P1.c_str(), P2.c_str());
    Serial.print("Connecting to ");
    Serial.println(P1);
    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    } 
    Serial.println("");
    Serial.println("STAIP: "+WiFi.localIP().toString());
    Feedback="STAIP: "+WiFi.localIP().toString();
  }    
  else if (cmd=="restart") {
    ESP.restart();
  }  
  else if (cmd=="digitalwrite") {
    ledcDetachPin(P1.toInt());
    pinMode(P1.toInt(), OUTPUT);
    digitalWrite(P1.toInt(), P2.toInt());
  }   
  else if (cmd=="analogwrite") {
    if (P1=="4") {
      ledcAttachPin(4, 4);  
      ledcSetup(4, 5000, 8);   
      ledcWrite(4,P2.toInt());  
    }
    else {
      ledcAttachPin(P1.toInt(), 5);
      ledcSetup(5, 5000, 8);
      ledcWrite(5,P2.toInt());
    }
  }  
  else if (cmd=="flash") {
    ledcAttachPin(4, 4);  
    ledcSetup(4, 5000, 8);   
     
    int val = P1.toInt();
    ledcWrite(4,val);  
  }  
  else if (cmd=="framesize") { 
    sensor_t * s = esp_camera_sensor_get();  
    if (P1=="96X96")
      s->set_framesize(s, FRAMESIZE_96X96);
    else if (P1=="QQVGA")
      s->set_framesize(s, FRAMESIZE_QQVGA);
    else if (P1=="QCIF")
      s->set_framesize(s, FRAMESIZE_QCIF);
    else if (P1=="HQVGA")
      s->set_framesize(s, FRAMESIZE_HQVGA);
    else if (P1=="240X240")
      s->set_framesize(s, FRAMESIZE_240X240);
    else if (P1=="QVGA")
      s->set_framesize(s, FRAMESIZE_QVGA);
    else if (P1=="CIF")
      s->set_framesize(s, FRAMESIZE_CIF);
    else if (P1=="HVGA")
      s->set_framesize(s, FRAMESIZE_HVGA);
    else if (P1=="VGA")
      s->set_framesize(s, FRAMESIZE_VGA);
    else if (P1=="SVGA")
      s->set_framesize(s, FRAMESIZE_SVGA);
    else if (P1=="XGA")
      s->set_framesize(s, FRAMESIZE_XGA);
    else if (P1=="HD")
      s->set_framesize(s, FRAMESIZE_HD);
    else if (P1=="SXGA")
      s->set_framesize(s, FRAMESIZE_SXGA);
    else if (P1=="UXGA")
      s->set_framesize(s, FRAMESIZE_UXGA);
    else 
      s->set_framesize(s, FRAMESIZE_QVGA); 
  }	
  else if (cmd=="quality") { 
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_quality(s, val);
  }
  else if (cmd=="contrast") {
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt(); 
    s->set_contrast(s, val);
  }
  else if (cmd=="brightness") {
    sensor_t * s = esp_camera_sensor_get();
    int val = P1.toInt();  
    s->set_brightness(s, val);  
  } 
  else if (cmd=="serial") { 
    if (P1!=""&P1!="stop") Serial.println(P1);
    if (P2!=""&P2!="stop") Serial.println(P2);
    Serial.println();
  }    
  else {
    Feedback="Command is not defined.";
  }
  if (Feedback=="") Feedback=Command;  
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brown-out detector reset on unstable power
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);  //enable debug output
  Serial.println();

  //Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);  //UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA  sets the initial frame resolution
  
  //Flash LED
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);    
  
  WiFi.mode(WIFI_AP_STA);
  
  //Assign a static IP to the client
  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  WiFi.begin(ssid, password);    //connect to the network

  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+10000) < millis()) break;    //wait up to 10 seconds to connect
  } 

  if (WiFi.status() == WL_CONNECTED) {    //if the connection succeeded
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);   //set the SSID to show the client's IP         
    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());  

    for (int i=0;i<5;i++) {   //blink the flash LED quickly if Wi-Fi connected
      ledcWrite(4,10);
      delay(200);
      ledcWrite(4,0);
      delay(200);    
    }         
  }
  else {
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);         

    for (int i=0;i<2;i++) {    //blink the flash LED slowly if Wi-Fi failed to connect
      ledcWrite(4,10);
      delay(1000);
      ledcWrite(4,0);
      delay(1000);    
    }
  }     

  //Assign a static IP to the AP side
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());    

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);  

  server.begin();          
}

//Custom home-page management UI
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <!DOCTYPE html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,initial-scale=1">
    <script src="https:\/\/ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js"></script>
    <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow/tfjs@4.20.0/dist/tf.min.js"></script>        
  </head>
  <body>
  <div id="container"></div>
  <img id="ShowImage" src="" style="display:none">
  <canvas id="canvas" style="display:none"></canvas>  
  <table>
  <tr>
    <td><input type="button" id="restart" value="Restart"></td> 
    <td colspan="2"><input type="button" id="getStill" value="Get Still" style="display:none"></td> 
  </tr> 
  <tr>
    <td>Model Path</td>
    <td colspan="2"><input type="text" id="modelpath" value="https:\/\/cdn.jsdelivr.net/gh/hyuto/yolov8-tfjs@master/public/yolov8n_web_model/model.json"></td> 
  </tr> 
  <tr>
    <td>Classes (,)</td>
    <td colspan="2"><input type="text" id="classes" value="person,bicycle,car,motorcycle,airplane,bus,train,truck,boat,traffic light,fire hydrant,stop sign,parking meter,bench,bird,cat,dog,horse,sheep,cow,elephant,bear,zebra,giraffe,backpack,umbrella,handbag,tie,suitcase,frisbee,skis,snowboard,sports ball,kite,baseball bat,baseball glove,skateboard,surfboard,tennis racket,bottle,wine glass,cup,fork,knife,spoon,bowl,banana,apple,sandwich,orange,broccoli,carrot,hot dog,pizza,donut,cake,chair,couch,potted plant,bed,dining table,toilet,tv,laptop,mouse,remote,keyboard,cell phone,microwave,oven,toaster,sink,refrigerator,book,clock,vase,scissors,teddy bear,hair drier,toothbrush"></td> 
  </tr>  
  <tr>
    <td></td> 
    <td colspan="2"><button type="button" id="btnModel" onclick="LoadModel();">Start Detection</button></td> 
  </tr>
  <tr>
    <td>Score</td>
    <td colspan="2">
      <select id="scorelimit">
        <option value="0">0</option>
        <option value="0.1">0.1</option>
        <option value="0.2">0.2</option>
        <option value="0.3">0.3</option>
        <option value="0.4">0.4</option>
        <option value="0.5">0.5</option>
        <option value="0.6" selected>0.6</option>
        <option value="0.7">0.7</option>
        <option value="0.8">0.8</option>
        <option value="0.9">0.9</option>
      </select>
    </td> 
  </tr>                         
  <tr>
    <td>MirrorImage</td> 
    <td colspan="2">  
      <select id="mirrorimage">
        <option value="1">Y</option>
        <option value="0">N</option>
      </select>
    </td>
  </tr>   
  <tr>
    <td>Resolution</td> 
    <td colspan="2">
      <select id="framesize">
        <option value="96X96">FRAMESIZE_96X96 (96x96)</option>
        <option value="QQVGA">FRAMESIZE_QQVGA (160x120)</option>
        <option value="QCIF">FRAMESIZE_QCIF (176x144)</option>
        <option value="HQVGA">FRAMESIZE_HQVGA (240x176)</option>
        <option value="240X240">FRAMESIZE_240X240 (240x240)</option>
        <option value="QVGA" selected="selected">FRAMESIZE_QVGA (320x240)</option>
        <option value="CIF">FRAMESIZE_CIF (400x296)</option>
        <option value="HVGA">FRAMESIZE_HVGA (480x320)</option>
        <option value="VGA">FRAMESIZE_VGA (640x480)</option>
        <option value="SVGA">FRAMESIZE_SVGA (800x600)</option>
        <option value="XGA">FRAMESIZE_XGA (1024x768)</option>
        <option value="HD">FRAMESIZE_HD (1280x720)</option>
        <option value="SXGA">FRAMESIZE_SXGA (1280x1024)</option>
        <option value="UXGA">FRAMESIZE_UXGA (1600x1200)</option>
      </select> 
    </td>
  </tr>    
  <tr>
    <td>Flash</td>
    <td colspan="2"><input type="range" id="flash" min="0" max="255" value="0"></td>
  </tr>
  <tr>
    <td>Quality</td>
    <td colspan="2"><input type="range" id="quality" min="10" max="63" value="10"></td>
  </tr>
  <tr>
    <td>Brightness</td>
    <td colspan="2"><input type="range" id="brightness" min="-2" max="2" value="0"></td>
  </tr>
  <tr>
    <td>Contrast</td>
    <td colspan="2"><input type="range" id="contrast" min="-2" max="2" value="0"></td>
  </tr>
  <tr>
    <td>Rotate</td>
    <td align="left" colspan="2">
        <select onchange="document.getElementById('canvas').style.transform='rotate('+this.value+')';">
          <option value="0deg">0deg</option>
          <option value="90deg">90deg</option>
          <option value="180deg">180deg</option>
          <option value="270deg">270deg</option>
        </select>
    </td>
  </tr>  
  </table>
  <div id="result" style="color:red"></div>
  </body>
  </html> 
  
  <script>
    var getStill = document.getElementById('getStill');
    var ShowImage = document.getElementById('ShowImage');
    var canvas = document.getElementById("canvas");
    var context = canvas.getContext("2d"); 
    var mirrorimage = document.getElementById("mirrorimage");  
    var flash = document.getElementById('flash');
    var result = document.getElementById('result');
    var myTimer;
    var restartCount=0;

    var modelPath = document.getElementById('modelpath');
    var classList = document.getElementById('classes');
    var scoreLimit = document.getElementById('scorelimit');
    var LABELS = [];      

    async function LoadModel() {
      if (modelPath.value=="") {
        result.innerHTML = "Please input model path.";
        return;
      }

      result.innerHTML = "Please wait for loading model.";
      
      tf.loadGraphModel(
        modelPath.value
      ).then(model => {
        Model = model;
        LABELS = classList.value.split(",").map(s => s.trim());
        ShowImage.src = 'http://' + window.location.hostname + ':81/stream';
        result.innerHTML = "";
        getStill.style.display = "block";
        canvas.style.display = "block";

        setTimeout(function(){getStill.click();}, 2000);
      }).catch(err => {
        result.innerHTML = "Error: " + err.message;
      });

    }
    
    getStill.onclick = function (event) {
      clearInterval(myTimer);  
      myTimer = setInterval(function(){error_handle();},5000);
      ShowImage.src=location.origin+'/?getstill='+Math.random();
    }
    
    function error_handle() {
      restartCount++;
      clearInterval(myTimer);
      if (restartCount<=2) {
        //message.innerHTML = "Get still error. <br>Restart ESP32-CAM "+restartCount+" times.";
        myTimer = setInterval(function(){getStill.click();},6000);
      }
      else
        message.innerHTML = "Get still error. <br>Please close the page and check ESP32-CAM.";
    }    
    
    ShowImage.onload = function (event) {
      clearInterval(myTimer);
      restartCount=0;

      DetectImage();        
    }     
    
    restart.onclick = function (event) {
      fetch(location.origin+'/?restart=stop');
    }    
    framesize.onclick = function (event) {
      fetch(document.location.origin+'/?framesize='+this.value+';stop');
    }  
    flash.onchange = function (event) {
      fetch(location.origin+'/?flash='+this.value+';stop');
    } 
    quality.onclick = function (event) {
      fetch(document.location.origin+'/?quality='+this.value+';stop');
    } 
    brightness.onclick = function (event) {
      fetch(document.location.origin+'/?brightness='+this.value+';stop');
    } 
    contrast.onclick = function (event) {
      fetch(document.location.origin+'/?contrast='+this.value+';stop');
    }                             
  
    async function DetectImage() {
      var imgW = ShowImage.naturalWidth;
      var imgH = ShowImage.naturalHeight;
      
      ShowImage.width = imgW;
      ShowImage.height = imgH;
      ShowImage.style.width = imgW + 'px';
      ShowImage.style.height = imgH + 'px';
      canvas.setAttribute("width", imgW);
      canvas.setAttribute("height", imgH);
      canvas.style.width = imgW + "px";
      canvas.style.height = imgH + "px";      
      
      if (mirrorimage.value==1) {
        context.translate((canvas.width + imgW) / 2, 0);
        context.scale(-1, 1);
        context.drawImage(ShowImage, 0, 0, imgW, imgH);
        context.setTransform(1, 0, 0, 1, 0, 0);
      }
      else
        context.drawImage(ShowImage,0,0,imgW,imgH);

      try {
        var threshold = Number(scoreLimit.value);
  
        var input = tf.tidy(() => {
            var img = tf.browser.fromPixels(canvas);
            
            var scale = Math.min(640 / imgW, 640 / imgH);
            var newW = Math.round(imgW * scale);
            var newH = Math.round(imgH * scale);
            
            var resized = tf.image.resizeBilinear(img, [newH, newW]);
            
            var padTop  = Math.floor((640 - newH) / 2);
            var padLeft = Math.floor((640 - newW) / 2);
            var padded  = tf.pad(resized, [
                [padTop,  640 - newH - padTop],
                [padLeft, 640 - newW - padLeft],
                [0, 0]
            ], 114 / 255);
            
            return padded.div(255.0).expandDims(0);
        });
  
        var output = Model.execute(input);
        input.dispose();
  
        var tensor = Array.isArray(output) ? output[0] : output;
        var data = await tensor.squeeze().transpose().array();
        tensor.dispose();
  
        var boxes = [], scores = [], classes = [];
        for (var i = 0; i < data.length; i++) {
          var row = data[i];
          var classScores = row.slice(4);
          var maxScore = Math.max(...classScores);
          if (maxScore < threshold) continue;
          var classId = classScores.indexOf(maxScore);
              
          var scale  = Math.min(640 / imgW, 640 / imgH);
          var padTop  = Math.floor((640 - imgH * scale) / 2);
          var padLeft = Math.floor((640 - imgW * scale) / 2);

          var cx = (row[0] - padLeft) / scale;
          var cy = (row[1] - padTop)  / scale;
          var w  =  row[2] / scale;
          var h  =  row[3] / scale;
              
          boxes.push([cy - h/2, cx - w/2, cy + h/2, cx + w/2]);
          scores.push(maxScore);
          classes.push(classId);
        }
  
        var s = Math.max(canvas.width, canvas.height);
        result.innerHTML = "";
  
        if (boxes.length > 0) {
          var boxTensor   = tf.tensor2d(boxes);
          var scoreTensor = tf.tensor1d(scores);
          var nmsIdx = await tf.image.nonMaxSuppressionAsync(boxTensor, scoreTensor, 50, 0.45, threshold);
          var idxArr = await nmsIdx.array();
          boxTensor.dispose(); scoreTensor.dispose(); nmsIdx.dispose();
  
          var res = "";
          idxArr.forEach(function(idx) {
            var box = boxes[idx];
            var y = box[0], x = box[1], y2 = box[2], x2 = box[3];
            var bw = x2 - x, bh = y2 - y;

            context.lineWidth = Math.round(s / 200);
            context.strokeStyle = "#00FFFF";
            context.beginPath();
            context.rect(x, y, bw, bh);
            context.stroke();
            context.lineWidth = "3";
            context.fillStyle = "yellow";
            context.font = Math.round(s / 30) + "px Arial";
            context.fillText(LABELS[classes[idx]], x, y);

            res += LABELS[classes[idx]]+","+Math.round(scores[idx]*100)+","+Math.round(x)+","+Math.round(y)+","+Math.round(bw)+","+Math.round(bh)+"<br>";
          });
          if (res != "")
            result.innerHTML = res.substr(0, res.length - 4);
        }
        
        setTimeout(function(){getStill.click();}, 150);
  
      } catch (error) {
        console.error(error);
        setTimeout(function(){getStill.click();}, 150);
      }
    }
  </script>   
)rawliteral";

void loop() {
  Feedback="";Command="";cmd="";P1="";P2="";P3="";P4="";P5="";P6="";P7="";P8="";P9="";
  ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;
  
   WiFiClient client = server.available();

  if (client) { 
    String currentLine = "";

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();             
        
        getCommand(c);   //parse the character read from the buffer into command parameters
                
        if (c == '\n') {
          if (currentLine.length() == 0) {    
            
            if (cmd=="getstill") {
              //Return the image in JPEG format
              camera_fb_t * fb = NULL;
              fb = esp_camera_fb_get();  
              if(!fb) {
                Serial.println("Camera capture failed");
                delay(1000);
                ESP.restart();
              }
  
              client.println("HTTP/1.1 200 OK");
              client.println("Access-Control-Allow-Origin: *");              
              client.println("Access-Control-Allow-Headers: Origin, X-Requested-With, Content-Type, Accept");
              client.println("Access-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
              client.println("Content-Type: image/jpeg");
              client.println("Content-Disposition: form-data; name=\"imageFile\"; filename=\"picture.jpg\""); 
              client.println("Content-Length: " + String(fb->len));             
              client.println("Connection: close");
              client.println();
              
              uint8_t *fbBuf = fb->buf;
              size_t fbLen = fb->len;
              for (size_t n=0;n<fbLen;n=n+1024) {
                if (n+1024<fbLen) {
                  client.write(fbBuf, 1024);
                  fbBuf += 1024;
                }
                else if (fbLen%1024>0) {
                  size_t remainder = fbLen%1024;
                  client.write(fbBuf, remainder);
                }
              }  
              
              esp_camera_fb_return(fb);
            
              pinMode(4, OUTPUT);
              digitalWrite(4, LOW);               
            }
            else {
              //Return the HTML home page or the Feedback message
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
              else {
                Data = String((const char *)INDEX_HTML);
              }
              int Index;
              for (Index = 0; Index < Data.length(); Index = Index+1000) {
                client.print(Data.substring(Index, Index+1000));
              }           
              
              client.println();
            }
                        
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
          if (Command.indexOf("stop")!=-1) {  //if the command contains the keyword "stop", disconnect immediately -> http://192.168.xxx.xxx/?cmd=aaa;bbb;ccc;stop
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

//Parse the command string and store it into variables
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
