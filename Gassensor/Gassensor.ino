#define DO_PIN D0
#define AO_PIN A0

void setup() {
  // Initialize the Serial to communicate with the Serial Monitor.
  Serial.begin(9600);
  // initialize the esp8266's pin as an input
  pinMode(DO_PIN, INPUT);
  pinMode(AO_PIN, INPUT);
  Serial.println("Warming up the MQ2 sensor");
  delay(20000);  // wait for the MQ2 to warm up
}

void loop() {
  bool gasState = digitalRead(DO_PIN);
  int gasState2 = analogRead(AO_PIN);
  if (gasState == HIGH)
    Serial.println("The gas is NOT present");
    Serial.println(gasState2);
  else
    Serial.println("The gas is present");
    Serial.println(gasState2);
}