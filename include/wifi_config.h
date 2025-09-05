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

// WiFi hotspot configuration
#define HOTSPOT_SSID "OOSIE-Radio"
#define HOTSPOT_PASSWORD ""
#define HOTSPOT_IP IPAddress(192, 168, 4, 1)
#define HOTSPOT_GATEWAY IPAddress(192, 168, 4, 1)
#define HOTSPOT_SUBNET IPAddress(255, 255, 255, 0)

// WiFi configuration mode options
enum WiFiConfigOption {
  WIFI_CONFIG_MANUAL = 0,
  WIFI_CONFIG_HOTSPOT = 1
};

// Function declarations
void updateSelectedCharForPosition();
void resetWiFiConfig();
void updateWiFiConfigDisplay();
bool connectToWiFi();
void configureWiFi();
void setupTime();
bool chooseWiFiConfigMethod();
void startWiFiHotspot();
void showHotspotInstructions();
void stopWiFiHotspot();
bool isHotspotActive();
void configureWiFiPowerManagement();

#endif
