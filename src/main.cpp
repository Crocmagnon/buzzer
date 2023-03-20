#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ESPAsyncWebServer.h>
#include <Audio.h>
#include <Preferences.h>

#include "setup.h"
#include "utils.h"
#include "config.h"

bool wasRunning = false;

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

  setCpuFrequencyMhz(80);
  
  // Setup is done, light up the LED
  delay(500);
  Serial.println("All setup & ready to go!");
  digitalWrite(LED, HIGH);
  updateLastAction();
  wasRunning = audio.isRunning();
}

void loop()
{
  checkButtonAndPlay();

  audio.loop();

  bool running = audio.isRunning();
  if (running && !wasRunning)
  {
    wasRunning = true;
    setCpuFrequencyMhz(240);
  }
  else if (!running && wasRunning)
  {
    wasRunning = false;
    updateLastAction();
    setCpuFrequencyMhz(80);
  }

  if (!running && ((millis() - lastActionTime) > DEEP_SLEEP_DELAY_MS))
  {
    deepSleep();
  }
}
