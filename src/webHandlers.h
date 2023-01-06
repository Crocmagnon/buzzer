#ifndef __MAIN_H__
#define __MAIN_H__

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>


void onStop(AsyncWebServerRequest *request);
void onPlay(AsyncWebServerRequest *request);
void onStatus(AsyncWebServerRequest *request);
void onListFiles(AsyncWebServerRequest *request);
void onSelectFile(AsyncWebServerRequest *request);
void onChangeVolume(AsyncWebServerRequest *request);
void onUpload(AsyncWebServerRequest *request);
void onUploadFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void onNotFound(AsyncWebServerRequest *request);

#endif