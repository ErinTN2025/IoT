#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "env.h"
#include <DHT.h>

#define DHT_SENSOR_PIN  D7 
#define DHT_SENSOR_TYPE DHT11

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

//  Thêm chân LED (giữ nguyên mọi thứ khác)
#define LED_PIN D2 

// WiFi credentials
const char WIFI_SSID[] = "VNU-IS 501";
const char WIFI_PASSWORD[] = "Vnu.edu.vn";

// Device name from AWS
const char THINGNAME[] = "esp8266";

// MQTT broker host address from AWS
const char MQTT_HOST[] = "akpsx0zlekl8x-ats.iot.ap-southeast-2.amazonaws.com";

// MQTT topics
const char AWS_IOT_PUBLISH_TOPIC[] = "esp8266/pub";
const char AWS_IOT_SUBSCRIBE_TOPIC[] = "esp8266/sub";

// Publishing interval
const long interval = 5000;

// Timezone offset from UTC
const int8_t TIME_ZONE = -5;

// Last time message was sent
unsigned long lastMillis = 0;

// WiFiClientSecure object for secure communication
WiFiClientSecure net;

// X.509 certificate for the CA
BearSSL::X509List cert(cacert);

// X.509 certificate for the client
BearSSL::X509List client_crt(client_cert);

// RSA private key
BearSSL::PrivateKey key(privkey);

// MQTT client instance
PubSubClient client(net);

// Function to connect to NTP server and set time
void NTPConnect() {
  Serial.print("Setting time using SNTP");
  configTime(TIME_ZONE * 3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 1510592825) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
}


void messageReceived(char *topic, byte *payload, unsigned int length) {
  Serial.print("Received [");
  Serial.print(topic);
  Serial.print("]: ");

  String jsonStr;
  for (unsigned int i = 0; i < length; i++) {
    jsonStr += (char)payload[i];
  }
  Serial.println(jsonStr);

  // Parse JSON
  StaticJsonDocument<200> doc;
  DeserializationError err = deserializeJson(doc, jsonStr);

  if (err) {
    Serial.print("JSON parse error: ");
    Serial.println(err.c_str());
    return;
  }

  // Lấy giá trị status
  int msg = doc["status"];   // 0 hoặc 1

  // Bật tắt LED
  if (msg == 1) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");
  } else if (msg == 0) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
  }
}

// Function to connect to AWS IoT Core
void connectAWS() {
  delay(3000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println(String("Attempting to connect to SSID: ") + String(WIFI_SSID));

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  NTPConnect();

  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);

  client.setServer(MQTT_HOST, 8883);
  client.setCallback(messageReceived);

  Serial.println("Connecting to AWS IoT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(1000);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

// Function to publish message to AWS IoT Core
void publishMessage() {
  float h  = dht_sensor.readHumidity();
  float t = dht_sensor.readTemperature();

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%  Temperature: ");
  Serial.print(t);
  Serial.println("°C");

  StaticJsonDocument<200> doc;
  doc["time"] = millis();
  doc["humidity"] = h;
  doc["temperature"] = t;

  char jsonBuffer[200];
  serializeJson(doc, jsonBuffer);

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void setup() {
  Serial.begin(9600);

  //  Thêm cấu hình LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  connectAWS();
}

void loop() {
  if (millis() - lastMillis > interval) {
    lastMillis = millis();
    if (client.connected()) {
      publishMessage();
    }
  }

  client.loop();
}
