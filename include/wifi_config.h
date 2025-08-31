#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include "Arduino.h"

// WiFi configuration variables
extern bool wifiConfigMode;
extern bool configuringSSID;
extern int charIndex;
extern int selectedChar;
extern String inputSSID;
extern String inputPassword;
extern const char charset[];
extern const int charsetSize;

// Function declarations
void updateSelectedCharForPosition();
void resetWiFiConfig();
void updateWiFiConfigDisplay();
bool connectToWiFi();
void configureWiFi();
void setupTime();

#endif
