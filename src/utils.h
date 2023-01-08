#ifndef __UTILS_H__
#define __UTILS_H__

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

bool fileIsValid(String fileName);
bool fileExists(String fileName);
void clearMessageArea();
void displayText(String text);
void play();

void selectFile(String fileName);

void displayWifiCreds();
void displayStatus();

void diagnosticPrint(String text);
void diagnosticPrintln(String text);

void deepSleep();

void audio_info(const char *info);
void updateLastAction();

#endif
