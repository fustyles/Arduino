#ifndef ESP32_CAMERA_PINS_H_
#define ESP32_CAMERA_PINS_H_

struct CameraPins
{
  int PWDN_GPIO_NUM;
  int RESET_GPIO_NUM;
  int XCLK_GPIO_NUM;
  int SIOD_GPIO_NUM;
  int SIOC_GPIO_NUM;
  int Y9_GPIO_NUM;
  int Y8_GPIO_NUM;
  int Y7_GPIO_NUM;
  int Y6_GPIO_NUM;
  int Y5_GPIO_NUM;
  int Y4_GPIO_NUM;
  int Y3_GPIO_NUM;
  int Y2_GPIO_NUM;
  int VSYNC_GPIO_NUM;
  int HREF_GPIO_NUM;
  int PCLK_GPIO_NUM;
};

#define CAMERA_MODEL_WROVER_KIT \
  {                             \
    .PWDN_GPIO_NUM = -1,        \
    .RESET_GPIO_NUM = -1,       \
    .XCLK_GPIO_NUM = 21,        \
    .SIOD_GPIO_NUM = 26,        \
    .SIOC_GPIO_NUM = 27,        \
    .Y9_GPIO_NUM = 35,          \
    .Y8_GPIO_NUM = 34,          \
    .Y7_GPIO_NUM = 39,          \
    .Y6_GPIO_NUM = 36,          \
    .Y5_GPIO_NUM = 19,          \
    .Y4_GPIO_NUM = 18,          \
    .Y3_GPIO_NUM = 5,           \
    .Y2_GPIO_NUM = 4,           \
    .VSYNC_GPIO_NUM = 25,       \
    .HREF_GPIO_NUM = 23,        \
    .PCLK_GPIO_NUM = 22,        \
  }

#define CAMERA_MODEL_ESP_EYE \
  {                          \
    .PWDN_GPIO_NUM = -1,     \
    .RESET_GPIO_NUM = -1,    \
    .XCLK_GPIO_NUM = 4,      \
    .SIOD_GPIO_NUM = 18,     \
    .SIOC_GPIO_NUM = 23,     \
    .Y9_GPIO_NUM = 36,       \
    .Y8_GPIO_NUM = 37,       \
    .Y7_GPIO_NUM = 38,       \
    .Y6_GPIO_NUM = 39,       \
    .Y5_GPIO_NUM = 35,       \
    .Y4_GPIO_NUM = 14,       \
    .Y3_GPIO_NUM = 13,       \
    .Y2_GPIO_NUM = 34,       \
    .VSYNC_GPIO_NUM = 5,     \
    .HREF_GPIO_NUM = 27,     \
    .PCLK_GPIO_NUM = 25,     \
  }

#define CAMERA_MODEL_M5STACK_PSRAM \
  {                                \
    .PWDN_GPIO_NUM = -1,           \
    .RESET_GPIO_NUM = 15,          \
    .XCLK_GPIO_NUM = 27,           \
    .SIOD_GPIO_NUM = 25,           \
    .SIOC_GPIO_NUM = 23,           \
    .Y9_GPIO_NUM = 19,             \
    .Y8_GPIO_NUM = 36,             \
    .Y7_GPIO_NUM = 18,             \
    .Y6_GPIO_NUM = 39,             \
    .Y5_GPIO_NUM = 5,              \
    .Y4_GPIO_NUM = 34,             \
    .Y3_GPIO_NUM = 35,             \
    .Y2_GPIO_NUM = 32,             \
    .VSYNC_GPIO_NUM = 22,          \
    .HREF_GPIO_NUM = 26,           \
    .PCLK_GPIO_NUM = 21,           \
  }

#define CAMERA_MODEL_M5STACK_V2_PSRAM \
  {                                   \
    .PWDN_GPIO_NUM = -1,              \
    .RESET_GPIO_NUM = 15,             \
    .XCLK_GPIO_NUM = 27,              \
    .SIOD_GPIO_NUM = 22,              \
    .SIOC_GPIO_NUM = 23,              \
    .Y9_GPIO_NUM = 19,                \
    .Y8_GPIO_NUM = 36,                \
    .Y7_GPIO_NUM = 18,                \
    .Y6_GPIO_NUM = 39,                \
    .Y5_GPIO_NUM = 5,                 \
    .Y4_GPIO_NUM = 34,                \
    .Y3_GPIO_NUM = 35,                \
    .Y2_GPIO_NUM = 32,                \
    .VSYNC_GPIO_NUM = 25,             \
    .HREF_GPIO_NUM = 26,              \
    .PCLK_GPIO_NUM = 21,              \
  }

#define CAMERA_MODEL_M5STACK_WIDE \
  {                               \
    .PWDN_GPIO_NUM = -1,          \
    .RESET_GPIO_NUM = 15,         \
    .XCLK_GPIO_NUM = 27,          \
    .SIOD_GPIO_NUM = 22,          \
    .SIOC_GPIO_NUM = 23,          \
    .Y9_GPIO_NUM = 19,            \
    .Y8_GPIO_NUM = 36,            \
    .Y7_GPIO_NUM = 18,            \
    .Y6_GPIO_NUM = 39,            \
    .Y5_GPIO_NUM = 5,             \
    .Y4_GPIO_NUM = 34,            \
    .Y3_GPIO_NUM = 35,            \
    .Y2_GPIO_NUM = 32,            \
    .VSYNC_GPIO_NUM = 25,         \
    .HREF_GPIO_NUM = 26,          \
    .PCLK_GPIO_NUM = 21,          \
  }

#define CAMERA_MODEL_M5STACK_ESP32CAM \
  {                                   \
    .PWDN_GPIO_NUM = -1,              \
    .RESET_GPIO_NUM = 15,             \
    .XCLK_GPIO_NUM = 27,              \
    .SIOD_GPIO_NUM = 25,              \
    .SIOC_GPIO_NUM = 23,              \
    .Y9_GPIO_NUM = 19,                \
    .Y8_GPIO_NUM = 36,                \
    .Y7_GPIO_NUM = 18,                \
    .Y6_GPIO_NUM = 39,                \
    .Y5_GPIO_NUM = 5,                 \
    .Y4_GPIO_NUM = 34,                \
    .Y3_GPIO_NUM = 35,                \
    .Y2_GPIO_NUM = 17,                \
    .VSYNC_GPIO_NUM = 22,             \
    .HREF_GPIO_NUM = 26,              \
    .PCLK_GPIO_NUM = 21,              \
  }

#define CAMERA_MODEL_AI_THINKER \
  {                             \
    .PWDN_GPIO_NUM = 32,        \
    .RESET_GPIO_NUM = -1,       \
    .XCLK_GPIO_NUM = 0,         \
    .SIOD_GPIO_NUM = 26,        \
    .SIOC_GPIO_NUM = 27,        \
    .Y9_GPIO_NUM = 35,          \
    .Y8_GPIO_NUM = 34,          \
    .Y7_GPIO_NUM = 39,          \
    .Y6_GPIO_NUM = 36,          \
    .Y5_GPIO_NUM = 21,          \
    .Y4_GPIO_NUM = 19,          \
    .Y3_GPIO_NUM = 18,          \
    .Y2_GPIO_NUM = 5,           \
    .VSYNC_GPIO_NUM = 25,       \
    .HREF_GPIO_NUM = 23,        \
    .PCLK_GPIO_NUM = 22,        \
  }

#define CAMERA_MODEL_TTGO_T_JOURNAL \
  {                                 \
    .PWDN_GPIO_NUM = 0,             \
    .RESET_GPIO_NUM = 15,           \
    .XCLK_GPIO_NUM = 27,            \
    .SIOD_GPIO_NUM = 25,            \
    .SIOC_GPIO_NUM = 23,            \
    .Y9_GPIO_NUM = 19,              \
    .Y8_GPIO_NUM = 36,              \
    .Y7_GPIO_NUM = 18,              \
    .Y6_GPIO_NUM = 39,              \
    .Y5_GPIO_NUM = 5,               \
    .Y4_GPIO_NUM = 34,              \
    .Y3_GPIO_NUM = 35,              \
    .Y2_GPIO_NUM = 17,              \
    .VSYNC_GPIO_NUM = 22,           \
    .HREF_GPIO_NUM = 26,            \
    .PCLK_GPIO_NUM = 21,            \
  }

#endif //ESP32_CAMERA_PINS_H_