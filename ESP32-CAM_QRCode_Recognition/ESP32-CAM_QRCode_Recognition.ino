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

#include "ESPino32CAM.h"
#include "ESPino32CAM_QRCode.h"
ESPino32CAM cam;
ESPino32QRCode qr;

void setup() {
  Serial.begin(115200);
  if (cam.init() != ESP_OK) {
    Serial.println("Fail");
    while (10);
  }
  qr.init(&cam);

  sensor_t *s = cam.sensor();
  s->set_framesize(s, FRAMESIZE_VGA);
  s->set_whitebal(s,true);
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
    }
    else
      cam.printDebug("FAIL");
  }
  cam.clearMemory(image_rgb);  
  
  delay(3000);
}
