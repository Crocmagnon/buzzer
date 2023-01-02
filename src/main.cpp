#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <AsyncJson.h>
#include <Audio.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "creds.h"

// #define B_WIFI_AP

#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

#define LED 2
#define BUTTON 18

#define SSD1306_NO_SPLASH
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
  String wifiIP = WiFi.softAPIP().toString();
  String wifiMode = "AP";
#else
  Serial.print("Connecting to wifi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  String wifiIP = WiFi.localIP().toString();
  String wifiMode = "client";
#endif
  String wifiMessage = wifiMode + " IP: " + wifiIP;
  Serial.println(wifiMessage);

  // Server
  server.on("/play", HTTP_GET, onPlay);
  server.on("/available-files", HTTP_GET, onAvailableFiles);
  server.on("/select-file", HTTP_POST, onSelectFile);
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404); });
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
  server.begin();

  Serial.println("Server ready!");

  // Audio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(21); // Max 21

  // Screen

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    return;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(wifiMessage);
  display.display();

  // Setup is done, light up the LED
  digitalWrite(LED, HIGH);
}

void loop()
{
  byte buttonCurrentState = digitalRead(BUTTON);
  if (buttonCurrentState == LOW && buttonLastState == HIGH)
    play();
  buttonLastState = buttonCurrentState;
}
