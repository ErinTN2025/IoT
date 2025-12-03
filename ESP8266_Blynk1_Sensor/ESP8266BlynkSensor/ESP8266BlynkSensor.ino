/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6_T-CewAR"
#define BLYNK_TEMPLATE_NAME         "TEST2"
#define BLYNK_AUTH_TOKEN            "IZi-BKrA-Pw3qTRmqkWK3LNdI7wo5Xe1"


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "VNU-IS P503";
char pass[] = "Vnu.edu.vn";

// Chân kết nối
const int RAIN_SENSOR_PIN = A0;    
const int LED_PIN         = D0;    

// Biến trạng thái
// int ledStatus = 0;
int sensorValue = 0;

void setup()
{
  Serial.begin(9600);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Ban đầu tắt đèn
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  Serial.println("Da ket noi Blynk va WiFi!");
}

void loop()
{
  Blynk.run();
  
  RainSensorAndControlLED();
  delay(500); 
}
// Hàm đọc cảm biến mưa và tự động bật/tắt đèn
void RainSensorAndControlLED()
{
  sensorValue = analogRead(RAIN_SENSOR_PIN);  
  Serial.print("Gia tri cam bien mua: ");
  Serial.println(sensorValue);

  
  Blynk.virtualWrite(V1, sensorValue);

  if (sensorValue < 700)  
  {
    digitalWrite(LED_PIN, HIGH);
    Blynk.virtualWrite(V2, digitalRead(LED_PIN));
    Serial.println("CO MUA -> BAT DEN");
  }
  else  
  {
    digitalWrite(LED_PIN, LOW);
    Blynk.virtualWrite(V2, digitalRead(LED_PIN));
    Serial.println("KHO -> TAT DEN");
  }
}


