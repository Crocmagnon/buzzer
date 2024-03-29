#include <Arduino.h>
#include <Audio.h>
#include <Preferences.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>

#include "config.h"
#include "utils.h"

bool fileIsValid(String fileName)
{
  return !fileName.startsWith(".") && (fileName.endsWith(".mp3") || fileName.endsWith(".wav"));
}

bool fileExists(String fileName)
{
  return SD.exists("/" + fileName);
}

void clearMessageArea()
{
  display.fillRect(SCREEN_MSG_X, SCREEN_MSG_Y, SCREEN_WIDTH - SCREEN_MSG_X, SCREEN_HEIGHT - SCREEN_MSG_Y, BLACK);
  display.setCursor(SCREEN_MSG_X, SCREEN_MSG_Y);
}

void displayText(String text)
{
  clearMessageArea();
  display.println(text);
  display.display();
}

void checkButtonAndPlay() {
  byte buttonCurrentState = digitalRead(BUTTON);
  if (buttonCurrentState == HIGH && buttonLastState == LOW) {
    startPress = millis();
  } else if (buttonCurrentState == LOW && buttonLastState == HIGH) {
    startPress = NOT_PRESSED;
  }
  long pressDuration = millis() - startPress;
  if (startPress != NOT_PRESSED && pressDuration >= MIN_PRESS_DURATION) {
    play();
    startPress = NOT_PRESSED;
  }
  buttonLastState = buttonCurrentState;
}

void play()
{
  updateLastAction();
  String selectedFile = preferences.getString(SELECTED_FILE);
  String path = "/" + selectedFile;
  Serial.println("Playing file: " + path);
  audio.stopSong();
  audio.connecttoFS(SD, path.c_str());
}

void diagnosticPrint(String text)
{
  Serial.print(text);
  display.print(text);
  display.display();
}
void diagnosticPrintln(String text)
{
  Serial.println(text);
  display.println(text);
  display.display();
}

void displayWifiCreds()
{
  display.clearDisplay();
  display.setCursor(0, 0);

  String ssid = preferences.getString(WIFI_SSID, "");
  display.print("Wifi: ");
  display.println(ssid);

  display.print("Pass: ");
#ifdef B_WIFI_AP
  String password = preferences.getString(WIFI_PASSWORD, "");
  display.println(password);
#else
  display.println("****");
#endif

  display.print("IP: ");
  String ip = preferences.getString(WIFI_IP, "");
  display.println(ip);
  display.display();
}

void displayStatus()
{
  clearMessageArea();

  String volume = String(preferences.getUChar(CURRENT_VOLUME));
  display.println("Volume: " + volume);

  String prefix = "Son: ";
  display.println(prefix + preferences.getString(SELECTED_FILE, "").substring(0, LINE_LENGTH - prefix.length()));

  display.println("\nRetourne moi !");

  display.display();
}

void selectFile(String fileName)
{
  preferences.putString(SELECTED_FILE, fileName);
  displayStatus();
  Serial.print("Select new file: ");
  Serial.println(fileName);
}

void deepSleep()
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Veille. Retourne-moi pour me reveiller.");
  display.display();

  Serial.println("Deep sleep!");
  esp_sleep_enable_ext0_wakeup(DEEP_SLEEP_WAKEUP, HIGH);
  digitalWrite(LED, LOW);
  esp_deep_sleep_start();
}

void updateLastAction()
{
  lastActionTime = millis();
}
