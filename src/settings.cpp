#include "settings.h"
#include "config.h"
#include "EEPROM.h"

// Global settings variables
String ssid = "";
String password = "";
volatile int volume = 5;
int currentStream = 0;
bool backlightAlwaysOn = true;
bool radioPowerOn = true;

void initializeEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  Serial.println("EEPROM initialized");
}

void saveSettings() {
  Settings settings;
  settings.version = SETTINGS_VERSION;
  settings.volume = volume;
  settings.currentStream = currentStream;
  settings.backlightAlwaysOn = backlightAlwaysOn;
  settings.radioPowerOn = radioPowerOn;
  
  // Save WiFi credentials
  strncpy(settings.wifiSSID, ssid.c_str(), sizeof(settings.wifiSSID) - 1);
  settings.wifiSSID[sizeof(settings.wifiSSID) - 1] = '\0';
  strncpy(settings.wifiPassword, password.c_str(), sizeof(settings.wifiPassword) - 1);
  settings.wifiPassword[sizeof(settings.wifiPassword) - 1] = '\0';
  
  EEPROM.put(0, settings);
  EEPROM.commit();
}

void loadSettings() {
  Settings settings;
  EEPROM.get(0, settings);
  
  // Check if EEPROM has valid settings
  if (settings.version == SETTINGS_VERSION) {
    // Load saved settings
    volume = settings.volume;
    currentStream = settings.currentStream;
    backlightAlwaysOn = settings.backlightAlwaysOn;
    radioPowerOn = settings.radioPowerOn;
    
    // Load WiFi credentials
    ssid = String(settings.wifiSSID);
    password = String(settings.wifiPassword);
    
    // Validate loaded values
    if (volume < 0) volume = 5;
    if (volume > 80) volume = 80;
    
    Serial.println("Settings loaded from EEPROM:");
    Serial.print("  Volume: ");
    Serial.println(volume);
    Serial.print("  Stream: ");
    Serial.println(currentStream);
    Serial.print("  Backlight Always On: ");
    Serial.println(backlightAlwaysOn ? "true" : "false");
    Serial.print("  Radio Power On: ");
    Serial.println(radioPowerOn ? "true" : "false");
    Serial.print("  WiFi SSID: ");
    Serial.println(ssid.length() > 0 ? ssid : "Not configured");
    Serial.print("  WiFi Password: ");
    Serial.println(password.length() > 0 ? "[Configured]" : "Not configured");
  } else {
    // First time or version mismatch, use defaults and save them
    Serial.println("No valid settings found, using defaults");
    volume = 5;
    currentStream = 0;
    backlightAlwaysOn = true;
    radioPowerOn = true;
    ssid = "";
    password = "";
    saveSettings();
  }
}
