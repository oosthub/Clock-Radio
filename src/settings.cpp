#include "settings.h"
#include "config.h"
#include "EEPROM.h"

// Global settings variables
String ssid = "";
String password = "";
String weatherApiKey = "";
volatile int volume = 5;
int currentStream = 0;
bool backlightAlwaysOn = true;
bool radioPowerOn = true;

// Global alarm variables
Alarm alarms[5];

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
  
  // Save weather API key
  strncpy(settings.weatherApiKey, weatherApiKey.c_str(), sizeof(settings.weatherApiKey) - 1);
  settings.weatherApiKey[sizeof(settings.weatherApiKey) - 1] = '\0';
  
  // Save alarms
  for (int i = 0; i < 5; i++) {
    settings.alarms[i] = alarms[i];
  }
  
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
    
    // Load weather API key
    weatherApiKey = String(settings.weatherApiKey);
    
    // Load alarms
    for (int i = 0; i < 5; i++) {
      alarms[i] = settings.alarms[i];
      
      // Validate alarm data to prevent corruption
      if (alarms[i].hour < 0 || alarms[i].hour > 23) {
        Serial.print("Alarm ");
        Serial.print(i + 1);
        Serial.println(" hour corrupted, resetting to defaults");
        alarms[i] = Alarm();
        snprintf(alarms[i].label, sizeof(alarms[i].label), "Alarm %d", i + 1);
      }
      
      if (alarms[i].minute < 0 || alarms[i].minute > 59) {
        Serial.print("Alarm ");
        Serial.print(i + 1);
        Serial.println(" minute corrupted, resetting to defaults");
        alarms[i] = Alarm();
        snprintf(alarms[i].label, sizeof(alarms[i].label), "Alarm %d", i + 1);
      }
      
      if (alarms[i].maxVolume < 1 || alarms[i].maxVolume > 80) {
        Serial.print("Alarm ");
        Serial.print(i + 1);
        Serial.println(" volume corrupted, resetting to defaults");
        alarms[i] = Alarm();
        snprintf(alarms[i].label, sizeof(alarms[i].label), "Alarm %d", i + 1);
      }
      
      if (alarms[i].schedule < 0 || alarms[i].schedule >= ALARM_SCHEDULE_COUNT) {
        Serial.print("Alarm ");
        Serial.print(i + 1);
        Serial.println(" schedule corrupted, resetting to defaults");
        alarms[i] = Alarm();
        snprintf(alarms[i].label, sizeof(alarms[i].label), "Alarm %d", i + 1);
      }
      
      if (alarms[i].autoOff < 0 || alarms[i].autoOff >= AUTO_OFF_COUNT) {
        Serial.print("Alarm ");
        Serial.print(i + 1);
        Serial.println(" auto-off corrupted, resetting to defaults");
        alarms[i] = Alarm();
        snprintf(alarms[i].label, sizeof(alarms[i].label), "Alarm %d", i + 1);
      }
    }
    
    // Validate loaded values
    if (volume < 0) volume = 5;
    if (volume > 80) volume = 80;
    
    Serial.println("Settings loaded from EEPROM:");
    Serial.print("  Version: ");
    Serial.println(settings.version);
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
    Serial.print("  Weather API Key: ");
    Serial.println(weatherApiKey.length() > 0 ? "[Configured]" : "Not configured");
    
    // Debug alarm data
    Serial.println("  Alarm status:");
    for (int i = 0; i < 5; i++) {
      Serial.print("    Alarm ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(alarms[i].enabled ? "ON " : "OFF");
      Serial.print(" ");
      Serial.printf("%02d:%02d", alarms[i].hour, alarms[i].minute);
      Serial.print(" Station:");
      Serial.print(alarms[i].stationIndex);
      Serial.print(" Vol:");
      Serial.print(alarms[i].maxVolume);
      Serial.print(" AutoOff:");
      Serial.println(alarms[i].autoOff);
    }
  } else {
    // First time, version mismatch, or upgrade from older version
    Serial.print("Settings version mismatch or first time. Found version: ");
    Serial.print(settings.version);
    Serial.print(", expected: ");
    Serial.println(SETTINGS_VERSION);
    
    // Load what we can from older versions
    if (settings.version > 0 && settings.version < SETTINGS_VERSION) {
      // Migrate from older version - load basic settings
      volume = settings.volume;
      currentStream = settings.currentStream;
      backlightAlwaysOn = settings.backlightAlwaysOn;
      radioPowerOn = settings.radioPowerOn;
      ssid = String(settings.wifiSSID);
      password = String(settings.wifiPassword);
      weatherApiKey = String(settings.weatherApiKey);
      Serial.println("Migrated basic settings from older version");
    } else {
      // First time or corrupt data, use defaults
      volume = 5;
      currentStream = 0;
      backlightAlwaysOn = true;
      radioPowerOn = true;
      ssid = "";
      password = "";
      weatherApiKey = "";
      Serial.println("Using default settings");
    }
    
    // Initialize alarms with defaults
    for (int i = 0; i < 5; i++) {
      alarms[i] = Alarm();  // Uses constructor defaults
      snprintf(alarms[i].label, sizeof(alarms[i].label), "Alarm %d", i + 1);
    }
    
    // Save the updated settings
    saveSettings();
    Serial.println("Settings saved with new version");
  }
}

void resetAlarm(int alarmIndex) {
  if (alarmIndex >= 0 && alarmIndex < 5) {
    alarms[alarmIndex] = Alarm();  // Reset to default values
    snprintf(alarms[alarmIndex].label, sizeof(alarms[alarmIndex].label), "Alarm %d", alarmIndex + 1);
    saveSettings();
    
    Serial.print("Alarm ");
    Serial.print(alarmIndex + 1);
    Serial.println(" reset to defaults");
  }
}
