------------------------
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ===== WIFI + MQTT SERVER =====
const char* ssid = "VNU-IS 501";
const char* password = "Vnu.edu.vn";
const char* mqtt_server = "10.11.51.179";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

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

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ============= CALLBACK ==============
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.println("JSON parse failed!");
    return;
  }

  const char* led = doc["LED"];
  if (strcmp(led, "Red") == 0) {
    digitalWrite(D0, HIGH);
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
  }
  else if (strcmp(led, "Green") == 0) {
    digitalWrite(D0, LOW);
    digitalWrite(D1, HIGH);
    digitalWrite(D2, LOW);
  }
  else if (strcmp(led, "Yellow") == 0) {
    digitalWrite(D0, LOW);
    digitalWrite(D1, LOW);
    digitalWrite(D2, HIGH);
  }
}

// ============= RECONNECT MQTT =============
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection... ");
    String clientId = "Node8266-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected!");
      client.subscribe("gateway_to_node");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" -> retry in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
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
  if (now - lastMsg > 2000) {
    lastMsg = now;
    DynamicJsonDocument doc(1024);
    JsonObject obj = doc.to<JsonObject>();
    int analogValue = analogRead(A0);
    Serial.print("Analog value: ");
    Serial.println(analogValue);
    float percent = 100.0f - analogValue * 100.0f / 1023.0f;
    Serial.print("Percent moisture: ");
    Serial.println(percent);

    obj["sensor"] = "Moil moisture";
    obj["value"] = percent;
    char jsonStr[60];
    serializeJson(doc, jsonStr, sizeof(jsonStr));
    client.publish("node_to_gateway", jsonStr);
  }
}