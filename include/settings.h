#ifndef SETTINGS_H
#define SETTINGS_H

#include "Arduino.h"

// Settings structure for EEPROM storage
struct Settings {
  byte version;
  int volume;
  int currentStream;
  bool backlightAlwaysOn;
  bool radioPowerOn;
  char wifiSSID[32];
  char wifiPassword[64];
  char weatherApiKey[64];
};

// Global settings variables
extern String ssid;
extern String password;
extern String weatherApiKey;
extern volatile int volume;
extern int currentStream;
extern bool backlightAlwaysOn;
extern bool radioPowerOn;

// Function declarations
void initializeEEPROM();
void saveSettings();
void loadSettings();

#endif
