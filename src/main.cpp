#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <Audio.h>
#include <Preferences.h>

#include "setup.h"
#include "utils.h"
#include "config.h"

byte buttonLastState = HIGH;
long lastDebounceTime = 0;

void setup()
{
  setCpuFrequencyMhz(240);
  // Setup GPIO
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(BUTTON, INPUT_PULLUP);
  buttonLastState = digitalRead(BUTTON);

  Serial.begin(115200);
  Serial.println("Serial... OK");

  preferences.begin("buzzer", false);
  Serial.println("Preferences... OK");

  setupScreen();
  setupSPIFFS();
  setupSDCard();
  setupAudio();
  selectDefaultFile();
  setupWifi();
  setupWebServer();
  diagnosticPrintln("Configuration OK!");

  displayWifiCreds();
  displayStatus();

  // Setup is done, light up the LED
  Serial.println("All setup & ready to go!");
  setCpuFrequencyMhz(80);
  digitalWrite(LED, HIGH);
}

void loop()
{
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY)
  {
    byte buttonCurrentState = digitalRead(BUTTON);
    if (buttonCurrentState == LOW && buttonLastState == HIGH)
      play();
    buttonLastState = buttonCurrentState;
  }
  audio.loop();
}

void audio_info(const char *info){
  String s_info = info;
  s_info.toLowerCase();
  s_info.trim();
  if (s_info == "closing audio file") {
    setCpuFrequencyMhz(80);
  }
  else if (s_info == "stream ready") {
    setCpuFrequencyMhz(240);
  }
}
