#include <Arduino.h>

#include <FastLED.h>
#define NUM_LEDS 8
#define DATA_PIN D4

CRGB leds[NUM_LEDS];
uint8_t indexLed = 0;

// HEARTBEAT
uint32_t previousMillisHB;
uint32_t intervalHB;

// WIFI
#include <ESP8266WiFi.h>

// Set WiFi credentials
#define WIFI_SSID "MYDEBUG"
#define WIFI_PASS "aqwzsx789*"

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("");
  Serial.println("");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  FastLED.setBrightness(100);

  // WIFI AP
  // WIFI
  WiFi.disconnect(true);

  Serial.println(F(""));
  Serial.println(F("connecting WiFi"));

  // AP MODE
  IPAddress apIP(10,20,30,1);
  IPAddress apNetMsk(255,255,0,0);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP,apIP, apNetMsk);
  bool apRC = WiFi.softAP("WIFI-TEST", "tototiti");

  if (apRC)
  {
    Serial.println(F("AP WiFi OK"));
  }
  else
  {
    Serial.println(F("AP WiFi failed"));
  }

  // Print ESP soptAP IP Address
  Serial.print(F("softAPIP: "));
  Serial.println(WiFi.softAPIP());


  /*
  // WIFI CLIENT
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  // Loop continuously while WiFi is not connected
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print("/");
  }

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  */

  // HEARTBEAT
  previousMillisHB = millis();
  intervalHB = 500;

  Serial.println("START !!!");
}

void loop()
{
  // HEARTBEAT
  if (millis() - previousMillisHB > intervalHB)
  {
    previousMillisHB = millis();

    // maj leds
    leds[indexLed] = CRGB::Black; 
    indexLed+=1;
    indexLed%=NUM_LEDS;
    leds[indexLed] = CRGB::Red; 
    FastLED.show();

    Serial.println(".");
  }

}

