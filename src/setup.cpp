#include <SPIFFS.h>
#include <Wire.h>  // Required by display.begin
#include <AsyncElegantOTA.h>
#include <Audio.h>

#ifdef B_WIFI_AP
#include "creds_ap.h"
#else
#include "creds.h"
#endif

#include "config.h"
#include "utils.h"
#include "webHandlers.h"

void setupScreen()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println("Display init failed");
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.println("Ecran... OK");
  display.display();
  Serial.println("Ecran... OK");
}

void setupSPIFFS()
{
  diagnosticPrint("Fichiers... ");
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS error. Exiting.");
    display.println("KO");
    display.display();
    while (true);
  }
  diagnosticPrintln("OK");
}

void setupSDCard()
{
  diagnosticPrint("Carte SD... ");
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  if (!SD.begin(SD_CS))
  {
    Serial.println("Error talking to SD card!");
    display.println("KO");
    display.display();
    while (true);
  }
  diagnosticPrintln("OK");
}

void setupAudio()
{
  diagnosticPrint("Audio... ");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  byte volume = preferences.getUChar(CURRENT_VOLUME, 12);
  audio.setVolume(volume);
  diagnosticPrintln("OK");
}

void selectDefaultFile()
{
  String linePrefix = "Son: ";
  diagnosticPrint(linePrefix);
  String selectedFile = preferences.getString(SELECTED_FILE, "");
  if (selectedFile == "" || !fileIsValid(selectedFile) || !fileExists(selectedFile))
  {
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file)
    {
      String fileName = file.name();
      if (fileIsValid(fileName))
      {
        Serial.println("Selected " + fileName);
        selectedFile = fileName;
        preferences.putString(SELECTED_FILE, fileName);
        break;
      }
      file.close();
      file = root.openNextFile();
    }
    root.close();
  }
  diagnosticPrintln(selectedFile.substring(0, LINE_LENGTH - linePrefix.length()));
}

void setupWifi()
{
  diagnosticPrint("Wifi...");
  preferences.putString(WIFI_SSID, ssid);
  preferences.putString(WIFI_PASSWORD, password);
#ifdef B_WIFI_AP
  WiFi.softAP(ssid, password);
  String wifiIP = WiFi.softAPIP().toString();
#else
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  String wifiIP = WiFi.localIP().toString();
#endif
  diagnosticPrintln(" OK");
  Serial.print("IP: ");
  Serial.println(wifiIP);
  preferences.putString(WIFI_IP, wifiIP);
}

void setupWebServer()
{
  Serial.print("Server... ");
  display.print("Serveur... ");
  display.display();
  server.on("/play", HTTP_GET, onPlay);
  server.on("/stop", HTTP_GET, onStop);
  server.on("/status", HTTP_GET, onStatus);
  server.on("/list-files", HTTP_GET, onListFiles);
  server.on("/select-file", HTTP_POST, onSelectFile);
  server.on("/change-volume", HTTP_POST, onChangeVolume);
  server.on("/upload", HTTP_POST, onUpload, onUploadFile);
  server.onNotFound(onNotFound);
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
  AsyncElegantOTA.begin(&server);
  server.begin();
  Serial.println("OK");
  display.println("OK");
  display.display();
}