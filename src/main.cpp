//ESP32 Internet Radio - Modular Version
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "SPIFFS.h"
#include "config.h"
#include "settings.h"
#include "display.h"
#include "encoder.h"
#include "menu.h"
#include "alarm.h"
#include "wifi_config.h"
#include "webserver.h"
#include "weather.h"

// Audio object
Audio audio;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3 Internet Radio Starting...");
  Serial.print("Firmware Version: ");
  Serial.println(FIRMWARE_VERSION);
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("Flash Size: ");
  Serial.println(ESP.getFlashChipSize());
  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("PSRAM Size: ");
  Serial.println(ESP.getPsramSize());
  Serial.print("Free PSRAM: ");
  Serial.println(ESP.getFreePsram());
  Serial.println("");
  
  // Initialize EEPROM and load settings
  initializeEEPROM();
  loadSettings();
  
  // Setup encoder pins
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(ENCODER_BTN, INPUT_PULLUP);
  
  // Setup interrupts (only for encoder rotation, not button)
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), handleEncoder, CHANGE);
  
  // Initialize encoder state
  encoderA_last = digitalRead(ENCODER_A);
  
  // Setup LCD
  setupLCD();
  
  // Initialize backlight activity timer
  lastActivity = millis();
  
  // Check if WiFi credentials are configured
  if (ssid.length() == 0) {
    Serial.println("No WiFi credentials found. Starting WiFi configuration...");
    // Create custom character for backspace before WiFi config
    lcd.createChar(0, backspaceSymbol);
    configureWiFi();
  }
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // Try to connect for 30 seconds
  unsigned long wifiStartTime = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - wifiStartTime < 30000)) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connection failed after 30 seconds. Starting WiFi configuration...");
    // Create custom character for backspace before WiFi config
    lcd.createChar(0, backspaceSymbol);
    configureWiFi();
    
    // Try connecting again with new credentials
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  
  // Setup time after WiFi connection
  setupTime();
  
  // Initialize SPIFFS filesystem
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialization failed!");
    Serial.println("Attempting to format SPIFFS...");
    if (SPIFFS.format()) {
      Serial.println("SPIFFS formatted successfully");
      if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS initialization failed after format!");
        return;
      }
    } else {
      Serial.println("SPIFFS format failed!");
      return;
    }
  }
  Serial.println("SPIFFS initialized successfully");
  
  // Initialize web server
  initWebServer();
  
  // Initialize weather module
  initWeather();
  
  // Load streams from JSON file for menu system
  loadMenuStreamsFromFile();
  
  // Initialize alarm system
  initializeAlarms();
  alarmSystemActive = true;
  
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume);
  
  // Only start streaming if radio is powered on
  if (radioPowerOn && menuStreamCount > 0) {
    audio.connecttohost(menuStreams[currentStream].url);
    playingStream = currentStream;
    isStreaming = true;
    currentStreamName = menuStreams[currentStream].name;
    Serial.print("Connected to: ");
    Serial.println(menuStreams[currentStream].name);
  } else {
    isStreaming = false;
    if (menuStreamCount == 0) {
      Serial.println("No streams available");
    } else {
      Serial.println("Radio is OFF - not starting stream");
    }
  }
}

void loop() {
  static int lastVolume = volume;
  static int lastStream = currentStream;
  
  // Check for menu timeout
  if (inMenu && (millis() - lastMenuActivity > MENU_TIMEOUT)) {
    exitMenu();
  }
  
  // Handle button press (menu navigation)
  if (checkButtonPress()) {
    lastMenuActivity = millis();
    
    // Check if an alarm is currently ringing
    if (activeAlarmIndex >= 0) {
      // Alarm is active - short press = snooze
      snoozeAlarm();
      return; // Don't process other button logic
    }
    
    if (!inMenu) {
      // Check if display is in auto-off mode and currently off (no recent activity)
      if (!backlightAlwaysOn && (millis() - lastActivity > BACKLIGHT_TIMEOUT)) {
        // Display is currently off - first press should just wake it up
        if (!displayJustWokenUp) {
          // First press: wake up display, show time for 5 seconds
          displayJustWokenUp = true;
          displayWakeTime = millis();
          lastActivity = millis(); // Wake up the display
          lcd.backlight();
          Serial.println("Display woken up - press again within 5 seconds to enter menu");
          return; // Don't enter menu yet
        } else {
          // Second press within 5 seconds: enter menu
          if (millis() - displayWakeTime <= 5000) {
            displayJustWokenUp = false;
            enterMenu();
            Serial.println("Entering menu after wake-up");
          } else {
            // More than 5 seconds passed, treat as first press again
            displayJustWokenUp = true;
            displayWakeTime = millis();
            lastActivity = millis();
            Serial.println("Display woken up again - press again within 5 seconds to enter menu");
            return;
          }
        }
      } else {
        // Display is already on or always-on mode - enter menu immediately
        displayJustWokenUp = false;
        lastActivity = millis(); // Update backlight activity
        enterMenu();
      }
    } else {
      // Already in menu - handle menu button press
      lastActivity = millis(); // Update backlight activity
      handleMenuButtonPress();
    }
  }
  
  // Handle long button press (power on/off) - only when not in menu
  if (!inMenu && checkLongButtonPress()) {
    // Check if an alarm is currently ringing
    if (activeAlarmIndex >= 0) {
      // Alarm is active - long press = stop alarm
      stopAlarm();
      return; // Don't process power toggle
    }
    
    // Check if an alarm is currently snoozing
    int snoozingIndex = getSnoozingAlarmIndex();
    if (snoozingIndex >= 0) {
      // Alarm is snoozing - long press = cancel snooze and turn on radio
      cancelSnooze();
      // Continue to turn on radio below
    }
    
    radioPowerOn = !radioPowerOn;
    Serial.print("Radio power: ");
    Serial.println(radioPowerOn ? "ON" : "OFF");
    
    if (radioPowerOn) {
      // Power on - start streaming
      audio.connecttohost(menuStreams[currentStream].url);
      playingStream = currentStream;
      isStreaming = true;
      currentStreamName = menuStreams[currentStream].name;
      Serial.print("Connected to: ");
      Serial.println(menuStreams[currentStream].name);
    } else {
      // Power off - stop streaming and cancel sleep timer
      audio.stopSong();
      isStreaming = false;
      if (sleepTimerActive) {
        sleepTimerActive = false; // Cancel sleep timer when manually turning off radio
        Serial.println("Streaming stopped - Sleep timer cancelled");
      } else {
        Serial.println("Streaming stopped");
      }
    }
    
    saveSettings(); // Save the power state
    forceImmediateLcdUpdate = true; // Update display immediately
    lastActivity = millis(); // Update backlight activity
  }
  
  // Handle brightness/backlight control
  if (brightnessChanged) {
    if (backlightAlwaysOn) {
      lcd.backlight();
    } else {
      // In auto-off mode, turn on backlight and start timer
      lcd.backlight();
      lastActivity = millis();
    }
    brightnessChanged = false;
    Serial.print("Backlight Mode: ");
    Serial.println(backlightAlwaysOn ? "ALWAYS ON" : "AUTO OFF");
    saveSettings(); // Save backlight setting
  }
  
  // Handle auto-off backlight timeout (only in auto-off mode)
  if (!backlightAlwaysOn && !inMenu) {
    if (millis() - lastActivity > BACKLIGHT_TIMEOUT) {
      lcd.noBacklight();
      displayJustWokenUp = false; // Reset wake-up state when display times out
    }
  }
  
  // Update activity time for menu actions and volume changes
  if (inMenu) {
    lcd.backlight(); // Always on while in menu
    lastActivity = millis(); // Keep updating activity while in menu
  }
  
  // Handle volume change (only when not in menu)
  if (volume != lastVolume && !inMenu) {
    audio.setVolume(volume);
    Serial.print("Volume: ");
    Serial.println(volume);
    lastVolume = volume;
    saveSettings(); // Save volume setting
  }
  
  // Handle stream change (when in streams menu)
  if (currentStream != lastStream && inMenu && currentMenu == MENU_STREAMS) {
    Serial.print("Selected Stream: ");
    if (menuStreamCount > 0) {
      Serial.println(menuStreams[currentStream].name);
    } else {
      Serial.println("No streams available");
    }
    lastStream = currentStream;
  }
  
  // Process audio (only when radio is on)
  if (radioPowerOn) {
    audio.loop();
  }
  
  // Handle web server
  handleWebServer();
  
  // Update weather data
  updateWeather();
  
  // Check sleep timer
  checkSleepTimer();
  
  // Check alarms
  checkAlarms();
  
  // Update LCD display
  updateLCD();
}

// Audio callback functions
void audio_info(const char *info) {
  Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info) {
  Serial.print("id3data     "); Serial.println(info);
}
void audio_eof_mp3(const char *info) {
  Serial.print("eof_mp3     "); Serial.println(info);
}
void audio_showstation(const char *info) {
  Serial.print("station     "); Serial.println(info);
  
  // Reset track info when station changes
  hasTrackInfo = false;
  showTrackInfo = false;
  currentTrackInfo = "";
}
void audio_showstreaminfo(const char *info) {
  Serial.print("streaminfo  "); Serial.println(info);
}
void audio_showstreamtitle(const char *info) {
  Serial.print("streamtitle "); Serial.println(info);
  
  // Update track info for display
  if (info && strlen(info) > 0) {
    currentTrackInfo = String(info);
    hasTrackInfo = true;
    // Reset the toggle timer to immediately show new track info
    lastTrackToggle = millis() - 10000;
    showTrackInfo = true;
    // Reset scroll position for new track
    trackScrollPosition = 0;
    lastTrackScroll = millis();
    forceImmediateLcdUpdate = true;
  } else {
    hasTrackInfo = false;
    showTrackInfo = false;
  }
}
void audio_bitrate(const char *info) {
  Serial.print("bitrate     "); Serial.println(info);
}
void audio_commercial(const char *info) {
  Serial.print("commercial  "); Serial.println(info);
}
void audio_icyurl(const char *info) {
  Serial.print("icyurl      "); Serial.println(info);
}
void audio_lasthost(const char *info) {
  Serial.print("lasthost    "); Serial.println(info);
}
void audio_eof_speech(const char *info) {
  Serial.print("eof_speech  "); Serial.println(info);
}