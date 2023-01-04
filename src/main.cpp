#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <AsyncJson.h>
#include <Audio.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

// Toggle on to switch to AP mode.
// Leave commented for wifi station mode.
// #define B_WIFI_AP

#ifdef B_WIFI_AP
#include "creds_ap.h"
#else
#include "creds.h"
#endif

// DAC
#define I2S_DOUT 32
#define I2S_BCLK 25
#define I2S_LRC 27

// SD CARD
#define SPI_MISO 18
#define SPI_MOSI 19
#define SPI_SCK 23
#define SD_CS 5

#define LED 2
#define BUTTON 33

#define SSD1306_NO_SPLASH
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SCREEN_MSG_X 0
#define SCREEN_MSG_Y 24

#define VOLUME_MIN 0
#define VOLUME_MAX 21

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

String selectedFile = "";

AsyncWebServer server(80);
Audio audio;
Preferences preferences;

byte buttonLastState = HIGH;
byte currentVolume = 12;

bool fileIsValid(String fileName)
{
  return !fileName.startsWith(".") && (fileName.endsWith(".mp3") || fileName.endsWith(".wav"));
}

void clearMessageArea()
{
  display.fillRect(SCREEN_MSG_X, SCREEN_MSG_Y, SCREEN_WIDTH - SCREEN_MSG_X, SCREEN_HEIGHT - SCREEN_MSG_Y, BLACK);
  display.setCursor(SCREEN_MSG_X, SCREEN_MSG_Y);
}

void displayText(String text)
{
  clearMessageArea();
  display.println(text);
  display.display();
}

void play()
{
  String path = "/" + selectedFile;
  Serial.println("Playing file: " + path);
  audio.connecttoFS(SD, path.c_str());
}

void onStop(AsyncWebServerRequest *request)
{
  audio.stopSong();
  request->send(200);
}

void onPlay(AsyncWebServerRequest *request)
{
  play();
  request->send(200);
}

void onStatus(AsyncWebServerRequest *request)
{
  Serial.println("Status");
  AsyncResponseStream *response = request->beginResponseStream("application/json");

  DynamicJsonDocument root(4096);
  JsonObject files = root.createNestedObject("files");
  files["selectedIndex"] = -1;
  files["moreNotShown"] = false;

  JsonObject volume = root.createNestedObject("volume");
  volume["current"] = currentVolume;
  volume["canDecrease"] = currentVolume > 0;
  volume["canIncrease"] = currentVolume < 21;

  JsonArray availableFiles = files.createNestedArray("available");
  File music = SD.open("/");
  File file = music.openNextFile();
  unsigned int index = 0;
  while (file)
  {
    String fileName = file.name();
    if (fileIsValid(fileName))
    {
      availableFiles.add(fileName);
      if (fileName == selectedFile) {
        files["selectedIndex"] = index;
      }
      index++;
    }
    file.close();

    if (root.overflowed())
    {
      files["moreNotShown"] = true;
      break;
    }
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
    preferences.putString("selectedFile", selectedFile);
    Serial.print(selectedFile);
    displayText("Selectionne : " + selectedFile);
  }
  Serial.println();
  onStatus(request);
}

void onChangeVolume(AsyncWebServerRequest *request)
{
  Serial.print("Volume: ");
  if (request->hasParam("modifier", true))
  {
    String s_modifier = request->getParam("modifier", true)->value();
    int modifier = s_modifier.toInt();
    currentVolume += modifier;
    if (currentVolume > VOLUME_MAX)
      currentVolume = VOLUME_MAX;
    else if (currentVolume < VOLUME_MIN)
      currentVolume = VOLUME_MIN;
    preferences.putUChar("currentVolume", currentVolume);
    audio.setVolume(currentVolume);
    Serial.print(currentVolume);
    clearMessageArea();
    display.print("Volume : ");
    display.println(currentVolume);
    display.display();
  }
  Serial.println();
  onStatus(request);
}

void onUpload(AsyncWebServerRequest *request)
{
  Serial.println("onUpload");
  request->send(200);
}

void onUploadFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  Serial.println("onUploadFile");
  if (!index)
  {
    Serial.printf("UploadStart: %s\n", filename.c_str());
    String filePath = "/" + filename;
    request->_tempFile = SD.open(filePath, FILE_WRITE);
  }
  if (!request->_tempFile)
  {
    Serial.println("Couldn't open file.");
    request->redirect("/");
    return;
  }
  if (len)
  {
    Serial.printf("Write to: %s\n", filename.c_str());
    request->_tempFile.write(data, len);
  }

  if (final)
  {
    Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
    request->_tempFile.close();
    request->redirect("/");
  }
}

void onNotFound(AsyncWebServerRequest *request)
{
  Serial.println("not found");
  request->send(400);
}

void setup()
{
  // Setup serial
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  preferences.begin("buzzer", false);

  // Screen
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println("Display init failed");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.println("Chargement...");
  display.display();

  pinMode(BUTTON, INPUT_PULLUP);
  buttonLastState = digitalRead(BUTTON);

  // Setup SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS error. Exiting.");
    display.println("Impossible d'acceder aux fichiers...");
    display.display();
    while (true);
  }

  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  if (!SD.begin(SD_CS))
  {
    Serial.println("Error talking to SD card!");
    display.println("Impossible d'acceder a la carte SD...");
    display.display();
    while (true);
  }

  selectedFile = preferences.getString("selectedFile", "");
  if (selectedFile == "" || !fileIsValid(selectedFile))
  {
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file)
    {
      String fileName = file.name();
      if (fileIsValid(fileName))
      {
        selectedFile = fileName;
        Serial.println("Selected " + fileName);
        display.println("Selectionne : " + fileName);
        display.display();
        break;
      }
      file.close();
      file = root.openNextFile();
    }
    root.close();
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  // Wifi
#ifdef B_WIFI_AP
  Serial.println("Setting up AP...");
  WiFi.softAP(ssid, password);
  String wifiIP = WiFi.softAPIP().toString();
  String wifiMode = "AP";
  display.print("Wifi: ");
  display.println(ssid);
  display.print("Pass: ");
  display.println(password);
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
  display.print("IP:   ");
  display.println(wifiIP);
  display.display();

  // Server
  server.on("/play", HTTP_GET, onPlay);
  server.on("/stop", HTTP_GET, onStop);
  server.on("/status", HTTP_GET, onStatus);
  server.on("/select-file", HTTP_POST, onSelectFile);
  server.on("/change-volume", HTTP_POST, onChangeVolume);
  server.on("/upload", HTTP_POST, onUpload, onUploadFile);
  server.onNotFound(onNotFound);
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
  server.begin();

  Serial.println("Server ready!");
  clearMessageArea();
  display.println("Pret !");
  display.display();

  // Audio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  currentVolume = preferences.getUChar("currentVolume", 12);
  audio.setVolume(currentVolume);

  // Setup is done, light up the LED
  digitalWrite(LED, HIGH);
}

void loop()
{
  byte buttonCurrentState = digitalRead(BUTTON);
  if (buttonCurrentState == LOW && buttonLastState == HIGH)
    play();
  buttonLastState = buttonCurrentState;
  audio.loop();
}
