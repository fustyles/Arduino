//Generated Date: Sat, 13 Jan 2024 06:31:19 GMT

#include <BleKeyboard.h>
BleKeyboard bleKeyboard;

#include <PS2X_lib.h>
#define PS2_DAT        19
#define PS2_CMD        23
#define PS2_SEL        5
#define PS2_CLK        18
#define pressures   false
#define rumble      false
PS2X ps2x;
int ps2_error = 0;
byte ps2_type = 0;
String ps2_rotate[8] = {"ul","u","ur","r","dr","d","dl","l"};

void blekeyboard(String type, uint8_t keycode1, uint8_t keycode2, uint8_t keycode3, int presstime, String characters) {
  if(bleKeyboard.isConnected()) {
    if (type=="press") {
      if (keycode1!=-1) bleKeyboard.press(keycode1);
      if (keycode2!=-1) bleKeyboard.press(keycode2);
      if (keycode3!=-1) bleKeyboard.press(keycode3);
      delay(presstime);
      bleKeyboard.releaseAll();
    } else if (type=="press_norelease") {
      if (keycode1!=-1) bleKeyboard.press(keycode1);
    } else if (type=="release") {
      if (keycode1!=-1) bleKeyboard.release(keycode1);
    } else if (type=="release_all") {
      bleKeyboard.releaseAll();
    } else if (type=="print") {
      bleKeyboard.print(characters);
    } else if (type=="write") {
      bleKeyboard.write(char(keycode1));
    }
  }
}

int analogMax = 255;

String side = "LEFT";
String ps2_stick_direction8(boolean position) {
  float X = analogMax/2;
  float Y = analogMax/2;
  if (position) {
    X = ps2x.Analog(PSS_LX);
    Y = ps2x.Analog(PSS_LY);
  } else {
    X = ps2x.Analog(PSS_RX);
    Y = ps2x.Analog(PSS_RY);
  }
  float upper = analogMax-1;
  float lower = 1;
  if (X<=lower&&Y<=lower)  return ps2_rotate[0];
  if (X>=lower&&X<=upper&&Y<=lower)  return ps2_rotate[1];
  if (X>=upper&&Y<=lower)  return ps2_rotate[2];
  if (X>=upper&&Y>=lower&&Y<=upper)  return ps2_rotate[3];
  if (X>=upper&&Y>=upper)  return ps2_rotate[4];
  if (X>=lower&&X<=upper&&Y>=upper)  return ps2_rotate[5];
  if (X<=lower&&Y>=upper)  return ps2_rotate[6];
  if (X<=lower&&Y>=lower&&Y<=upper)  return ps2_rotate[7];
  return "x";
}

void setup()
{
  bleKeyboard.setName("ESP32 PS2");
  bleKeyboard.begin();
  delay(10);

  Serial.begin(115200);
  delay(300);
  ps2_error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  if (ps2_error==0)
    ps2_type = ps2x.readType();
  if (!((ps2_error == 0))) {
    Serial.println("RESTART");
    delay(1000);
    ESP.restart();
  } else {
    Serial.println("OK");
  }
}

void loop()
{
  if (ps2_error != 0) return;
  if (ps2_type == 2)
    ps2x.read_gamepad();
  else
    ps2x.read_gamepad(false, 0);
  if (ps2x.ButtonPressed(PSB_L2)) {
    side = "LEFT";
  } else if (ps2x.ButtonPressed(PSB_R2)) {
    side = "RIGHT";
  }
  if (ps2x.ButtonPressed(PSB_CIRCLE)) {
    blekeyboard("press", (32), -1, -1, 60, "");
  } else if (ps2x.ButtonPressed(PSB_CROSS)) {
    blekeyboard("press", (122), -1, -1, 60, "");
  } else if (ps2x.ButtonPressed(PSB_TRIANGLE)) {
    blekeyboard("press", (KEY_DOWN_ARROW), -1, -1, 60, "");
    if (side == "RIGHT") {
      blekeyboard("press", (KEY_DOWN_ARROW), (KEY_LEFT_ARROW), -1, 60, "");
      blekeyboard("press", (KEY_LEFT_ARROW), -1, -1, 60, "");
    } else {
      blekeyboard("press", (KEY_DOWN_ARROW), (KEY_RIGHT_ARROW), -1, 60, "");
      blekeyboard("press", (KEY_RIGHT_ARROW), -1, -1, 60, "");
    }
    blekeyboard("press", (32), -1, -1, 60, "");
  } else if (ps2x.ButtonPressed(PSB_SQUARE)) {
    if (side == "RIGHT") {
      blekeyboard("press", (KEY_LEFT_ARROW), -1, -1, 60, "");
    } else {
      blekeyboard("press", (KEY_RIGHT_ARROW), -1, -1, 60, "");
    }
    blekeyboard("press", (KEY_DOWN_ARROW), -1, -1, 60, "");
    if (side == "RIGHT") {
      blekeyboard("press", (KEY_DOWN_ARROW), (KEY_LEFT_ARROW), -1, 60, "");
    } else {
      blekeyboard("press", (KEY_DOWN_ARROW), (KEY_RIGHT_ARROW), -1, 60, "");
    }
    blekeyboard("press", (32), -1, -1, 60, "");
  } else if (ps2x.ButtonPressed(PSB_R1)) {
    blekeyboard("press", (KEY_DOWN_ARROW), -1, -1, 60, "");
    if (side == "RIGHT") {
      blekeyboard("press", (KEY_DOWN_ARROW), (KEY_RIGHT_ARROW), -1, 60, "");
      blekeyboard("press", (KEY_RIGHT_ARROW), -1, -1, 60, "");
    } else {
      blekeyboard("press", (KEY_DOWN_ARROW), (KEY_LEFT_ARROW), -1, 60, "");
      blekeyboard("press", (KEY_LEFT_ARROW), -1, -1, 60, "");
    }
    blekeyboard("press", (122), -1, -1, 60, "");
  }
  if ((ps2_stick_direction8(true)=="ur")) {
    blekeyboard("press", (KEY_UP_ARROW), (KEY_RIGHT_ARROW), -1, 60, "");
  } else if ((ps2_stick_direction8(true)=="dr")) {
    blekeyboard("press", (KEY_DOWN_ARROW), (KEY_RIGHT_ARROW), -1, 60, "");
  } else if ((ps2_stick_direction8(true)=="dl")) {
    blekeyboard("press", (KEY_DOWN_ARROW), (KEY_LEFT_ARROW), -1, 60, "");
  } else if ((ps2_stick_direction8(true)=="ul")) {
    blekeyboard("press", (KEY_UP_ARROW), (KEY_LEFT_ARROW), -1, 60, "");
  } else if ((ps2_stick_direction8(true)=="u")) {
    blekeyboard("press", (KEY_UP_ARROW), -1, -1, 60, "");
  } else if ((ps2_stick_direction8(true)=="r")) {
    blekeyboard("press_norelease", (KEY_RIGHT_ARROW), -1, -1, 0, "");
  } else if ((ps2_stick_direction8(true)=="d")) {
    blekeyboard("press_norelease", (KEY_DOWN_ARROW), -1, -1, 0, "");
  } else if ((ps2_stick_direction8(true)=="l")) {
    blekeyboard("press_norelease", (KEY_LEFT_ARROW), -1, -1, 0, "");
  } else if ((ps2_stick_direction8(true)=="x")) {
    blekeyboard("release_all", -1, -1, -1, 0, "");
  }
  delay(10);
}
