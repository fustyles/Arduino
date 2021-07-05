int delaytime = 3000;  //掃描成功後間隔時間

#include "ESP32QRCodeReader.h"

#include "quirc.h"
#include "Arduino.h"

ESP32QRCodeReader::ESP32QRCodeReader() : ESP32QRCodeReader(CAMERA_MODEL_AI_THINKER, FRAMESIZE_QVGA)
{
}

ESP32QRCodeReader::ESP32QRCodeReader(framesize_t frameSize) : ESP32QRCodeReader(CAMERA_MODEL_AI_THINKER, frameSize)
{
}

ESP32QRCodeReader::ESP32QRCodeReader(CameraPins pins) : ESP32QRCodeReader(pins, FRAMESIZE_QVGA)
{
}

ESP32QRCodeReader::ESP32QRCodeReader(CameraPins pins, framesize_t frameSize) : pins(pins), frameSize(frameSize)
{
  qrCodeQueue = xQueueCreate(10, sizeof(struct QRCodeData));
}

ESP32QRCodeReader::~ESP32QRCodeReader()
{
  end();
}

QRCodeReaderSetupErr ESP32QRCodeReader::setup()
{
  if (!psramFound())
  {
    return SETUP_NO_PSRAM_ERROR;
  }

  cameraConfig.ledc_channel = LEDC_CHANNEL_0;
  cameraConfig.ledc_timer = LEDC_TIMER_0;
  cameraConfig.pin_d0 = pins.Y2_GPIO_NUM;
  cameraConfig.pin_d1 = pins.Y3_GPIO_NUM;
  cameraConfig.pin_d2 = pins.Y4_GPIO_NUM;
  cameraConfig.pin_d3 = pins.Y5_GPIO_NUM;
  cameraConfig.pin_d4 = pins.Y6_GPIO_NUM;
  cameraConfig.pin_d5 = pins.Y7_GPIO_NUM;
  cameraConfig.pin_d6 = pins.Y8_GPIO_NUM;
  cameraConfig.pin_d7 = pins.Y9_GPIO_NUM;
  cameraConfig.pin_xclk = pins.XCLK_GPIO_NUM;
  cameraConfig.pin_pclk = pins.PCLK_GPIO_NUM;
  cameraConfig.pin_vsync = pins.VSYNC_GPIO_NUM;
  cameraConfig.pin_href = pins.HREF_GPIO_NUM;
  cameraConfig.pin_sscb_sda = pins.SIOD_GPIO_NUM;
  cameraConfig.pin_sscb_scl = pins.SIOC_GPIO_NUM;
  cameraConfig.pin_pwdn = pins.PWDN_GPIO_NUM;
  cameraConfig.pin_reset = pins.RESET_GPIO_NUM;
  cameraConfig.xclk_freq_hz = 10000000;
  cameraConfig.pixel_format = PIXFORMAT_GRAYSCALE;

  //cameraConfig.frame_size = FRAMESIZE_SVGA;
  cameraConfig.frame_size = frameSize;
  cameraConfig.jpeg_quality = 15;
  cameraConfig.fb_count = 1;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&cameraConfig);
  if (err != ESP_OK)
  {
    return SETUP_CAMERA_INIT_ERROR;
  }
  return SETUP_OK;
}

void dumpData(const struct quirc_data *data)
{
  Serial.printf("Version: %d\n", data->version);
  Serial.printf("ECC level: %c\n", "MLHQ"[data->ecc_level]);
  Serial.printf("Mask: %d\n", data->mask);
  Serial.printf("Length: %d\n", data->payload_len);
  Serial.printf("Payload: %s\n", data->payload);
}

void qrCodeDetectTask(void *taskData)
{
  ESP32QRCodeReader *self = (ESP32QRCodeReader *)taskData;
  camera_config_t camera_config = self->cameraConfig;
  if (camera_config.frame_size > FRAMESIZE_SVGA)
  {
    if (self->debug)
    {
      Serial.println("Camera Size err");
    }
    vTaskDelete(NULL);
    return;
  }

  struct quirc *q = NULL;
  uint8_t *image = NULL;
  camera_fb_t *fb = NULL;

  uint16_t old_width = 0;
  uint16_t old_height = 0;

  if (self->debug)
  {
    Serial.printf("begin to qr_recoginze\r\n");
  }
  q = quirc_new();
  if (q == NULL)
  {
    if (self->debug)
    {
      Serial.print("can't create quirc object\r\n");
    }
    vTaskDelete(NULL);
    return;
  }

  while (true)
  {

    if (self->debug)
    {
      Serial.printf("alloc qr heap: %u\r\n", xPortGetFreeHeapSize());
      Serial.printf("uxHighWaterMark = %d\r\n", uxTaskGetStackHighWaterMark(NULL));
      Serial.print("begin camera get fb\r\n");
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);

    fb = esp_camera_fb_get();
    if (!fb)
    {
      if (self->debug)
      {
        Serial.println("Camera capture failed");
      }
      continue;
    }

    if (old_width != fb->width || old_height != fb->height)
    {
      if (self->debug)
      {
        Serial.printf("Recognizer size change w h len: %d, %d, %d \r\n", fb->width, fb->height, fb->len);
        Serial.println("Resize the QR-code recognizer.");
        // Resize the QR-code recognizer.
      }
      if (quirc_resize(q, fb->width, fb->height) < 0)
      {
        if (self->debug)
        {
          Serial.println("Resize the QR-code recognizer err (cannot allocate memory).");
        }
        esp_camera_fb_return(fb);
        fb = NULL;
        image = NULL;
        continue;
      }
      else
      {
        old_width = fb->width;
        old_height = fb->height;
      }
    }

    // Serial.printf("quirc_begin\r\n");
    image = quirc_begin(q, NULL, NULL);
    if (self->debug)
    {
      Serial.printf("Frame w h len: %d, %d, %d \r\n", fb->width, fb->height, fb->len);
    }
    memcpy(image, fb->buf, fb->len);
    quirc_end(q);

    if (self->debug)
    {
      Serial.printf("quirc_end\r\n");
    }
    int count = quirc_count(q);
    if (count == 0)
    {
      if (self->debug)
      {
        Serial.printf("Error: not a valid qrcode\n");
      }
      esp_camera_fb_return(fb);
      fb = NULL;
      image = NULL;
      continue;
    }

    struct quirc_code code;
    struct quirc_data data;
    quirc_decode_error_t err;
    struct QRCodeData qrCodeData;
         
    for (int i = 0; i < count; i++)
    {
      quirc_extract(q, i, &code);
      err = quirc_decode(&code, &data);

      if (err)
      {
        const char *error = quirc_strerror(err);
        int len = strlen(error);
        if (self->debug)
        {
          Serial.printf("Decoding FAILED: %s\n", error);
        }
        for (int j = 0; j < len; j++)
        {
          qrCodeData.payload[j] = error[j];
        }
        qrCodeData.valid = false;
        qrCodeData.payload[len] = '\0';
        qrCodeData.payloadLen = len;
      }
      else
      {
        if (self->debug)
        {
          Serial.printf("Decoding successful:\n");
          dumpData(&data);
        }

        qrCodeData.dataType = data.data_type;
        for (int j = 0; j < data.payload_len; j++)
        {
          qrCodeData.payload[j] = data.payload[j];
        }
        qrCodeData.valid = true;
        qrCodeData.payload[data.payload_len] = '\0';
        qrCodeData.payloadLen = data.payload_len;
      }
      xQueueSend(self->qrCodeQueue, &qrCodeData, (TickType_t)0);

      Serial.println();

      if (qrCodeData.valid == true) {
        delay(delaytime);
        break;
      }
    }

    //Serial.printf("finish recoginize\r\n");
    esp_camera_fb_return(fb);
    fb = NULL;
    image = NULL;
  }
  quirc_destroy(q);
  vTaskDelete(NULL);
}

void ESP32QRCodeReader::begin()
{
  beginOnCore(0);
}

void ESP32QRCodeReader::beginOnCore(BaseType_t core)
{
  if (!begun)
  {
    xTaskCreatePinnedToCore(qrCodeDetectTask, "qrCodeDetectTask", QR_CODE_READER_STACK_SIZE, this, QR_CODE_READER_TASK_PRIORITY, &qrCodeTaskHandler, core);
    begun = true;
  }
}

bool ESP32QRCodeReader::receiveQrCode(struct QRCodeData *qrCodeData, long timeoutMs)
{
  return xQueueReceive(qrCodeQueue, qrCodeData, (TickType_t)pdMS_TO_TICKS(timeoutMs)) != 0;
}

void ESP32QRCodeReader::end()
{
  if (begun)
  {
    TaskHandle_t tmpTask = qrCodeTaskHandler;
    if (qrCodeTaskHandler != NULL)
    {
      qrCodeTaskHandler = NULL;
      vTaskDelete(tmpTask);
    }
  }
  begun = false;
}

void ESP32QRCodeReader::setDebug(bool on)
{
  debug = on;
}
