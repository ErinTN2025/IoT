#include <PubSubClient.h>
#include "NetworkHelpers.h"
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ================== BIẾN ==================
int setTemp = 25;
float controlTemp = 25.0;        // Nhiệt độ hiện tại (sẽ publish cái này)
bool powerOn = false;

int statusCode = 1;
String statusText = "Off";

const int TEMP_MIN = 16;
const int TEMP_MAX = 32;

unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 2800;  // ~3 giây 1 lần → mượt

bool isStable = false;  // CỜ QUAN TRỌNG: đã đạt đích và đang ở chế độ giữ ổn định chưa?

void mqtt_connected() {
  mqtt.publish("ds/Terminal", "Device connected\n");
}

void mqtt_handler(const String& topic, const String& value) {
  Serial.printf("↓ %s = %s\n", topic.c_str(), value.c_str());

  if (topic == "downlink/ds/Power") {
    powerOn = (value == "1");
    if (!powerOn) {
      controlTemp = 0.0;
      isStable = false;
      statusCode = 1;
      statusText = "Off";
    } else {
      // Bật máy → bắt đầu từ setTemp luôn
      controlTemp = setTemp;
      isStable = false;  // Phải tiến về lại từ đầu
    }
  }

  if (topic == "downlink/ds/Set Temperature") {
    int temp = value.toInt();
    setTemp = constrain(temp, TEMP_MIN, TEMP_MAX);
    isStable = false;  // Thay đổi setTemp → phải chạy lại từ đầu
    Serial.println("New Set Temperature = " + String(setTemp));
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 3000) delay(10);
  dht.begin();
  systemShowDeviceInfo();
  controlTemp = setTemp;
  randomSeed(millis());
}

void loop() {
  if (millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();

    if (!powerOn) {
      controlTemp = 0.0;
      isStable = false;
      statusCode = 1;
      statusText = "Off";
    }
    else {
      float diff = setTemp - controlTemp;
      if (abs(diff) > 0.5) {
        isStable = false;  

        float step = 0.2 + (float)random(0, 90) / 100.0;  
        if (diff > 0) controlTemp += step;
        else          controlTemp -= step;

        // Không vượt quá đích
        if ((diff > 0 && controlTemp > setTemp) || (diff < 0 && controlTemp < setTemp)) {
          controlTemp = setTemp;
        }

        // Trạng thái đang chạy
        statusCode = (controlTemp < setTemp) ? 3 : 4;
        statusText = (controlTemp < setTemp) ? "Heating" : "Cooling";
      }
      // ĐÃ ĐẠT ĐÍCH (trong ±0.5°C) → BẮT ĐẦU RANDOM NHẸ
      else {
        if (!isStable) {
          Serial.println(">>> ĐÃ ĐẠT NHIỆT ĐỘ ĐÍCH – CHUYỂN SANG CHẾ ĐỘ GIỮ ỔN ĐỊNH + DAO ĐỘNG");
          isStable = true;
        }

        // CHỈ KHI ĐÃ ỔN ĐỊNH MỚI RANDOM NHẸ ±1.6°C
        controlTemp = setTemp - 0.1 + (float)random(0, 3201) / 1000.0;
        controlTemp = constrain(controlTemp, setTemp - 0.1, setTemp + 0.1);

        statusCode = 2;
        statusText = "Idle";
      }

      controlTemp = round(controlTemp * 10.0) / 10.0;
    }

    // PUBLISH
    mqtt.publish("ds/Current Temperature", String(controlTemp, 1).c_str());
    mqtt.publish("ds/Status", String(statusCode).c_str());
    mqtt.publish("ds/StatusText", statusText.c_str());
    mqtt.publish("ds/Power", powerOn ? "1" : "0");

    Serial.printf("→ %.1f°C | Set: %d | %s\n", controlTemp, setTemp, statusText.c_str());
  }

  // RSSI + Uptime
  EVERY_N_MILLIS(10000) { mqtt.publish("ds/uptime", String(millis()/1000).c_str()); }
  EVERY_N_MILLIS(15000) { mqtt.publish("ds/rssi", String(WiFi.RSSI()).c_str()); }

  // Network
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  else if (!mqtt.connected()) connectMQTT();
  else mqtt.loop();

  delay(10);
}