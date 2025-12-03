/*
  Basic ESP8266 MQTT example
  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.
  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.
  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.

const char* ssid = "VNU-IS 501";
const char* password = "Vnu.edu.vn";
const char* mqtt_server = "10.11.51.179";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  StaticJsonDocument <256> doc;
  deserializeJson(doc,payload);

  // deserializeJson(doc,str); can use string instead of payload
  const char* led = doc["LED"];
  bool value = doc["value"];
  // Switch on the LED if an 1 was received as first character
  if (strcmp(led, "Red") == 0) {
    digitalWrite(D0, HIGH);
    digitalWrite(D1,LOW);
    digitalWrite(D2,LOW);
}
  else if (strcmp(led, "Green") == 0) {
    digitalWrite(D0,LOW);
    digitalWrite(D1, HIGH);
    digitalWrite(D2,LOW);
}
  else if (strcmp(led, "Yellow") == 0) {
    digitalWrite(D0,LOW);
    digitalWrite(D1,LOW);
    digitalWrite(D2, HIGH);

}
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("servertoclient", "hello IoT Gateway...");
      // ... and resubscribe
      client.subscribe("clienttoserver");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(A0, INPUT);
  pinMode(D0,OUTPUT); //red
  pinMode(D1,OUTPUT); //green
  pinMode(D2,OUTPUT); //yellow
  Serial.begin(115200);
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
    JsonObject obj=doc.as<JsonObject>();
    int analogValue = analogRead(A0)-100;
    Serial.println(analogValue);
    float percent =100-analogValue*100/1023;
    Serial.println(percent);
    doc["sensor"] = "Moil moisture";
    doc["value"] = percent;
    char jsonStr[60];
    serializeJson(doc, jsonStr);
    Serial.println();
    client.publish("servertoclient", jsonStr);
  }
}
