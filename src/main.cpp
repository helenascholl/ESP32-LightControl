#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char *WIFI_SSID = "SomeSSID";
const char *WIFI_PASSWORD = "SomePassword";

const char *MQTT_BROKER_IP = "0.0.0.0";
const char *MQTT_CLIENT_ID = "esp32";

int relay1 = 2;
int relay2 = 4;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");

  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char) payload[i];
  }

  Serial.println(message);

  int lightNr = topic[strlen(topic) - 1];
  int relay = -1;

  if (lightNr == '1') {
    relay = relay1;
  } else if (lightNr == '2') {
    relay = relay2;
  }

  String stateOff = "off";
  String stateOn = "on";

  if (relay != -1 && (message == stateOff || message == stateOn)) {
    Serial.print("Turning light ");
    Serial.print((char) lightNr);
    Serial.print(" ");

    if (message == stateOff) {
      digitalWrite(relay, HIGH);
      Serial.println(stateOff);
    } else if (message == stateOn) {
      digitalWrite(relay, LOW);
      Serial.println(stateOn);
    }
  }
}

void connectToWiFi(const char *ssid, const char *password) {
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToMqtt(const char *brokerIp, uint16_t port = 1883) {
  Serial.print("Connecting to MQTT Broker at ");
  Serial.print(brokerIp);

  mqttClient.setServer(brokerIp, port);
  mqttClient.setCallback(mqttCallback);

  mqttClient.connect(MQTT_CLIENT_ID);

  while (!mqttClient.connected()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to MQTT Broker");
}

void setup() {
  Serial.begin(115200);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  connectToWiFi(WIFI_SSID, WIFI_PASSWORD);
  connectToMqtt(MQTT_BROKER_IP);

  mqttClient.subscribe("/home/rooms/1/lights/+");
}

void loop() {
  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker");

    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("Reconnected to MQTT Broker");
      delay(5000);
    }
  }

  mqttClient.loop();
}
