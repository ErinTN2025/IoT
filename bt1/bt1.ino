#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "VNU-IS 501";
const char* password = "Vnu.edu.vn";
const char* mqtt_server = "10.11.51.178";  // IP của IoT Gateway (máy chạy Mosquitto)

// Topic MQTT
#define TOPIC_PUBLISH   "soil/sensor"      // Gửi dữ liệu độ ẩm
#define TOPIC_CONTROL   "soil/actuator"    // Nhận lệnh điều khiển LED

// Chân kết nối
#define SOIL_SENSOR_PIN   A0        // Cảm biến độ ẩm đất (analog)
#define LED_GREEN         D1        // GPIO5
#define LED_YELLOW        D2        // GPIO4
#define LED_RED           D3        // GPIO0  (chú ý: D3 có pull-up, vẫn dùng được)

// ====================================================
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastSend = 0;
const long interval = 5000;  // Gửi dữ liệu mỗi 5 giây

// Trạng thái LED
bool ledGreen = false, ledYellow = false, ledRed = false;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void turnOffAllLEDs() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, LOW);
  ledGreen = ledYellow = ledRed = false;
}

void controlLEDs(const char* color) {
  turnOffAllLEDs();
  Serial.print("Bật đèn LED: ");
  Serial.println(color);

  if (strcmp(color, "green") == 0) {
    digitalWrite(LED_GREEN, HIGH);
    ledGreen = true;
  }
  else if (strcmp(color, "yellow") == 0) {
    digitalWrite(LED_YELLOW, HIGH);
    ledYellow = true;
  }
  else if (strcmp(color, "red") == 0) {
    digitalWrite(LED_RED, HIGH);
    ledRed = true;
  }
}

// Xử lý tin nhắn từ Gateway
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.println(message);

  // Phân tích JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* actuator = doc["actuator"];  // "led"
  const char* value = doc["value"];        // "green", "yellow", "red", "off"

  if (strcmp(actuator, "led") == 0) {
    if (strcmp(value, "off") == 0) {
      turnOffAllLEDs();
      Serial.println("Tắt hết LED");
    } else {
      controlLEDs(value);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "SoilNode-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish(TOPIC_PUBLISH, "{\"status\":\"online\",\"node\":\"soil-node1\"}");
      client.subscribe(TOPIC_CONTROL);
      Serial.print("Subscribed to: ");
      Serial.println(TOPIC_CONTROL);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Cấu hình LED
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  turnOffAllLEDs();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastSend > interval) {
    lastSend = now;

    // Đọc cảm biến độ ẩm (giả lập hoặc thật)
    int raw = analogRead(SOIL_SENSOR_PIN);              // 0-1023
    int humidity = map(raw, 300, 1023, 100, 0);            // 100% = ướt, 0% = khô
    humidity = constrain(humidity, 0, 100);

    Serial.print("Độ ẩm đất: ");
    Serial.print(humidity);
    Serial.println("%");

    // Tạo JSON gửi lên Gateway
    StaticJsonDocument<200> doc;
    doc["sensor"] = "soil_moisture";
    doc["humidity"] = humidity;

    char jsonBuffer[200];
    serializeJson(doc, jsonBuffer); 

    Serial.print("Publish: ");
    Serial.println(jsonBuffer);

    client.publish(TOPIC_PUBLISH, jsonBuffer);
  }
}