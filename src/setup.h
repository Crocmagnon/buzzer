#ifndef __SETUP_H__
#define __SETUP_H__

#include <ESPAsyncWebServer.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <Audio.h>

void setupScreen();
void setupSPIFFS();
void setupSDCard();
void setupAudio();
void selectDefaultFile();
void setupWifi();
void setupWebServer();

#endif
