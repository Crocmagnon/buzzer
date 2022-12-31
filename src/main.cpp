#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

const char *ssid = "buzzer";
const char *password = "123456789";

const byte led = 2;
bool ledOn = false;

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
  Serial.println("Setting up AP...");
  WiFi.softAP(ssid, password);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Server
  server.on("/play", HTTP_GET, onPlay);
  server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
  server.begin();
  Serial.println("Server ready!");
}

void loop()
{
}
