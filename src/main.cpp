#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char *ssid = "buzzer";
const char *password = "123456789";

const byte led = 2;

AsyncWebServer server(80);

void setup()
{
  // Setup serial
  Serial.begin(115200);
  /*
    Wait for serial monitor to be connected.
    Remove this when running without computer.
   */
  while (!Serial)
  {
  }
  Serial.println("\n");
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
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }

  // Wifi
  Serial.println("Setting up AP...");
  WiFi.softAP(ssid, password);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });
  server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/w3.css", "text/css"); });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/script.js", "text/javascript"); });
  server.on("/play", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              // TODO: play file through speaker
              Serial.println("/play");
              request->send(200); });
  
  server.begin();
  Serial.println("Server ready!");
}

void loop()
{
  // put your main code here, to run repeatedly:
}