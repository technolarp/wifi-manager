#include <Arduino.h>


// ASYNCWEBSERVER
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <IPAddress.h>

AsyncWebServer server(80);

// LITTLEFS
#include <LittleFS.h>

// CONFIG WIFI
#define SIZE_ARRAY 20
#define WIFI_CLIENTS 5

IPAddress apIP;
IPAddress apNetMsk;
char apName[SIZE_ARRAY];
char apPassword[SIZE_ARRAY];

uint8_t wifiConnectDelay = 5;

char ssid[WIFI_CLIENTS][SIZE_ARRAY];
char password[WIFI_CLIENTS][SIZE_ARRAY];
bool active[WIFI_CLIENTS];

bool disableSsid = false;
bool rebootEsp = false;

// ARDUINOJSON
#include <ArduinoJson.h>
JsonDocument doc;

// FASTLED
#include <FastLED.h>
#define NUM_LEDS 8
#define DATA_PIN D4
bool ledState = true;

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
void writeNetworkConfig(const char *filename);
void defaultAPConfig();

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("");
  Serial.println("");
  
  // FASTLED
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  FastLED.clear();
  FastLED.show();
  
  // READ WIFI CONFIG
  mountFS();
  listDir("/config");
  printJsonFile("/config/networkconfig.json");
  readNetworkConfig("/config/networkconfig.json");

  // LOOP TO WIFI CLIENT
  Serial.println(F(""));
  Serial.println(F("connecting to wifi as client"));

  for (uint8_t i = 0; i < WIFI_CLIENTS; i++)
  {
    if (active[i] && strlen(ssid[i])>0)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        Serial.print(F("ssid: "));
        Serial.println(ssid[i]);
        
        WiFi.disconnect(true);
        wifiFlag = true;
        WiFi.begin(ssid[i], password[i]);

        FastLED.clear();
        // Loop continuously while WiFi is not connected
        while ( (WiFi.status() != WL_CONNECTED) && (wifiFlag) )
        {
          delay(100);
          Serial.print("/");
          
          if (ledState)
          {
            leds[i%NUM_LEDS] = CRGB::Blue;
          }
          else
          {
            leds[i%NUM_LEDS] = CRGB::Black;
          }
          FastLED.show();
          ledState = !ledState;
          

          if (millis() - previousMillisHB > (wifiConnectDelay*1000) )
          {
            previousMillisHB = millis();
            wifiFlag = false;
          }
        }
      }

      Serial.println(F(" "));
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.print(F("connected to "));
        Serial.println(ssid[i]);
        Serial.print(F("IP address: "));
        Serial.println(WiFi.localIP());
      }
      else
      {
        if (disableSsid)
        {
          Serial.print(F("disable this ssid: "));
          Serial.println(ssid[i]);
          active[i]=false;
          writeNetworkConfig("/config/networkconfig.json");
        }
        if (rebootEsp)
        {
          Serial.println(F("reboot"));
          delay(1000);
          ESP.restart();
        }        
      }
    }    
  }
  Serial.println(" ");
  
  // WIFI AP MODE
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(F("failed to connect to wifi as client, creating a wifi AP: "));
    Serial.println(apName);

    // WiFi.mode(WIFI_AP_STA);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP,apIP,apNetMsk);
    bool apRC = WiFi.softAP(apName, apPassword);

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

  // ASYNC WEBSERVER
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello, world");
  });

  server.begin();

  // HEARTBEAT
  previousMillisHB = millis();
  intervalHB = 500;

  Serial.println(F("START !!!"));
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

    if (indexLed == 0)
    {
      Serial.println(F("."));
    }
    else
    {
      Serial.print(F("."));
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

      Serial.println(F("Create default AP config"));
      defaultAPConfig();
    }
    else
    {
      // Copy values from the JsonObject to the Config
      // parse wifi ssid list
      if (doc["wifiClientsConfig"]["wifiClientsList"].is<JsonVariant>())
      {
        JsonArray wifiClientArray = doc["wifiClientsConfig"]["wifiClientsList"];

        for (uint8_t i = 0; i < WIFI_CLIENTS; i++)
        {
          if (wifiClientArray[i]["ssid"].is<JsonVariant>())
          {
            strlcpy(ssid[i], wifiClientArray[i]["ssid"], SIZE_ARRAY);
            strlcpy(password[i], wifiClientArray[i]["password"], SIZE_ARRAY);
            active[i]=wifiClientArray[i]["active"].as<boolean>();
          }
        }

        wifiClientArray.clear();
      }

      if (doc["wifiClientsConfig"]["wifiConnectDelay"].is<unsigned short>())
      {
        wifiConnectDelay=doc["wifiClientsConfig"]["wifiConnectDelay"];
      }

      if (doc["wifiClientsConfig"]["disableSsid"].is<boolean>())
      {
        disableSsid=doc["wifiClientsConfig"]["disableSsid"].as<boolean>();
      }

      if (doc["wifiClientsConfig"]["rebootEsp"].is<boolean>())
      {
        rebootEsp=doc["wifiClientsConfig"]["rebootEsp"].as<boolean>();
      }

      // parse wifi AP config
      if (doc["wifiAPConfig"]["apIP"].is<JsonVariant>())
      {
        JsonArray apIPArray = doc["wifiAPConfig"]["apIP"];

        for (uint8_t i = 0; i < 4; i++)
        {
          apIP[i] = apIPArray[i];
        }

        apIPArray.clear();
      }

      if (doc["wifiAPConfig"]["apNetMsk"].is<JsonVariant>())
      {
        JsonArray apNetMskArray = doc["wifiAPConfig"]["apNetMsk"];

        for (uint8_t i = 0; i < 4; i++)
        {
          apNetMsk[i] = apNetMskArray[i];
        }

        apNetMskArray.clear();
      }

      if (doc["wifiAPConfig"]["apName"].is<JsonVariant>())
      {
        strlcpy(apName,
                doc["wifiAPConfig"]["apName"],
                SIZE_ARRAY);
      }

      if (doc["wifiAPConfig"]["apPassword"].is<JsonVariant>())
      {
        strlcpy(apPassword,
                doc["wifiAPConfig"]["apPassword"],
                SIZE_ARRAY);
      }

      // check AP config & create default config 
      if (strlen(apName) == 0)
      {
        Serial.println(F("no apName in config, creating default"));
        defaultAPConfig();
      }
    }
    doc.clear();

    // Close the file
    file.close();
  }

  void writeNetworkConfig(const char *filename)
  {
    // Delete existing file, otherwise the configuration is appended to the file
    LittleFS.remove(filename);

    // Open file for writing
    File file = LittleFS.open(filename, "w");
    if (!file)
    {
      Serial.println(F("Failed to create file"));
      return;
    }

    // Allocate a temporary JsonDocument
    JsonDocument doc;

    JsonArray arraySsid = doc["wifiClientsConfig"]["wifiClientsList"].to<JsonArray>();
    for (uint8_t i = 0; i < WIFI_CLIENTS; i++)
    {
      JsonDocument docSsid;
      docSsid["ssid"] = ssid[i];
      docSsid["password"] = password[i];
      docSsid["active"] = active[i];

      arraySsid.add(docSsid);
    }

    doc["wifiClientsConfig"]["wifiConnectDelay"] = wifiConnectDelay;
    doc["wifiClientsConfig"]["disableSsid"] = disableSsid;
    doc["wifiClientsConfig"]["rebootEsp"] = rebootEsp;
    
    doc["wifiAPConfig"]["apName"] = apName;
    doc["wifiAPConfig"]["apPassword"] = apPassword;

    JsonArray arrayIp = doc["wifiAPConfig"]["apIP"].to<JsonArray>();
    for (uint8_t i = 0; i < 4; i++)
    {
      arrayIp.add(apIP[i]);
    }

    JsonArray arrayNetMask = doc["wifiAPConfig"]["apNetMsk"].to<JsonArray>();
    for (uint8_t i = 0; i < 4; i++)
    {
      arrayNetMask.add(apNetMsk[i]);
    }

    // Serial.println("doc json");
    // serializeJson(doc, Serial);
    // Serial.println();    

    // Serialize JSON to file
    if (serializeJson(doc, file) == 0)
    {
      Serial.println(F("Failed to write to file"));
    }

    // Close the file (File's destructor doesn't close the file)
    file.close();
  }

  void defaultAPConfig()
  {
    // default AP config
    apIP[0] = 192;
    apIP[1] = 168;
    apIP[2] = 1;
    apIP[3] = 1;

    apNetMsk[0] = 255;
    apNetMsk[1] = 255;
    apNetMsk[2] = 255;
    apNetMsk[3] = 0;

    snprintf(apName, SIZE_ARRAY, "TECHNOLARP_%04d", (ESP.getChipId() & 0xFFFF));
    strlcpy(apPassword, "", SIZE_ARRAY);
  }