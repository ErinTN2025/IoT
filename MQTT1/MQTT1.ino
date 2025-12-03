-----------------------------
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi
const char* ssid = "VNU-IS 501";
const char* password = "Vnu.edu.vn";

// MQTT SERVER 1 (Server chính trên PC)
const char* mqtt_server_1 = "10.11.51.179";
// MQTT SERVER 2 (MQTTX 2 hoặc PC 2)
const char* mqtt_server_2 = "10.11.51.237";   // sửa IP tuỳ máy bạn

WiFiClient espClient;
WiFiClient espClient2;
PubSubClient mqtt1(espClient);
PubSubClient mqtt2(espClient2);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.printf("Connecting to %s\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Callback khi nhận dữ liệu từ Node
void callback(char* topic, byte* payload, unsigned int length) {
  // Chỉ xử lý nếu đúng topic dữ liệu Node gửi lên:
  if (strcmp(topic, "node_to_gateway") == 0) {
    Serial.print("\n[Gateway] Received from Node: ");
    Serial.println(topic);

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
      Serial.println("JSON parse failed!");
      return;
    }

    // LẤY CHỈ GIÁ TRỊ CẢM BIẾN ĐỘ ẨM
    float sensorValue = doc["value"];

    // Phân loại màu LED theo giá trị độ ẩm
    DynamicJsonDocument res(64);
    if (sensorValue < 50) {
      res["LED"] = "Yellow";
    } 
    else if (sensorValue < 70) {
      res["LED"] = "Green";
    } 
    else {
      res["LED"] = "Red";
    }

    char buffer[50];
    serializeJson(res, buffer, sizeof(buffer));

    // Gửi điều khiển xuống Node qua topic "gateway_to_node"
    mqtt1.publish("gateway_to_node", buffer);
    Serial.print("[Gateway] Sent to Node: ");
    Serial.println(buffer);

    // Forward dữ liệu JSON "sensor" và "value" lên SERVER 2 (MQTT2 PC), topic "gateway_to_mqtt2/data"
    // Không sửa hoặc thêm trường vào payload
    mqtt2.publish("gateway_to_mqtt2/data", payload, length);
    Serial.println("[Gateway] Forwarded data to MQTT2 (PC2)");
  }
}

// Kết nối lại và subscribe đúng topic ở MQTT1
void reconnectMQTT(PubSubClient &client, const char* name, bool subscribeNode) {
  while (!client.connected()) {
    Serial.printf("Attempting MQTT connection to %s ...\n", name);
    String clientId = "Gateway-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.printf("Connected to %s\n", name);
      // Subscribe đúng topic Node gửi dữ liệu ở server 1
      if (subscribeNode) {
        client.subscribe("node_to_gateway");
      }
    } 
    else {
      Serial.printf("Failed to connect to %s, rc=%d. Retrying in 5s...\n",
                    name, client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();

  mqtt1.setServer(mqtt_server_1, 1883);
  mqtt1.setCallback(callback);

  mqtt2.setServer(mqtt_server_2, 1883);
}

void loop() {
  if (!mqtt1.connected()) reconnectMQTT(mqtt1, mqtt_server_1, true);
  if (!mqtt2.connected()) reconnectMQTT(mqtt2, mqtt_server_2, false);

  mqtt1.loop();
  mqtt2.loop();
}