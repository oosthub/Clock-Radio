//ESP32 Internet Radio - Modular Version
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "config.h"
#include "settings.h"
#include "display.h"
#include "encoder.h"
#include "menu.h"
#include "wifi_config.h"

// Audio object
Audio audio;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3 Internet Radio Starting...");
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
    Serial.println("No WiFi credentials found. Starting configuration...");
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
  
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(volume);
  
  // Only start streaming if radio is powered on
  if (radioPowerOn) {
    audio.connecttohost(streams[currentStream].url);
    playingStream = currentStream;
    isStreaming = true;
    currentStreamName = streams[currentStream].name;
    Serial.print("Connected to: ");
    Serial.println(streams[currentStream].name);
  } else {
    isStreaming = false;
    Serial.println("Radio is OFF - not starting stream");
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
    lastActivity = millis(); // Update backlight activity
    
    if (!inMenu) {
      // Enter menu mode
      enterMenu();
    } else {
      // Handle menu button press
      handleMenuButtonPress();
    }
  }
  
  // Handle long button press (power on/off) - only when not in menu
  if (!inMenu && checkLongButtonPress()) {
    radioPowerOn = !radioPowerOn;
    Serial.print("Radio power: ");
    Serial.println(radioPowerOn ? "ON" : "OFF");
    
    if (radioPowerOn) {
      // Power on - start streaming
      audio.connecttohost(streams[currentStream].url);
      playingStream = currentStream;
      isStreaming = true;
      currentStreamName = streams[currentStream].name;
      Serial.print("Connected to: ");
      Serial.println(streams[currentStream].name);
    } else {
      // Power off - stop streaming
      audio.stopSong();
      isStreaming = false;
      Serial.println("Streaming stopped");
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
    }
  }
  
  // Update activity time for menu actions and volume changes
  if (inMenu) {
    lcd.backlight(); // Always on while in menu
    lastActivity = millis(); // Keep updating activity while in menu
  }
  
  // Handle volume change (only when not in menu or in volume menu)
  if (volume != lastVolume && (!inMenu || currentMenu == MENU_VOLUME)) {
    audio.setVolume(volume);
    Serial.print("Volume: ");
    Serial.println(volume);
    lastVolume = volume;
    saveSettings(); // Save volume setting
    
    if (inMenu && currentMenu == MENU_VOLUME) {
      lastMenuActivity = millis();  // Reset timeout when adjusting volume in menu
    }
  }
  
  // Handle stream change (when in streams menu)
  if (currentStream != lastStream && inMenu && currentMenu == MENU_STREAMS) {
    Serial.print("Selected Stream: ");
    Serial.println(streams[currentStream].name);
    lastStream = currentStream;
  }
  
  // Process audio (only when radio is on)
  if (radioPowerOn) {
    audio.loop();
  }
  
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
}
void audio_showstreaminfo(const char *info) {
  Serial.print("streaminfo  "); Serial.println(info);
}
void audio_showstreamtitle(const char *info) {
  Serial.print("streamtitle "); Serial.println(info);
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