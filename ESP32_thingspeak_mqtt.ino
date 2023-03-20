//Generated Date: Mon, 20 Mar 2023 01:24:03 GMT

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

char _lwifi_ssid[] = "teacher";
char _lwifi_pass[] = "87654321";

const char* mqtt_server = "mqtt3.thingspeak.com";
const unsigned int mqtt_port = 1883;
#define MQTT_USER "ORgsJg4UMTYkKxQgNAkmFA81"
#define MQTT_PASSWORD "Ug0bXHfAd93EEznD8YO0kxSo1"
String clientId = "ORgsJg4UMTYkKxQgNAkmFA81";

void initWiFi() {

  for (int i=0;i<2;i++) {
    WiFi.begin(_lwifi_ssid, _lwifi_pass);

    delay(1000);
    Serial.println("");
    Serial.print("Connecting to ");
    Serial.println(_lwifi_ssid);

    long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        if ((StartTime+5000) < millis()) break;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("STAIP address: ");
      Serial.println(WiFi.localIP());
      Serial.println("");

      break;
    }
  }
}

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
String mqtt_data = "";

void mqtt_sendText(String topic, String text) {
    if (mqtt_client.connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      mqtt_client.publish(topic.c_str(), text.c_str());
    }
}

void reconnect() {
  while (!mqtt_client.connected()) {
    String mqtt_clientId = "ORgsJg4UMTYkKxQgNAkmFA8";
    if (mqtt_client.connect(mqtt_clientId.c_str(), MQTT_USER, MQTT_PASSWORD)) {
      mqtt_client.subscribe("channels/997237/subscribe");
    } else {
      delay(5000);
    }
  }
}

String getThingspeakField(String json, int recordNumber, String fieldName) {
  String getRecord ="";
  int s;
  int e;
  String data = json;
  int fr=0;
  for (int i=1;i<=100;i++) {
    s = data.indexOf("{\"created_at", fr);
    e = data.indexOf("}", fr);
    if ((s!=-1)&&(e!=-1)&&(i==recordNumber))  {
      data = data.substring(s,e+1);
      break;
    }
    else
      fr = e+1;
  }
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  JsonObject obj = doc.as<JsonObject>();
  String res = obj[fieldName];
  return res;
}

void setup()
{
  Serial.begin(115200);

  initWiFi();
  randomSeed(micros());
  mqtt_client.setServer(mqtt_server,mqtt_port);
  mqtt_client.setCallback(callback);
  //mqtt_client.setBufferSize(1024);


}

void loop()
{
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  mqtt_data = "";
  for (int ci = 0; ci < length; ci++) {
    char c = payload[ci];
    mqtt_data+=c;
  }
  String data = (getThingspeakField((mqtt_data), 1, "field1"));
  Serial.println(data);
}
