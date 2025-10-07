/*
1/ si aucune infos, il crée un AP wifi par defaut
2/ 
*/


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
#define WIFI_SSID_1 "MYDEBUG3"
#define WIFI_PASS_1 "----------"
bool WIFI_FLAG_1 = true;

#define WIFI_SSID_2 "HOLALALA"
#define WIFI_PASS_2 "ttyyuuii"
bool WIFI_FLAG_2 = true;

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("");
  Serial.println("");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  FastLED.setBrightness(100);

  // HEARTBEAT
  previousMillisHB = millis();
  intervalHB = 15*1000; // 10 s

  // WIFI CLIENT
  Serial.println(F(""));
  Serial.println(F("connecting WiFi"));

  // WIFI CLIENT 1
  WiFi.disconnect(true);
  WiFi.begin(WIFI_SSID_1, WIFI_PASS_1);

  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID_1);
  Serial.println(millis()/1000);
  // Loop continuously while WiFi is not connected
  while ( (WiFi.status() != WL_CONNECTED) && (WIFI_FLAG_1) )
  {
    delay(100);
    Serial.print("/");

    if (millis() - previousMillisHB > intervalHB)
    {
      previousMillisHB = millis();
      WIFI_FLAG_1 = false;
    }
  }  
  Serial.println("");
  Serial.println(millis()/1000);
  Serial.println("--WIFI_SSID_1--");
  Serial.println("");

  // WIFI CLIENT 2
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.disconnect(true);
    WiFi.begin(WIFI_SSID_2, WIFI_PASS_2);

    // Connecting to WiFi...
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID_2);
    // Loop continuously while WiFi is not connected
    while ( (WiFi.status() != WL_CONNECTED) && (WIFI_FLAG_2) )
    {
      delay(100);
      Serial.print("/");

      if (millis() - previousMillisHB > intervalHB)
      {
        previousMillisHB = millis();
        WIFI_FLAG_2 = false;
      }
    }
    
    Serial.println("");
    Serial.println(millis()/1000);
    Serial.println("--WIFI_SSID_2--");
    Serial.println(""); 
  }
  

  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  

  // WIFI AP MODE
  if (WiFi.status() != WL_CONNECTED)
  {
    IPAddress apIP(10,20,30,1);
    IPAddress apNetMsk(255,255,0,0);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(apIP,apIP,apNetMsk);
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
  }
  
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

