/*
1/ si aucune infos, il crée un AP wifi par defaut
2/
*/

/*
A FAIRE
5 => WIFI_CLIENTS
creer le wifi AP
voir pour tableau objet json
*/

#include <Arduino.h>
#include <IPAddress.h>

// LITTLEFS
#include <LittleFS.h>

// CONFIG WIFI
#define SIZE_ARRAY 20
#define WIFI_CLIENTS 5

IPAddress apIP;
IPAddress apNetMsk;
char apName[SIZE_ARRAY];
char apPassword[SIZE_ARRAY];

char ssid[WIFI_CLIENTS][SIZE_ARRAY];
char password[WIFI_CLIENTS][SIZE_ARRAY];
bool active[WIFI_CLIENTS];

char ssid2[WIFI_CLIENTS][SIZE_ARRAY];
char password2[WIFI_CLIENTS][SIZE_ARRAY];
bool active2[WIFI_CLIENTS];

// ARDUINOJSON
#include <ArduinoJson.h>
JsonDocument doc;

// FASTLED
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
bool wifiFlag = true;

// function declaration
void mountFS();
void printJsonFile(const char *filename);
void listDir(const char *dirname);
void readNetworkConfig(const char *filename);

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("");
  Serial.println("");
  
  // FASTLED
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(100);

  // HEARTBEAT
  previousMillisHB = millis();
  intervalHB = 10 * 1000;

  // READ WIFI CONFIG
  mountFS();
  listDir("/config");
  //printJsonFile("/config/networkconfig.json");
  readNetworkConfig("/config/networkconfig.json");

  // LOOP TO WIFI CLIENT
  // WIFI CLIENT
  Serial.println(F(""));
  Serial.println(F("connecting WiFi"));

  for (uint8_t i = 0; i < WIFI_CLIENTS; i++)
  {
    Serial.print("ssid: ");
    Serial.println(ssid[i]);
    // Serial.print(" pw: ");
    // Serial.print(password[i]);
    // Serial.print(" act: ");
    // Serial.println(active[i]);

    if (active[i]==1 && strlen(ssid[i])>0)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        WiFi.disconnect(true);
        wifiFlag = true;
        WiFi.begin(ssid[i], password[i]);

        // Loop continuously while WiFi is not connected
        while ( (WiFi.status() != WL_CONNECTED) && (wifiFlag) )
        {
          delay(100);
          Serial.print("/");

          if (millis() - previousMillisHB > intervalHB)
          {
            previousMillisHB = millis();
            wifiFlag = false;
          }
        }
      }
      Serial.println(" ");
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.print("connected to ");
        Serial.println(ssid[i]);
      }
    }
    
  }
  Serial.println(" ");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  
  
  /*
  

  // WIFI CLIENT 1
  

  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID_1);
  Serial.println(millis()/1000);
  
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
    indexLed += 1;
    indexLed %= NUM_LEDS;
    leds[indexLed] = CRGB::Red;
    FastLED.show();

    Serial.print(".");
    if (indexLed == 0)
    {
      Serial.println(".");
    }
  }
}


void mountFS()
  {
    Serial.println(F("Mount LittleFS"));
    if (!LittleFS.begin())
    {
      Serial.println(F("LittleFS mount failed"));
      return;
    }
  }

  void printJsonFile(const char *filename)
  {
    // Open file for reading
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
      Serial.println(F("Failed to open file for reading"));
    }

    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      Serial.println(F("Failed to deserialize file in print object"));
      Serial.println(error.c_str());
    }
    else
    {
      // serializeJsonPretty(doc, Serial);
      Serial.print(measureJson(doc));
      serializeJson(doc, Serial);
      Serial.println();
    }

    // Close the file (File's destructor doesn't close the file)
    file.close();
  }

  void listDir(const char *dirname)
  {
    Serial.printf("Listing directory: %s", dirname);
    Serial.println();

    Dir root = LittleFS.openDir(dirname);

    while (root.next())
    {
      File file = root.openFile("r");
      Serial.print(F("  FILE: "));
      Serial.print(root.fileName());
      Serial.print(F("  SIZE: "));
      Serial.print(file.size());
      Serial.println();
      file.close();
    }
    Serial.println();
  }


  void readNetworkConfig(const char *filename)
  {
    // lire les données depuis le fichier littleFS
    // Open file for reading
    File file = LittleFS.open(filename, "r");
    if (!file)
    {
      Serial.println(F("Failed to open file for reading"));
      return;
    }

    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      Serial.println(F("Failed to deserialize file in read network "));
      Serial.println(error.c_str());
    }
    else
    {
      // Copy values from the JsonObject to the Config
      if (doc["apIP"].is<JsonVariant>())
      {
        JsonArray apIPArray = doc["apIP"];

        apIP[0] = apIPArray[0];
        apIP[1] = apIPArray[1];
        apIP[2] = apIPArray[2];
        apIP[3] = apIPArray[3];
      }

      if (doc["apNetMsk"].is<JsonVariant>())
      {
        JsonArray apNetMskArray = doc["apNetMsk"];

        apNetMsk[0] = apNetMskArray[0];
        apNetMsk[1] = apNetMskArray[1];
        apNetMsk[2] = apNetMskArray[2];
        apNetMsk[3] = apNetMskArray[3];
      }

      if (doc["apName"].is<JsonVariant>())
      {
        strlcpy(apName,
                doc["apName"],
                SIZE_ARRAY);
      }

      if (doc["apPassword"].is<JsonVariant>())
      {
        strlcpy(apPassword,
                doc["apPassword"],
                SIZE_ARRAY);
      }

      // if (doc["wifi_client"].is<JsonVariant>())
      // {
      //   JsonArray wifiClientArray = doc["wifi_client"];

      //   for (uint8_t i = 0; i < WIFI_CLIENTS; i++)
      //   {
      //     JsonArray wifiArray = wifiClientArray[i];          
      //     //serializeJson(wifiArray, Serial);
      //     //Serial.println("");
          
      //     strlcpy(ssid[i], wifiArray[0], SIZE_ARRAY);
      //     strlcpy(password[i], wifiArray[1], SIZE_ARRAY);
      //     active[i]= wifiArray[2];

      //     //serializeJson(wifiArray[0], Serial);
      //     //Serial.println("");
      //     //Serial.println(ssid[i]);
      //   }
      // }

      if (doc["wifi_client_2"].is<JsonVariant>())
      {
        JsonArray wifiClientArray2 = doc["wifi_client_2"];
        // serializeJson(wifiClientArray2, Serial);
        // Serial.println("");

        for (uint8_t i = 0; i < WIFI_CLIENTS; i++)
        {
          if (wifiClientArray2[i]["ssid"].is<JsonVariant>())
          {
            strlcpy(ssid[i], wifiClientArray2[i]["ssid"], SIZE_ARRAY);
            strlcpy(password[i], wifiClientArray2[i]["password"], SIZE_ARRAY);
            // copyArray(wifiClientArray2[i]["ssid"], ssid[i]);
            // copyArray(wifiClientArray2[i]["password"], password[i]);
            active[i]= wifiClientArray2[i]["active"];
            
            // serializeJson(wifiClientArray2[i]["ssid"] , Serial);
            // Serial.println("");
          }
        }
      }
    }

    // Close the file (File's destructor doesn't close the file)
    file.close();
  }
;