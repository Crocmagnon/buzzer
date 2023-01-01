#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <AsyncJson.h>
#include "creds.h"

// #define B_WIFI_AP

const byte led = 2;
bool ledOn = false;
String selectedFile = "";

AsyncWebServer server(80);

void onPlay(AsyncWebServerRequest *request)
{
  Serial.println("Toggling LED");
  if (ledOn)
  {
    ledOn = false;
    digitalWrite(led, LOW);
  }
  else
  {
    ledOn = true;
    digitalWrite(led, HIGH);
  }
  request->send(200);
}

void onAvailableFiles(AsyncWebServerRequest *request)
{
  Serial.println("Available files");
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  
  DynamicJsonDocument root(256);
  root["selectedFile"] = selectedFile;
  JsonArray files = root.createNestedArray("files");
  File music = SPIFFS.open("/music");
  File file = music.openNextFile();
  while (file)
  {
    Serial.print("File: ");
    String fileName = file.name();
    Serial.println(fileName);
    files.add(fileName);
    file.close();
    file = music.openNextFile();
  }
  serializeJson(root, *response);
  
  request->send(response);
}

void onSelectFile(AsyncWebServerRequest *request)
{
  Serial.print("Select file: ");
  if (request->hasParam("fileName", true))
  {
    selectedFile = request->getParam("fileName", true)->value();
    Serial.print(selectedFile);
  }
  Serial.println();
  request->send(200);
}

void setup()
{
  // Setup serial
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  // Setup SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS error. Exiting.");
    return;
  }

  // List existing files
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while (file)
  {
    Serial.print("File: ");
    Serial.println(file.path());
    file.close();
    file = root.openNextFile();
  }

// Wifi
#ifdef B_WIFI_AP
  Serial.println("Setting up AP...");
  WiFi.softAP(ssid, password);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
#else
  Serial.print("Connecting to wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif

  // Server
  server.on("/play", HTTP_GET, onPlay);
  server.on("/available-files", HTTP_GET, onAvailableFiles);
  server.on("/select-file", HTTP_POST, onSelectFile);
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
  server.begin();
  Serial.println("Server ready!");
}

void loop()
{
}
