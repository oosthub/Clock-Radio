#include "menu.h"
#include "config.h"
#include "settings.h"
#include "display.h"
#include "alarm.h"
#include "ota_update.h"
#include "Audio.h"
#include "WiFi.h"
#include "ArduinoJson.h"
#include "SPIFFS.h"
#include "weather.h"

// Menu variables
MenuState currentMenu = MENU_SLEEP;
bool inMenu = false;
unsigned long lastMenuActivity = 0;
SleepMenuState currentSleepMenu = SLEEP_MENU_BLANK;
WiFiMenuState currentWiFiMenu = WIFI_MENU_IP;
WeatherMenuState currentWeatherMenu = WEATHER_MENU_TEMPERATURE;
SystemMenuState currentSystemMenu = SYSTEM_MENU_FIRMWARE;
AlarmMenuState currentAlarmMenu = ALARM_MENU_SLOT1;
AlarmSubMenuState currentAlarmSubMenu = ALARM_SUB_BACK;
int currentAlarmSlot = 0;
bool inAlarmSubMenu = false;
bool editingAlarmOption = false;
bool editingTime = false;
bool editingHours = false;
bool editingMinutes = false;
bool showingConfirmation = false;
bool confirmationChoice = false;
bool brightnessChanged = false;
int playingStream = 0;

// Sleep timer variables
unsigned long sleepTimerStart = 0;
unsigned long sleepTimerDuration = 0;
bool sleepTimerActive = false;

// Dynamic stream storage for menu system
RadioStream menuStreams[MAX_MENU_STREAMS];
int menuStreamCount = 0;

// External audio object (defined in main.cpp)
extern Audio audio;

void createDefaultStreamsFile() {
  DynamicJsonDocument doc(2048);
  JsonArray array = doc.to<JsonArray>();
  
  // Default stream data
  const char* defaultStreams[][2] = {
    {"Jacaranda FM", "https://edge.iono.fm/xice/jacarandafm_live_medium.aac"},
    {"Pretoria FM", "https://edge.iono.fm/xice/362_medium.aac"},
    {"Lekker FM", "https://zas3.ndx.co.za:8002/stream"},
    {"Groot FM", "https://edge.iono.fm/xice/330_medium.aac"},
    {"RSG", "https://28553.live.streamtheworld.com/RSGAAC.aac"}
  };
  
  // Add default streams using loop
  for (int i = 0; i < 5; i++) {
    JsonObject stream = array.createNestedObject();
    stream["name"] = defaultStreams[i][0];
    stream["url"] = defaultStreams[i][1];
  }
  
  // Save to file
  File file = SPIFFS.open("/streams.json", "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
    Serial.println("Default streams file created successfully");
  } else {
    Serial.println("Failed to create default streams file");
  }
}

void loadDefaultStreamsToMemory() {
  // Fallback: load default streams directly to memory
  menuStreamCount = 5;
  strcpy(menuStreams[0].name, "Jacaranda FM");
  strcpy(menuStreams[0].url, "https://edge.iono.fm/xice/jacarandafm_live_medium.aac");
  strcpy(menuStreams[1].name, "Pretoria FM");
  strcpy(menuStreams[1].url, "https://edge.iono.fm/xice/362_medium.aac");
  strcpy(menuStreams[2].name, "Lekker FM");
  strcpy(menuStreams[2].url, "https://zas3.ndx.co.za:8002/stream");
  strcpy(menuStreams[3].name, "Groot FM");
  strcpy(menuStreams[3].url, "https://edge.iono.fm/xice/330_medium.aac");
  strcpy(menuStreams[4].name, "RSG");
  strcpy(menuStreams[4].url, "https://28553.live.streamtheworld.com/RSGAAC.aac");
  Serial.println("Default streams loaded to memory as fallback");
}

void loadMenuStreamsFromFile() {
  File file = SPIFFS.open("/streams.json", "r");
  if (!file) {
    Serial.println("No streams.json file found, creating default streams file");
    createDefaultStreamsFile();
    // Now try to load the file we just created
    file = SPIFFS.open("/streams.json", "r");
    if (!file) {
      Serial.println("Failed to create default streams file, using memory fallback");
      loadDefaultStreamsToMemory();
      return;
    }
  }
  
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.println("Failed to parse streams.json for menu, using memory fallback");
    loadDefaultStreamsToMemory();
    return;
  }
  
  menuStreamCount = 0;
  JsonArray array = doc.as<JsonArray>();
  for (JsonObject stream : array) {
    if (menuStreamCount >= MAX_MENU_STREAMS) break;
    
    const char* name = stream["name"];
    const char* url = stream["url"];
    
    if (name && url) {
      strncpy(menuStreams[menuStreamCount].name, name, 16);
      menuStreams[menuStreamCount].name[16] = '\0'; // Ensure null termination
      strncpy(menuStreams[menuStreamCount].url, url, 255);
      menuStreams[menuStreamCount].url[255] = '\0'; // Ensure null termination
      menuStreamCount++;
    }
  }
  
  Serial.print("Loaded ");
  Serial.print(menuStreamCount);
  Serial.println(" streams for menu from JSON file");
}

void enterMenu() {
  inMenu = true;
  lastMenuActivity = millis();
  displayJustWokenUp = false; // Reset wake-up state when entering menu
  
  // Reset sleep menu to blank option when entering menu system
  if (currentMenu == MENU_SLEEP) {
    currentSleepMenu = SLEEP_MENU_BLANK;
  }
  
  // Reset alarm menu state when entering menu system (timeout recovery)
  if (currentMenu == MENU_ALARMS) {
    inAlarmSubMenu = false;
    editingAlarmOption = false;
    editingTime = false;
    editingHours = false;
    editingMinutes = false;
    currentAlarmSubMenu = ALARM_SUB_BACK;
    // Keep currentAlarmMenu and currentAlarmSlot as they were
  }
  
  printCurrentMenu();
  forceImmediateLcdUpdate = true;
}

void exitMenu() {
  inMenu = false;
  showingConfirmation = false;
  displayJustWokenUp = false; // Reset wake-up state when exiting menu
  Serial.println("Exiting menu - Volume control active");
  forceImmediateLcdUpdate = true;
}

void nextMenuItem() {
  currentMenu = (MenuState)((currentMenu + 1) % MENU_COUNT);
  
  // Reset sleep menu to blank option when entering sleep menu
  if (currentMenu == MENU_SLEEP) {
    currentSleepMenu = SLEEP_MENU_BLANK;
  }
  
  // Reset alarm menu when entering alarm menu
  if (currentMenu == MENU_ALARMS) {
    currentAlarmMenu = ALARM_MENU_BLANK;
    currentAlarmSlot = 0;
  }
  
  printCurrentMenu();
  forceImmediateLcdUpdate = true;
}

void printCurrentMenu() {
  switch (currentMenu) {
    case MENU_SLEEP:
      Serial.println("MENU: Sleep");
      break;
    case MENU_STREAMS:
      Serial.println("MENU: Streams");
      Serial.print("Current Stream: ");
      if (menuStreamCount > 0) {
        Serial.print(menuStreams[currentStream].name);
        if (currentStream == playingStream) {
          Serial.println(" (PLAYING)");
        } else {
          Serial.println("");
        }
      } else {
        Serial.println("No streams available");
      }
      break;
    case MENU_BRIGHTNESS:
      Serial.println("MENU: Brightness");
      Serial.print("Current Mode: ");
      Serial.print(backlightAlwaysOn ? "ALWAYS ON" : "AUTO OFF");
      Serial.println("");
      break;
    case MENU_WIFI:
      Serial.println("MENU: WiFi");
      Serial.print("SSID: ");
      Serial.println(ssid.length() > 0 ? ssid : "Not configured");
      Serial.print("Password: ");
      Serial.println(password.length() > 0 ? "[Configured]" : "Not configured");
      break;
    case MENU_WEATHER:
      Serial.println("MENU: Weather");
      Serial.print("Location: ");
      Serial.println(weatherLocation.length() > 0 ? weatherLocation : "Unknown");
      Serial.print("Temperature: ");
      Serial.print(currentWeather.temperature);
      Serial.println("°C");
      Serial.print("Description: ");
      Serial.println(currentWeather.description);
      Serial.print("Humidity: ");
      Serial.print(currentWeather.humidity);
      Serial.println("%");
      Serial.print("API Key: ");
      Serial.println(weatherApiKey.length() > 0 ? "[Configured]" : "Not configured");
      break;
    case MENU_ALARMS:
      Serial.print("MENU: Alarms - Slot ");
      Serial.print(currentAlarmSlot + 1);
      Serial.print("/5 - ");
      Serial.print(alarms[currentAlarmSlot].enabled ? "ENABLED" : "DISABLED");
      Serial.print(" - ");
      Serial.printf("%02d:%02d", alarms[currentAlarmSlot].hour, alarms[currentAlarmSlot].minute);
      Serial.println("");
      break;
    case MENU_SYSTEM:
      Serial.println("MENU: System");
      Serial.print("Firmware Version: ");
      Serial.println(FIRMWARE_VERSION);
      break;
  }
}

void displayCurrentMenu() {
  switch (currentMenu) {
    case MENU_SLEEP: {
      lcd.setCursor(0, 0);
      lcd.print("MENU: Sleep");
      lcd.setCursor(0, 1);
      switch (currentSleepMenu) {
        case SLEEP_MENU_BLANK:
          lcd.print("                "); // Print blank line (16 spaces)
          break;
        case SLEEP_MENU_OFF:
          lcd.print("OFF");
          break;
        case SLEEP_MENU_15MIN:
          lcd.print("15 minutes");
          break;
        case SLEEP_MENU_30MIN:
          lcd.print("30 minutes");
          break;
        case SLEEP_MENU_60MIN:
          lcd.print("60 minutes");
          break;
        case SLEEP_MENU_90MIN:
          lcd.print("90 minutes");
          break;
        case SLEEP_MENU_5MIN:
          lcd.print("5 minutes");
          break;
      }
      break;
    }
    case MENU_STREAMS: {
      lcd.setCursor(0, 0);
      lcd.print("MENU: Station");
      lcd.setCursor(0, 1);
      if (menuStreamCount > 0) {
        String streamName = menuStreams[currentStream].name;
        if (streamName.length() > 16) {
          streamName = streamName.substring(0, 16);
        }
        lcd.print(streamName);
      } else {
        lcd.print("No streams");
      }
      break;
    }
    case MENU_BRIGHTNESS: {
      lcd.setCursor(0, 0);
      lcd.print("MENU: Backlight");
      lcd.setCursor(0, 1);
      lcd.print("Mode: ");
      lcd.print(backlightAlwaysOn ? "ALWAYS ON" : "AUTO OFF");
      break;
    }
    case MENU_WIFI: {
      if (showingConfirmation) {
        lcd.setCursor(0, 0);
        lcd.print("Reset WiFi?");
        lcd.setCursor(0, 1);
        lcd.print(confirmationChoice ? "> YES    NO" : "  YES  > NO");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("MENU: WiFi");
        lcd.setCursor(0, 1);
        switch (currentWiFiMenu) {
          case WIFI_MENU_IP: {
            if (WiFi.status() == WL_CONNECTED) {
              String ip = WiFi.localIP().toString();
              if (ip.length() > 16) {
                // Truncate IP if too long
                ip = ip.substring(0, 16);
              }
              lcd.print(ip);
            } else {
              lcd.print("Not connected");
            }
            break;
          }
          case WIFI_MENU_SSID: {
            lcd.print("SSID: ");
            String displaySSID = ssid;
            if (displaySSID.length() > 10) {
              displaySSID = displaySSID.substring(0, 10);
            }
            lcd.print(displaySSID);
            break;
          }
          case WIFI_MENU_PASSWORD: {
            lcd.print("PASS: ");
            if (password.length() > 0) {
              lcd.print("*****");
            }
            break;
          }
          case WIFI_MENU_RESET: {
            lcd.print("Reset WiFi");
            break;
          }
        }
      }
      break;
    }
    case MENU_WEATHER: {
      // Weather menu display is now handled in display.cpp
      break;
    }
    case MENU_SYSTEM: {
      lcd.setCursor(0, 0);
      lcd.print("MENU: System");
      lcd.setCursor(0, 1);
      switch (currentSystemMenu) {
        case SYSTEM_MENU_FIRMWARE:
          lcd.print("Firm: ");
          lcd.print(FIRMWARE_VERSION);
          break;
        case SYSTEM_MENU_UPDATE:
          lcd.print("Update");
          break;
      }
      break;
    }
  }
}

void selectStream() {
  if (menuStreamCount == 0) {
    Serial.println("No streams available to select");
    return;
  }
  
  Serial.print("Connecting to: ");
  Serial.println(menuStreams[currentStream].name);
  
  // Only actually connect if radio is powered on
  if (radioPowerOn) {
    audio.connecttohost(menuStreams[currentStream].url);
    playingStream = currentStream;
    isStreaming = true;
    currentStreamName = menuStreams[currentStream].name;
  } else {
    Serial.println("Radio is OFF - stream selection saved but not playing");
    isStreaming = false;
  }
  
  saveSettings();
}

void resetWiFiSettings() {
  // Clear WiFi credentials in EEPROM
  ssid = "";
  password = "";
  saveSettings();
  
  // Show reset message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Reset");
  lcd.setCursor(0, 1);
  lcd.print("Rebooting...");
  delay(2000);
  
  Serial.println("WiFi settings reset - rebooting");
  ESP.restart();
}

void handleMenuEncoderClockwise(unsigned long currentTime) {
  if (currentMenu == MENU_WIFI) {
    if (showingConfirmation) {
      confirmationChoice = !confirmationChoice;
      forceImmediateLcdUpdate = true;
    } else {
      currentWiFiMenu = (WiFiMenuState)((currentWiFiMenu + 1) % WIFI_MENU_COUNT);
      forceImmediateLcdUpdate = true;
    }
  } else if (currentMenu == MENU_WEATHER) {
    currentWeatherMenu = (WeatherMenuState)((currentWeatherMenu + 1) % WEATHER_MENU_COUNT);
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_SYSTEM) {
    currentSystemMenu = (SystemMenuState)((currentSystemMenu + 1) % SYSTEM_MENU_COUNT);
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_ALARMS) {
    if (inAlarmSubMenu) {
      if (editingAlarmOption) {
        // We're editing a specific sub-menu option - rotate to change its value
        switch (currentAlarmSubMenu) {
          case ALARM_SUB_ENABLED:
            alarms[currentAlarmSlot].enabled = !alarms[currentAlarmSlot].enabled;
            break;
          case ALARM_SUB_TIME:
            if (editingTime) {
              if (editingHours) {
                // Increment hour
                alarms[currentAlarmSlot].hour = (alarms[currentAlarmSlot].hour + 1) % 24;
              } else if (editingMinutes) {
                // Increment minute
                alarms[currentAlarmSlot].minute++;
                if (alarms[currentAlarmSlot].minute >= 60) {
                  alarms[currentAlarmSlot].minute = 0;
                }
              }
            }
            break;
          case ALARM_SUB_STATION:
            alarms[currentAlarmSlot].stationIndex = (alarms[currentAlarmSlot].stationIndex + 1) % menuStreamCount;
            break;
          case ALARM_SUB_SCHEDULE:
            alarms[currentAlarmSlot].schedule = (AlarmSchedule)((alarms[currentAlarmSlot].schedule + 1) % ALARM_SCHEDULE_COUNT);
            break;
          case ALARM_SUB_VOLUME:
            alarms[currentAlarmSlot].maxVolume += 1;
            if (alarms[currentAlarmSlot].maxVolume > 80) alarms[currentAlarmSlot].maxVolume = 1;
            break;
          case ALARM_SUB_AUTO_OFF:
            alarms[currentAlarmSlot].autoOff = (AlarmAutoOff)((alarms[currentAlarmSlot].autoOff + 1) % AUTO_OFF_COUNT);
            break;
        }
      } else {
        // We're browsing sub-menu options
        currentAlarmSubMenu = (AlarmSubMenuState)((currentAlarmSubMenu + 1) % ALARM_SUB_COUNT);
      }
    } else {
      // We're browsing main alarm slots
      currentAlarmMenu = (AlarmMenuState)((currentAlarmMenu + 1) % ALARM_MENU_COUNT);
      // Update currentAlarmSlot based on the menu selection
      if (currentAlarmMenu >= ALARM_MENU_SLOT1 && currentAlarmMenu <= ALARM_MENU_SLOT5) {
        currentAlarmSlot = currentAlarmMenu; // SLOT1=0, SLOT2=1, etc.
      }
    }
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_STREAMS) {
    currentStream++;
    if (currentStream >= menuStreamCount) currentStream = 0;
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_SLEEP) {
    currentSleepMenu = (SleepMenuState)((currentSleepMenu + 1) % SLEEP_MENU_COUNT);
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_BRIGHTNESS) {
    backlightAlwaysOn = !backlightAlwaysOn;
    brightnessChanged = true;
    lastActivity = currentTime;
    forceImmediateLcdUpdate = true;
  }
}

void handleMenuEncoderCounterClockwise(unsigned long currentTime) {
  if (currentMenu == MENU_WIFI) {
    if (showingConfirmation) {
      confirmationChoice = !confirmationChoice;
      forceImmediateLcdUpdate = true;
    } else {
      currentWiFiMenu = (WiFiMenuState)((currentWiFiMenu - 1 + WIFI_MENU_COUNT) % WIFI_MENU_COUNT);
      forceImmediateLcdUpdate = true;
    }
  } else if (currentMenu == MENU_WEATHER) {
    currentWeatherMenu = (WeatherMenuState)((currentWeatherMenu - 1 + WEATHER_MENU_COUNT) % WEATHER_MENU_COUNT);
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_SYSTEM) {
    currentSystemMenu = (SystemMenuState)((currentSystemMenu - 1 + SYSTEM_MENU_COUNT) % SYSTEM_MENU_COUNT);
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_ALARMS) {
    if (inAlarmSubMenu) {
      if (editingAlarmOption) {
        // We're editing a specific sub-menu option - rotate to change its value
        switch (currentAlarmSubMenu) {
          case ALARM_SUB_ENABLED:
            alarms[currentAlarmSlot].enabled = !alarms[currentAlarmSlot].enabled;
            break;
          case ALARM_SUB_TIME:
            if (editingTime) {
              if (editingHours) {
                // Decrement hour
                alarms[currentAlarmSlot].hour = (alarms[currentAlarmSlot].hour - 1 + 24) % 24;
              } else if (editingMinutes) {
                // Decrement minute
                if (alarms[currentAlarmSlot].minute == 0) {
                  alarms[currentAlarmSlot].minute = 59;
                } else {
                  alarms[currentAlarmSlot].minute--;
                }
              }
            }
            break;
          case ALARM_SUB_STATION:
            alarms[currentAlarmSlot].stationIndex = (alarms[currentAlarmSlot].stationIndex - 1 + menuStreamCount) % menuStreamCount;
            break;
          case ALARM_SUB_SCHEDULE:
            alarms[currentAlarmSlot].schedule = (AlarmSchedule)((alarms[currentAlarmSlot].schedule - 1 + ALARM_SCHEDULE_COUNT) % ALARM_SCHEDULE_COUNT);
            break;
          case ALARM_SUB_VOLUME:
            alarms[currentAlarmSlot].maxVolume -= 1;
            if (alarms[currentAlarmSlot].maxVolume < 1) alarms[currentAlarmSlot].maxVolume = 80;
            break;
          case ALARM_SUB_AUTO_OFF:
            alarms[currentAlarmSlot].autoOff = (AlarmAutoOff)((alarms[currentAlarmSlot].autoOff - 1 + AUTO_OFF_COUNT) % AUTO_OFF_COUNT);
            break;
        }
      } else {
        // We're browsing sub-menu options
        currentAlarmSubMenu = (AlarmSubMenuState)((currentAlarmSubMenu - 1 + ALARM_SUB_COUNT) % ALARM_SUB_COUNT);
      }
    } else {
      // We're browsing main alarm slots
      currentAlarmMenu = (AlarmMenuState)((currentAlarmMenu - 1 + ALARM_MENU_COUNT) % ALARM_MENU_COUNT);
      // Update currentAlarmSlot based on the menu selection
      if (currentAlarmMenu >= ALARM_MENU_SLOT1 && currentAlarmMenu <= ALARM_MENU_SLOT5) {
        currentAlarmSlot = currentAlarmMenu; // SLOT1=0, SLOT2=1, etc.
      }
    }
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_STREAMS) {
    currentStream--;
    if (currentStream < 0) currentStream = menuStreamCount - 1;
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_SLEEP) {
    currentSleepMenu = (SleepMenuState)((currentSleepMenu - 1 + SLEEP_MENU_COUNT) % SLEEP_MENU_COUNT);
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_BRIGHTNESS) {
    backlightAlwaysOn = !backlightAlwaysOn;
    brightnessChanged = true;
    lastActivity = currentTime;
    forceImmediateLcdUpdate = true;
  }
}

void handleMenuButtonPress() {
  if (currentMenu == MENU_WIFI) {
    if (showingConfirmation) {
      if (confirmationChoice) {
        resetWiFiSettings();
      } else {
        showingConfirmation = false;
        forceImmediateLcdUpdate = true;
      }
    } else {
      switch (currentWiFiMenu) {
        case WIFI_MENU_IP:
        case WIFI_MENU_SSID:
        case WIFI_MENU_PASSWORD:
          nextMenuItem();
          break;
        case WIFI_MENU_RESET:
          showingConfirmation = true;
          confirmationChoice = false;
          forceImmediateLcdUpdate = true;
          Serial.println("Showing WiFi reset confirmation");
          break;
      }
    }
  } else if (currentMenu == MENU_WEATHER) {
    handleWeatherMenuButtonPress();
  } else if (currentMenu == MENU_SYSTEM) {
    handleSystemMenuButtonPress();
  } else if (currentMenu == MENU_ALARMS) {
    handleAlarmMenuButtonPress();
  } else if (currentMenu == MENU_SLEEP) {
    handleSleepMenuButtonPress();
  } else if (currentMenu == MENU_STREAMS) {
    if (currentStream == playingStream) {
      nextMenuItem();
    } else {
      selectStream();
      exitMenu();
    }
  } else {
    nextMenuItem();
  }
}

void handleWeatherMenuButtonPress() {
  switch (currentWeatherMenu) {
    case WEATHER_MENU_TEMPERATURE:
    case WEATHER_MENU_HUMIDITY:
    case WEATHER_MENU_DESCRIPTION:
    case WEATHER_MENU_API_KEY:
      // These are informational - just advance to next main menu item (like WiFi menu)
      nextMenuItem();
      break;
    case WEATHER_MENU_UPDATE:
      // Force a weather update
      Serial.println("Manual weather update requested");
      lastWeatherUpdate = 0; // Reset timer to force immediate update
      forceWeatherUpdate(); // Call weather update function
      exitMenu(); // Exit menu after triggering update
      break;
  }
}

void handleSystemMenuButtonPress() {
  switch (currentSystemMenu) {
    case SYSTEM_MENU_FIRMWARE:
      // Firmware version is informational - just move to next menu
      nextMenuItem();
      break;
    case SYSTEM_MENU_UPDATE:
      // Check for and perform firmware update
      Serial.println("Firmware update requested");
      
      // Show checking message
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Checking for");
      lcd.setCursor(0, 1);
      lcd.print("updates...");
      delay(1000);
      
      OTAResult result = checkForUpdate();
      
      if (result == OTA_NO_UPDATE) {
        // Show "Up to Date" message briefly
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Firmware");
        lcd.setCursor(0, 1);
        lcd.print("Up to Date");
        delay(2000);
        exitMenu();
      } else if (result == OTA_SUCCESS) {
        // New version available - ask user to confirm
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Update found!");
        lcd.setCursor(0, 1);
        lcd.print("Installing...");
        delay(1000);
        
        // Download and install update
        if (downloadAndInstallUpdate()) {
          // Update successful - device will reboot
          Serial.println("Update completed successfully");
        } else {
          // Update failed
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Update Failed");
          lcd.setCursor(0, 1);
          lcd.print("Try again later");
          delay(3000);
          exitMenu();
        }
      } else {
        // Network error or other issue
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Update Error");
        lcd.setCursor(0, 1);
        lcd.print("Check WiFi");
        delay(3000);
        exitMenu();
      }
      break;
  }
}

void handleSleepMenuButtonPress() {
  switch (currentSleepMenu) {
    case SLEEP_MENU_BLANK:
      // Blank option - just move to next menu
      nextMenuItem();
      break;
    case SLEEP_MENU_OFF:
      // Turn off sleep timer
      sleepTimerActive = false;
      Serial.println("Sleep timer turned OFF");
      exitMenu(); // Exit menu after action
      break;
    case SLEEP_MENU_15MIN:
      setSleepTimer(15);
      exitMenu(); // Exit menu after action
      break;
    case SLEEP_MENU_30MIN:
      setSleepTimer(30);
      exitMenu(); // Exit menu after action
      break;
    case SLEEP_MENU_60MIN:
      setSleepTimer(60);
      exitMenu(); // Exit menu after action
      break;
    case SLEEP_MENU_90MIN:
      setSleepTimer(90);
      exitMenu(); // Exit menu after action
      break;
    case SLEEP_MENU_5MIN:
      setSleepTimer(5);
      exitMenu(); // Exit menu after action
      break;
  }
}

void setSleepTimer(int minutes) {
  sleepTimerStart = millis();
  sleepTimerDuration = minutes * 60 * 1000; // Convert minutes to milliseconds
  sleepTimerActive = true;
  
  Serial.print("Sleep timer set for ");
  Serial.print(minutes);
  Serial.println(" minutes");
}

void checkSleepTimer() {
  if (sleepTimerActive && (millis() - sleepTimerStart >= sleepTimerDuration)) {
    // Timer expired - turn off radio
    Serial.println("Sleep timer expired - turning off radio");
    radioPowerOn = false;
    audio.stopSong();
    sleepTimerActive = false;
    forceImmediateLcdUpdate = true;
  }
}

void displayAlarmMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  if (!inAlarmSubMenu) {
    // Main alarm menu - show slot list
    if (currentAlarmMenu == ALARM_MENU_BLANK) {
      lcd.print("Alarms");
      lcd.setCursor(0, 1);
      lcd.print("                "); // Blank line for navigation
      return;
    }
    
    // Show "Alarms" on first line
    lcd.print("Alarms");
    lcd.setCursor(0, 1);
    
    // Show current slot option
    switch (currentAlarmMenu) {
      case ALARM_MENU_SLOT1:
        lcd.print("Slot 1/5");
        if (alarms[0].enabled) lcd.print("    ON");
        break;
      case ALARM_MENU_SLOT2:
        lcd.print("Slot 2/5");
        if (alarms[1].enabled) lcd.print("    ON");
        break;
      case ALARM_MENU_SLOT3:
        lcd.print("Slot 3/5");
        if (alarms[2].enabled) lcd.print("    ON");
        break;
      case ALARM_MENU_SLOT4:
        lcd.print("Slot 4/5");
        if (alarms[3].enabled) lcd.print("    ON");
        break;
      case ALARM_MENU_SLOT5:
        lcd.print("Slot 5/5");
        if (alarms[4].enabled) lcd.print("    ON");
        break;
    }
  } else {
    // Sub-menu - show alarm slot and status on first line
    switch (currentAlarmSubMenu) {
      case ALARM_SUB_BACK:
        lcd.print(String(currentAlarmSlot + 1) + ": ON");
        lcd.setCursor(0, 1);
        lcd.print("< BACK");
        break;
      case ALARM_SUB_ENABLED:
        lcd.print(String(currentAlarmSlot + 1) + ": Enable");
        if (editingAlarmOption) lcd.print("    *");
        lcd.setCursor(0, 1);
        lcd.print(alarms[currentAlarmSlot].enabled ? "YES" : "NO");
        break;
      case ALARM_SUB_TIME:
      {
        lcd.print(String(currentAlarmSlot + 1) + ": Time");
        if (editingAlarmOption) lcd.print("      *");
        lcd.setCursor(0, 1);
        
        // Check if we should show blinking (every 500ms)
        bool showBlink = (millis() / 500) % 2 == 0;
        
        // Display hours with blinking if editing
        if (editingTime && editingHours && !showBlink) {
          lcd.print("  "); // Blank spaces for blinking hours
        } else {
          if (alarms[currentAlarmSlot].hour < 10) lcd.print("0");
          lcd.print(alarms[currentAlarmSlot].hour);
        }
        
        lcd.print(":");
        
        // Display minutes with blinking if editing
        if (editingTime && editingMinutes && !showBlink) {
          lcd.print("  "); // Blank spaces for blinking minutes
        } else {
          if (alarms[currentAlarmSlot].minute < 10) lcd.print("0");
          lcd.print(alarms[currentAlarmSlot].minute);
        }
        break;
      }
      case ALARM_SUB_STATION:
        lcd.print(String(currentAlarmSlot + 1) + ": Station");
        if (editingAlarmOption) lcd.print("   *");
        lcd.setCursor(0, 1);
        if (alarms[currentAlarmSlot].stationIndex < menuStreamCount) {
          lcd.print(menuStreams[alarms[currentAlarmSlot].stationIndex].name);
        } else {
          lcd.print("Unknown");
        }
        break;
      case ALARM_SUB_SCHEDULE:
        lcd.print(String(currentAlarmSlot + 1) + ": Schedule");
        if (editingAlarmOption) lcd.print("  *");
        lcd.setCursor(0, 1);
        switch (alarms[currentAlarmSlot].schedule) {
          case ALARM_ONCE:
            lcd.print("Once");
            break;
          case ALARM_DAILY:
            lcd.print("Daily");
            break;
          case ALARM_WEEKDAYS:
            lcd.print("Weekdays");
            break;
          case ALARM_WEEKENDS:
            lcd.print("Weekends");
            break;
        }
        break;
      case ALARM_SUB_VOLUME:
        lcd.print(String(currentAlarmSlot + 1) + ": Volume");
        if (editingAlarmOption) lcd.print("    *");
        lcd.setCursor(0, 1);
        lcd.print(alarms[currentAlarmSlot].maxVolume);
        break;
      case ALARM_SUB_AUTO_OFF:
        lcd.print(String(currentAlarmSlot + 1) + ": Auto Off");
        if (editingAlarmOption) lcd.print("   *");
        lcd.setCursor(0, 1);
        switch (alarms[currentAlarmSlot].autoOff) {
          case AUTO_OFF_NO:
            lcd.print("NO");
            break;
          case AUTO_OFF_5MIN:
            lcd.print("5 minutes");
            break;
          case AUTO_OFF_15MIN:
            lcd.print("15 minutes");
            break;
          case AUTO_OFF_30MIN:
            lcd.print("30 minutes");
            break;
          case AUTO_OFF_60MIN:
            lcd.print("60 minutes");
            break;
          case AUTO_OFF_90MIN:
            lcd.print("90 minutes");
            break;
        }
        break;
    }
  }
}

void handleAlarmMenuButtonPress() {
  if (!inAlarmSubMenu) {
    // Main alarm menu - handle slot selection
    if (currentAlarmMenu == ALARM_MENU_BLANK) {
      // Exit alarm menu and go to next main menu
      nextMenuItem();
    } else {
      // Enter sub-menu for the selected slot
      inAlarmSubMenu = true;
      currentAlarmSubMenu = ALARM_SUB_BACK;
      // currentAlarmSlot is already set based on currentAlarmMenu
    }
  } else {
    // Sub-menu handling
    if (editingAlarmOption) {
      // We're in edit mode
      if (currentAlarmSubMenu == ALARM_SUB_TIME && editingTime) {
        // Special time editing state machine
        if (editingHours) {
          // Move from editing hours to editing minutes
          editingHours = false;
          editingMinutes = true;
        } else if (editingMinutes) {
          // Confirm time and exit time editing
          editingMinutes = false;
          editingTime = false;
          editingAlarmOption = false;
          saveSettings();
        }
      } else {
        // Regular option - confirm the change and go back to browse mode
        editingAlarmOption = false;
        editingTime = false;
        editingHours = false;
        editingMinutes = false;
        saveSettings();
      }
    } else {
      // We're in sub-menu browse mode
      if (currentAlarmSubMenu == ALARM_SUB_BACK) {
        // Go back to main alarm menu
        inAlarmSubMenu = false;
        editingAlarmOption = false;
        editingTime = false;
        editingHours = false;
        editingMinutes = false;
      } else if (currentAlarmSubMenu == ALARM_SUB_TIME) {
        // Enter time editing mode - start with hours
        editingAlarmOption = true;
        editingTime = true;
        editingHours = true;
        editingMinutes = false;
      } else {
        // Enter edit mode for other sub-options (ENABLED, STATION, SCHEDULE, VOLUME, AUTO_OFF)
        editingAlarmOption = true;
      }
    }
  }
  forceImmediateLcdUpdate = true;
}
