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

void displayWifiCreds();
void displaySelectedFile();

void diagnosticPrint(String text);
void diagnosticPrintln(String text);

#endif
