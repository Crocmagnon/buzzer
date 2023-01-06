#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <SD.h>

#include "config.h"
#include "utils.h"

void onStop(AsyncWebServerRequest *request)
{
  Serial.println("Stop playing");
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

  StaticJsonDocument<96> root;
  String file = preferences.getString(SELECTED_FILE, "");
  root["files"]["selected"] = file.c_str();

  JsonObject volume = root.createNestedObject("volume");
  byte currentVolume = preferences.getUChar(CURRENT_VOLUME);
  volume["current"] = currentVolume;
  volume["canDecrease"] = currentVolume > 0;
  volume["canIncrease"] = currentVolume < 21;

  serializeJson(root, *response);
  request->send(response);
}

void onListFiles(AsyncWebServerRequest *request)
{
  Serial.print("List files cursor=");
  int cursor = 0;
  if (request->hasParam("cursor")) {
    String s_cursor = request->getParam("cursor")->value();
    cursor = s_cursor.toInt();
  }
  Serial.println(cursor);

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  StaticJsonDocument<512> root;
  root["next"] = -1;
  JsonArray files = root.createNestedArray("files");
  File music = SD.open("/");
  File file = music.openNextFile();
  int index = 0;
  while (file)
  {
    String fileName = file.name();
    if (fileIsValid(fileName))
    {
      index++;
      if (index >= cursor)
        files.add(fileName);
    }
    file.close();

    if (root.overflowed())
    {
      root["next"] = index;
      break;
    }
    file = music.openNextFile();
  }
  if (root["next"] == -1) {
    root.remove("next");
  }

  serializeJson(root, *response);
  request->send(response);
}

void onSelectFile(AsyncWebServerRequest *request)
{
  Serial.println("Select file");
  if (request->hasParam("fileName", true))
  {
    String selectedFile = request->getParam("fileName", true)->value();
    selectFile(selectedFile);
  }
  onStatus(request);
}

void onChangeVolume(AsyncWebServerRequest *request)
{
  Serial.print("Volume: ");
  if (request->hasParam("modifier", true))
  {
    String s_modifier = request->getParam("modifier", true)->value();
    int modifier = s_modifier.toInt();
    byte currentVolume = preferences.getUChar(CURRENT_VOLUME);
    currentVolume += modifier;
    if (currentVolume > VOLUME_MAX)
      currentVolume = VOLUME_MAX;
    else if (currentVolume < VOLUME_MIN)
      currentVolume = VOLUME_MIN;
    preferences.putUChar(CURRENT_VOLUME, currentVolume);
    audio.setVolume(currentVolume);
    Serial.print(currentVolume);
    String s_volume = String(currentVolume);
    displayText("Volume : " + s_volume);
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
  if (!index)
  {
    Serial.printf("Upload start: %s\n", filename.c_str());
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
    request->_tempFile.write(data, len);
  }

  if (final)
  {
    Serial.printf("Upload end: %s, %u B\n", filename.c_str(), index + len);
    request->_tempFile.close();
    request->redirect("/");
    selectFile(filename);
  }
}

void onNotFound(AsyncWebServerRequest *request)
{
  Serial.println("not found");
  request->send(400);
}
