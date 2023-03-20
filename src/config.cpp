#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <Audio.h>
#include <Preferences.h>

#include "config.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server(80);
Audio audio;
Preferences preferences;

long lastActionTime = 0;
byte buttonLastState = HIGH;
long startPress = NOT_PRESSED;
