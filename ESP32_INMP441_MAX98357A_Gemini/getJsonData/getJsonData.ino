//Generated Date: Tue, 12 Aug 2025 16:03:34 GMT

String result = "{  \"text\": \"你好嗎？\",  \"devices\": [    {      \"servoAngle\": -1    }  ],  \"response\": \"你好！我很好，謝謝你的關心。你呢？\"}";
#include <ArduinoJson.h>

JsonObject jsonObject;
DynamicJsonDocument jsonObject_doc(2048);

JsonArray deviceArray;
DynamicJsonDocument deviceArray_doc(1024);

JsonObject deviceItem;
DynamicJsonDocument deviceItem_doc(1024);

void setup()
{
  Serial.begin(115200);
  delay(10);
deserializeJson(jsonObject_doc, result);
  jsonObject = jsonObject_doc.as<JsonObject>();
  Serial.println(jsonObject["text"].as<String>());
  Serial.println(jsonObject["response"].as<String>());
  deviceArray = jsonObject["devices"].as<JsonArray>();
  for (int i = 0; i <= deviceArray.size() - 1; i++) {
    deviceItem = deviceArray[i];
    if (deviceItem["servoAngle"].as<String>()) {
      Serial.println(deviceItem["servoAngle"].as<String>());
    }
  }
}

void loop()
{

}
