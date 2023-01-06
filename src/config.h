#ifndef __CONFIG_H__
#define __CONFIG_H__

// Uncomment to switch to AP mode.
// Leave commented for wifi station mode.
// #define B_WIFI_AP

// DAC
#define I2S_DOUT 32
#define I2S_BCLK 25
#define I2S_LRC 27

// SD CARD
#define SPI_MISO 18
#define SPI_MOSI 19
#define SPI_SCK 23
#define SD_CS 5

// GPIO
#define LED 2
#define BUTTON 33

#define DEBOUNCE_DELAY 1000

// Screen
#define SSD1306_NO_SPLASH
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SCREEN_MSG_X 0
#define SCREEN_MSG_Y 24

#define LINE_LENGTH 21

// Volume
#define VOLUME_MIN 0
#define VOLUME_MAX 21

// Preference keys
#define SELECTED_FILE "selectedFile"
#define CURRENT_VOLUME "currentVolume"
#define WIFI_IP "wifiIP"
#define WIFI_SSID "wifiSSID"
#define WIFI_PASSWORD "wifiPassword"

#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <Audio.h>
#include <Preferences.h>

extern Adafruit_SSD1306 display;
extern AsyncWebServer server;
extern Audio audio;
extern Preferences preferences;

#endif