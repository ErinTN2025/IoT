/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP8266 chip.

  NOTE: This requires ESP8266 support package:
    https://github.com/esp8266/Arduino

  Please be sure to select the right ESP8266 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6xR8Ja6gB"
#define BLYNK_TEMPLATE_NAME         "TEST1"
#define BLYNK_AUTH_TOKEN            "CsWGJvrD-frAC_AWuLekPo2WuKgw5PXj"


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "VNU-IS 501";
char pass[] = "Vnu.edu.vn";

int ledStatus = 0;

void setup()
{
  // Debug console
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT); // Initialise digital pin 2 as an output pin
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

BLYNK_WRITE(V1) // Executes when the value of virtual pin 0 changes
{
  if(param.asInt() == 1)
  {
    // execute this code if the switch widget is now ON
    digitalWrite(LED_BUILTIN,HIGH);  // Set digital pin 2 HIGH
  }
  else
  {
    // execute this code if the switch widget is now OFF
    digitalWrite(LED_BUILTIN,LOW);  // Set digital pin 2 LOW    
  }
  ledStatus = digitalRead(LED_BUILTIN);
  Blynk.virtualWrite(V2,ledStatus);
}

void loop()
{
  Blynk.run();
}

