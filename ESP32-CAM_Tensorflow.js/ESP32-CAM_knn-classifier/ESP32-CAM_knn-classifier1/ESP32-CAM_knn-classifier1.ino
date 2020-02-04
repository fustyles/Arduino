/*
ESP32-CAM knn-classifier
Open the page in Chrome.
Author : ChungYi Fu (Kaohsiung, Taiwan)  2020-2-4 18:00
https://www.facebook.com/francefu

Line 451, Line 611
You can set the value of remote model url and load model automatically.

首頁
http://APIP
http://STAIP

自訂指令格式 :  
http://APIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9
http://STAIP/control?cmd=P1;P2;P3;P4;P5;P6;P7;P8;P9

預設AP端IP： 192.168.4.1
http://192.168.xxx.xxx?ip
http://192.168.xxx.xxx?mac
http://192.168.xxx.xxx?restart
http://192.168.xxx.xxx?digitalwrite=pin;value
http://192.168.xxx.xxx?analogwrite=pin;value
http://192.168.xxx.xxx?flash=value        //value= 0~255 閃光燈
http://192.168.xxx.xxx?getstill                 //取得視訊影像
http://192.168.xxx.xxx?framesize=size     //size= UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA 改變影像解析度
http://192.168.xxx.xxx?quality=value    // value = 10 to 63
http://192.168.xxx.xxx?brightness=value    // value = -2 to 2
http://192.168.xxx.xxx?contrast=value    // value = -2 to 2 
http://192.168.xxx.xxx?tcp=domain;port;request;wait
http://192.168.xxx.xxx?linenotify=token;request
--> request = message=xxxxx
--> request = message=xxxxx&stickerPackageId=xxxxx&stickerId=xxxxx
http://192.168.xxx.xxx?sendCapturedImageToLineNotify=token  //傳送影像截圖至LineNotify，最大解析度是SXGA

查詢Client端IP：
查詢IP：http://192.168.4.1/?ip
重設網路：http://192.168.4.1/?resetwifi=ssid;password
*/

//輸入WIFI連線帳號密碼
const char* ssid     = "*****";   //your network SSID
const char* password = "*****";   //your network password

//輸入AP端連線帳號密碼
const char* apssid = "ESP32-CAM";
const char* appassword = "12345678";    //AP端密碼至少要八個字元以上

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"         //視訊
#include "soc/soc.h"            //用於電源不穩不重開機
#include "soc/rtc_cntl_reg.h"   //用於電源不穩不重開機

String Feedback="";   //回傳客戶端訊息
//指令參數值
String Command="",cmd="",P1="",P2="",P3="",P4="",P5="",P6="",P7="",P8="",P9="";
//指令拆解狀態值
byte ReceiveState=0,cmdState=1,strState=1,questionstate=0,equalstate=0,semicolonstate=0;

// WARNING!!! Make sure that you have either selected ESP32 Wrover Module,
//            or another board which has PSRAM enabled

//安可信ESP32-CAM模組腳位設定
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

//執行自訂指令
void ExecuteCommand()
{
  //Serial.println("");
  //Serial.println("Command: "+Command);
  if (cmd!="getstill") {
    Serial.println("cmd= "+cmd+" ,P1= "+P1+" ,P2= "+P2+" ,P3= "+P3+" ,P4= "+P4+" ,P5= "+P5+" ,P6= "+P6+" ,P7= "+P7+" ,P8= "+P8+" ,P9= "+P9);
    Serial.println("");
  }
  
  if (cmd=="your cmd") {
    // You can do anything.
    // Feedback="<font color=\"red\">Hello World</font>";  可自訂回傳的HTML程式碼
  }
  else if (cmd=="ip") {
    Feedback="AP IP: "+WiFi.softAPIP().toString();    
    Feedback+=", ";
    Feedback+="STA IP: "+WiFi.localIP().toString();
  }  
  else if (cmd=="mac") {
    Feedback="STA MAC: "+WiFi.macAddress();
  }  
  else if (cmd=="resetwifi") {  //重設WIFI連線
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
    if (P1="4") {
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
    if (P1=="QQVGA")
      s->set_framesize(s, FRAMESIZE_QQVGA);
    else if (P1=="HQVGA")
      s->set_framesize(s, FRAMESIZE_HQVGA);
    else if (P1=="QVGA")
      s->set_framesize(s, FRAMESIZE_QVGA);
    else if (P1=="CIF")
      s->set_framesize(s, FRAMESIZE_CIF);
    else if (P1=="VGA")
      s->set_framesize(s, FRAMESIZE_VGA);  
    else if (P1=="SVGA")
      s->set_framesize(s, FRAMESIZE_SVGA);
    else if (P1=="XGA")
      s->set_framesize(s, FRAMESIZE_XGA);
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
  else if (cmd=="detectCount") {
    Serial.println(P1+" = "+P2); 
  }
  else if (cmd=="tcp") {
    String domain=P1;
    int port=P2.toInt();
    String request=P3;
    int wait=P4.toInt();      // wait = 0 or 1

    if ((port==443)||(domain.indexOf("https")==0)||(domain.indexOf("HTTPS")==0))
      Feedback=tcp_https(domain,request,port,wait);
    else
      Feedback=tcp_http(domain,request,port,wait);  
  }
  else if (cmd=="linenotify") {    // message=xxx&stickerPackageId=xxx&stickerId=xxx
    String token = P1;
    String request = P2;
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
  }
  else if (cmd=="sendCapturedImageToLineNotify") { 
    Feedback=sendCapturedImageToLineNotify(P1);
    if (Feedback=="") Feedback="The image failed to send. <br>The framesize may be too large.";
  } 
  else {
    Feedback="Command is not defined.";
  }
  if (Feedback=="") Feedback=Command;  
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //關閉電源不穩就重開機的設定
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);  //開啟診斷輸出
  Serial.println();

  //視訊組態設定
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
  
  //視訊初始化
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }

  //設定視訊畫面解析度初始值
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);  //UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA  設定初始化影像解析度

  //閃光燈
  ledcAttachPin(4, 4);  
  ledcSetup(4, 5000, 8);    
  
  WiFi.mode(WIFI_AP_STA);
  
  //指定Client端靜態IP
  //WiFi.config(IPAddress(192, 168, 201, 100), IPAddress(192, 168, 201, 2), IPAddress(255, 255, 255, 0));

  WiFi.begin(ssid, password);    //執行網路連線

  delay(1000);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      if ((StartTime+10000) < millis()) break;    //等待10秒連線
  } 

  if (WiFi.status() == WL_CONNECTED) {    //若連線成功
    WiFi.softAP((WiFi.localIP().toString()+"_"+(String)apssid).c_str(), appassword);   //設定SSID顯示客戶端IP         
    Serial.println("");
    Serial.println("STAIP address: ");
    Serial.println(WiFi.localIP());  

    for (int i=0;i<5;i++) {   //若連上WIFI設定閃光燈快速閃爍
      ledcWrite(4,10);
      delay(200);
      ledcWrite(4,0);
      delay(200);    
    }         
  }
  else {
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);         

    for (int i=0;i<2;i++) {    //若連不上WIFI設定閃光燈慢速閃爍
      ledcWrite(4,10);
      delay(1000);
      ledcWrite(4,0);
      delay(1000);    
    }
    //ESP.restart();    //重開機執行WIFI連線
  }     

  //指定AP端IP    
  //WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0)); 
  Serial.println("");
  Serial.println("APIP address: ");
  Serial.println(WiFi.softAPIP());    

  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);

  server.begin();          
}

//自訂網頁首頁管理介面  程式碼中網址 https:// 須增加脫位字元為 https:\/\/
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
  <!DOCTYPE html>
  <head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <script src="https:\/\/ajax.googleapis.com/ajax/libs/jquery/1.8.0/jquery.min.js"></script>
  <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow/tfjs"></script>
  <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow-models/mobilenet"></script>
  <script src="https:\/\/cdn.jsdelivr.net/npm/@tensorflow-models/knn-classifier"></script>      
  </head><body>
  <img id="ShowImage" src="" style="display:none">
  <canvas id="canvas" width="0" height="0"></canvas>
  <img id="example" width="320" height="240" style="display:none">  
  <table>
  <tr>
    <td><input type="button" id="restart" value="Restart"></td> 
    <td colspan="2"><input type="button" id="getStill" value="getStill" style="display:none"></td> 
  </tr>
  <tr>
    <td><button onclick="saveModel();">Save Model</button></td>
    <td colspan="2"><input type="file" id="getModel" style="width:100px"></input></td>
  </tr>
  <tr>
    <td><button id="clearAllClasses">Clear Classes</button></td>
    <td colspan="2"><input type="checkbox" id="startdetection">Start Detection</td>
  </tr>  
  <tr>
    <td><button id="addExample">Train</button></td>
    <td>
      <select id="Class">
        <option value="0">0</option>      
        <option value="1">1</option>
        <option value="2">2</option>
        <option value="3">3</option>
        <option value="4">4</option>
        <option value="5">5</option>
        <option value="6">6</option>
        <option value="7">7</option>
        <option value="8">8</option>
        <option value="9">9</option>
      </select></td>
    <td><span id="count" style="color:red">0</span></td>
  </tr>
  <tr>
    <td>Probability Limit</td> 
    <td colspan="2">
      <select id="probabilityLimit">
        <option value="0" selected="selected">0</option>
        <option value="0.3">0.3</option>
        <option value="0.6">0.6</option>
        <option value="0.9">0.9</option>
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
    <td>Resolution</td> 
    <td colspan="2">
      <select id="framesize">
        <option value="UXGA">UXGA(1600x1200)</option>
        <option value="SXGA">SXGA(1280x1024)</option>
        <option value="XGA">XGA(1024x768)</option>
        <option value="SVGA">SVGA(800x600)</option>
        <option value="VGA">VGA(640x480)</option>
        <option value="CIF">CIF(400x296)</option>
        <option value="QVGA" selected="selected">QVGA(320x240)</option>
        <option value="HQVGA">HQVGA(240x176)</option>
        <option value="QQVGA">QQVGA(160x120)</option>
      </select> 
    </td>
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
  <tr>
    <td>MirrorImage</td> 
    <td colspan="2">  
      <select id="mirrorimage">
        <option value="1">yes</option>
        <option value="0">no</option>
      </select>
    </td>
  </tr>   

  </table>
  <iframe id="ifr" style="display:none"></iframe>
  <div id="lastValue" style="display:none"></div>
  <div id="result" style="color:red">Please wait for loading model.</div>
  </body>
  </html> 
  
  <script>
    var modelPath = "";    //網頁開啟時自動載入遠端模型  https:\/\/***.***.***.***/model.json   
    var getStill = document.getElementById('getStill');
    var ShowImage = document.getElementById('ShowImage');
    var canvas = document.getElementById("canvas");
    var context = canvas.getContext("2d");
    var clearAllClasses = document.getElementById('clearAllClasses');
    var addExample = document.getElementById('addExample');
    var example = document.getElementById('example');
    var Class = document.getElementById('Class');
    var getModel = document.getElementById('getModel');
    var object = document.getElementById('object');
    var probabilityLimit = document.getElementById('probabilityLimit');
    var startdetection = document.getElementById('startdetection');
    var mirrorimage = document.getElementById("mirrorimage");      
    var count = document.getElementById('count'); 
    var result = document.getElementById('result');
    var lastValue = document.getElementById('lastValue');
    var flash = document.getElementById('flash'); 
    var ifr = document.getElementById('ifr');
    var myTimer; 
    var restartCount=0;
    var classifier;
    var mobilenetModule;

    function stopDetection() {
      startdetection.checked = false;
      lastValue.innerHTML = "";
      result.innerHTML = "";
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
        result.innerHTML = "Get still error. <br>Restart ESP32-CAM "+restartCount+" times.";
        myTimer = setInterval(function(){getStill.click();},10000);
        ifr.src = document.location.origin+'?restart';
      }
      else
        result.innerHTML = "Get still error. <br>Please close the page and check ESP32-CAM.";
    }

    ShowImage.onload = function (event) {     
      clearInterval(myTimer);
      restartCount=0;
      canvas.setAttribute("width", ShowImage.width);
      canvas.setAttribute("height", ShowImage.height);
      if (mirrorimage.value==1) {
        context.translate((canvas.width + ShowImage.width) / 2, 0);
        context.scale(-1, 1);
        context.drawImage(ShowImage, 0, 0, ShowImage.width, ShowImage.height);
        context.setTransform(1, 0, 0, 1, 0, 0);
      }
      else
        context.drawImage(ShowImage,0,0,ShowImage.width,ShowImage.height);
        
      if (startdetection.checked) {
        example.onload = function () {
          predictClass(example);
        }
        example.src = canvas.toDataURL('image/jpeg');
      } 
      
      try { 
        document.createEvent("TouchEvent");
        setTimeout(function(){getStill.click();},250);
      }
      catch(e) { 
        setTimeout(function(){getStill.click();},150);
      }      
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
    
    function clearAllClasses_onclick (event) {
      result.innerHTML = '';
      count.innerHTML = "0";
      classifier.clearAllClasses();
    };
    clearAllClasses.addEventListener("click", clearAllClasses_onclick, true);           
    
    function addExample_onclick (event) {
      example.src = canvas.toDataURL('image/jpeg');
      addExampleImage(example, Class.value);
      count.innerHTML = Number(count.innerHTML)+1;
    };
    addExample.addEventListener("click", addExample_onclick, true);

    function addExampleImage(img,index) {
      // Add MobileNet activations to the model repeatedly for all classes.
      var Image = tf.browser.fromPixels(img);
      var logits = mobilenetModule.infer(Image, 'conv_preds');
      classifier.addExample(logits, index);
    }  

    function saveModel() {
      let dataset = classifier.getClassifierDataset();
      let myDataset = {}
      Object.keys(dataset).forEach((key) => {
        let data = dataset[key].dataSync();
        myDataset[key] = Array.from(data);
      });
      let json = JSON.stringify(myDataset)
      
      var link = document.createElement('a');
      link.download="model.json";
      link.href='data:text/text;charset=utf-8,' + encodeURIComponent(json);
      document.body.appendChild(link);
      link.click();
      link.remove();
    }
    
    getModel.onchange = function (event) {
      var target = event.target || window.event.srcElement;
      var files = target.files;
      var fr = new FileReader();
      if (files.length>0) {
        fr.onload = function () {     
          var dataset = fr.result;
          var myDataset = JSON.parse(dataset)
          Object.keys(myDataset).forEach((key) => {
            myDataset[key] = tf.tensor(myDataset[key], [myDataset[key].length / 1024, 1024]);
          })
          classifier.setClassifierDataset(myDataset);
        }
        fr.readAsText(files[0]);
      }
    }                                     

    classifier = knnClassifier.create();
              
    function ObjectDetect() {
      result.innerHTML = "Please wait for loading model.";
      mobilenet.load().then(Module => {
        mobilenetModule = Module;
        result.innerHTML = "";
        getStill.style.display = "block";
        getStill.click();
        
        if (modelPath!="") {
          getRemoteModel(modelPath);
        }
      }); 
    }
    
    async function predictClass(img) { 
      // Make a prediction.
      const Image = tf.browser.fromPixels(img);
      const xlogits = mobilenetModule.infer(Image, 'conv_preds');
      const predict = await classifier.predictClass(xlogits);
      //console.log(predict);
      if (predict.label) {
        var msg = "";
        var MaxProbability = 0;
        for (i=0;i<Class.length;i++) {
          if (predict.confidences[i.toString()]>=0) msg += "[class "+i+"] " + predict.confidences[i.toString()] + "<br>";
          if (i==predict.label) MaxProbability = Number(predict.confidences[i.toString()]);    //取得可能性最大的Class
        }
        if (MaxProbability>=probabilityLimit.value) {   //若辨識結果可能性大於等於設定的底限則畫面顯示結果
          msg = "<font color='red'>Result : class " + predict.label + "</font><br><br>"+msg;              
        }
        else
          msg = "<br><br>"+msg; 
        result.innerHTML = msg; 
        
        if (MaxProbability>=probabilityLimit.value) {   //若辨識結果可能性大於等於設定的底限則執行外部指令
          if (lastValue.innerHTML==predict.label) return;  //若辨識結果維持不變，則不重複執行外部控制指令，避免快速重複執行相同指令造成耗用資源或晶片當機
          lastValue.innerHTML= predict.label;    //更新紀錄目前偵測結果
        
          //依辨識結果predict.label執行ESP32-CAM自訂網址參數指令
          if (predict.label=="0") {
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */            
          }
          else if (predict.label=="1") {
            /*
            var gpio = 4;
            var val = 10;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */            
          }
          else if (predict.label=="2") {
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }
          else if (predict.label=="3") {
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }
          else if (predict.label=="4") {
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }
          else if (predict.label=="5") {
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }
          else if (predict.label=="6") {
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }
          else if (predict.label=="7") {
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }
          else if (predict.label=="8") {
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          }
          else if (predict.label=="9") {
            /*
            var gpio = 4;
            var val = 0;
            var cmd = "analogwrite";  //digitalwrite
            ifr.src = document.location.origin+'?'+cmd+'='+gpio+';'+val;
            */
          } 
        }
        else
          lastValue.innerHTML = "";
      }
    }

    function getRemoteModel(target) {
      var data = $.ajax({
      type: "get",
      dataType: "text",
      url: target,
      success: function(response)
        {
          try {
            var myDataset = JSON.parse(response)
            Object.keys(myDataset).forEach((key) => {
              myDataset[key] = tf.tensor(myDataset[key], [myDataset[key].length / 1024, 1024]);
            })
            classifier.setClassifierDataset(myDataset);
            result.innerHTML = "Load model successfully.";
          }
          catch (e) {
            result.innerHTML = "Load model failed.";
          }
        },
        error: function(exception)
        {
          result.innerHTML = "Load model failed.";
        }
      });
    }      

    window.onload = function () { ObjectDetect(); }    
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
        
        getCommand(c);   //將緩衝區取得的字元拆解出指令參數
                
        if (c == '\n') {
          if (currentLine.length() == 0) {    
            
            if (cmd=="getstill") {
              //回傳JPEG格式影像
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
              //回傳HTML首頁或Feedback
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
          if (Command.indexOf("stop")!=-1) {  //若指令中含關鍵字stop立即斷線 -> http://192.168.xxx.xxx/?cmd=aaa;bbb;ccc;stop
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

//拆解命令字串置入變數
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

String tcp_https(String domain,String request,int port,byte wait)
{
    WiFiClientSecure client_tcp;

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

String LineNotify(String token, String request, byte wait)
{
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
  
  if (client_tcp.connect("notify-api.line.me", 443)) 
  {
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("User-Agent: ESP8266/1.0");
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

String sendCapturedImageToLineNotify(String token) 
{
  String getAll="", getBody = "";
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "";
  }  
      
  WiFiClientSecure client_tcp;
  Serial.println("Connect to notify-api.line.me");
  
  if (client_tcp.connect("notify-api.line.me", 443)) {
    Serial.println("Connection successful");

    String message = "Welcome to Taiwan.";
    String head = "--Taiwan\r\nContent-Disposition: form-data; name=\"message\"; \r\n\r\n" + message + "\r\n--Taiwan\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--Taiwan--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    client_tcp.println("POST /api/notify HTTP/1.1");
    client_tcp.println("Connection: close"); 
    client_tcp.println("Host: notify-api.line.me");
    client_tcp.println("Authorization: Bearer " + token);
    client_tcp.println("Content-Length: " + String(totalLen));
    client_tcp.println("Content-Type: multipart/form-data; boundary=Taiwan");
    client_tcp.println();
    client_tcp.print(head);
    
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        client_tcp.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client_tcp.write(fbBuf, remainder);
      }
    }  
    
    client_tcp.print(tail);
    esp_camera_fb_return(fb);
    
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
          if (c == '\n') 
          {
            if (getAll.length()==0) state=true; 
            getAll = "";
          } 
          else if (c != '\r')
            getAll += String(c);
          if (state==true) getBody += String(c);
          startTime = millis();
       }
       if (getBody.length()>0) break;
    }
    client_tcp.stop();
    //Serial.println(getAll); 
    Serial.println(getBody);
  }
  else {
    getAll="Connected to notify-api.line.me failed.";
    getBody="Connected to notify-api.line.me failed.";
    Serial.println("Connected to notify-api.line.me failed.");
  }
  
  //return getAll;
  return getBody;
}
