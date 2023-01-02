#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <AsyncJson.h>
#include <Audio.h>
#include "creds.h"

// #define B_WIFI_AP

#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

#define LED 2
#define BUTTON 18

String selectedFile = "";

AsyncWebServer server(80);
Audio audio;

byte buttonLastState = HIGH;

void play()
{
  String path = "/music/" + selectedFile;
  Serial.println("Playing file: " + path);
  audio.connecttoFS(SPIFFS, path.c_str());
}

void onPlay(AsyncWebServerRequest *request)
{
  play();
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
    String fileName = file.name();
    if (!fileName.startsWith("."))
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
  onAvailableFiles(request);
}

void setup()
{
  // Setup serial
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  pinMode(BUTTON, INPUT_PULLUP);

  // Setup SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS error. Exiting.");
    return;
  }

  File root = SPIFFS.open("/music");
  File file = root.openNextFile();
  while (file)
  {
    String fileName = file.name();
    if (!fileName.startsWith("."))
    {
      selectedFile = fileName;
      Serial.println("Selected " + fileName);
      break;
    }
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
    delay(500);
  }
  Serial.println();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif

  // Server
  server.on("/play", HTTP_GET, onPlay);
  server.on("/available-files", HTTP_GET, onAvailableFiles);
  server.on("/select-file", HTTP_POST, onSelectFile);
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404); });
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
  server.begin();

  Serial.println("Server ready!");

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); // Max 21

  digitalWrite(LED, HIGH);
}

void loop()
{
  byte buttonCurrentState = digitalRead(BUTTON);
  if (buttonCurrentState == LOW && buttonLastState == HIGH)
    play();
  buttonLastState = buttonCurrentState;
}
