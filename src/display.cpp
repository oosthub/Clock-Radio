#include "display.h"
#include "config.h"
#include "settings.h"
#include "menu.h"
#include "alarm.h"
#include "wifi_config.h"
#include "weather.h"
#include "Wire.h"
#include "time.h"
#include "WiFi.h"

// Display variables
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
unsigned long lastLcdUpdate = 0;
bool isStreaming = false;
String currentStreamName = "";
bool forceImmediateLcdUpdate = false;
bool showVolumeDisplay = false;
unsigned long lastVolumeChange = 0;
unsigned long lastActivity = 0;
bool displayJustWokenUp = false;
unsigned long displayWakeTime = 0;

// Radio startup display control
bool radioJustTurnedOn = false;
bool waitingForStreamStart = false;
unsigned long radioTurnOnTime = 0;

// Now Playing Info variables
String currentTrackInfo = "";
bool hasTrackInfo = false;
bool showTrackInfo = false;
unsigned long lastTrackToggle = 0;
int trackScrollPosition = 0;
unsigned long lastTrackScroll = 0;

// Temporary message display variables
bool showTemporaryMessage = false;
String temporaryMessage = "";
unsigned long temporaryMessageStart = 0;
unsigned long temporaryMessageDuration = 3000;

// Variables to track what's currently displayed to avoid unnecessary updates
String lastDisplayedLine0 = "";
String lastDisplayedLine1 = "";
bool lastShowVolumeDisplay = false;
bool lastInMenu = false;

// Custom characters for LCD display
byte backspaceSymbol[8] = {
  0b00000,
  0b00100,
  0b01100,
  0b11111,
  0b01100,
  0b00100,
  0b00000,
  0b00000
};

// Weather custom characters
byte degreeSymbol[8] = {
  0b01110,
  0b01010,
  0b01110,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

byte sunSymbol[8] = {
  0b00000,
  0b10101,
  0b01110,
  0b11111,
  0b01110,
  0b10101,
  0b00000,
  0b00000
};

byte cloudSymbol[8] = {
  0b00000,
  0b00000,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
  0b00000,
  0b00000
};

byte clockSymbol[8] = {
  0b01110,
  0b10001,
  0b10101,
  0b10101,
  0b10011,
  0b10001,
  0b01110,
  0b00000
};

byte upArrowSymbol[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};


// Helper function to check if any alarms are enabled
bool hasEnabledAlarms() {
  for (int i = 0; i < MAX_ALARMS; i++) {
    if (alarms[i].enabled) {
      return true;
    }
  }
  return false;
}

void scanI2C() {
  Serial.println("Scanning for I2C devices...");
  int deviceCount = 0;
  
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" I2C device(s)");
  }
}

void setupLCD() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("I2C initialized");
  
  // Scan for I2C devices first
  scanI2C();
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Create custom characters
  lcd.createChar(0, backspaceSymbol);  // Character 0: backspace
  lcd.createChar(1, degreeSymbol);     // Character 1: degree symbol
  lcd.createChar(2, sunSymbol);        // Character 2: sun
  lcd.createChar(3, cloudSymbol);      // Character 3: cloud
  lcd.createChar(4, clockSymbol);      // Character 4: clock
  lcd.createChar(5, upArrowSymbol);    // Character 5: up arrow

  // Test if LCD is responding
  lcd.setCursor(0, 0);
  lcd.print("OOSIE Radio");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  
  Serial.println("LCD initialized with custom characters");
  delay(2000);
}

void updateLCDLine(int line, String content, bool center) {
  // Pad or truncate content to exactly 16 characters
  if (content.length() > 16) {
    content = content.substring(0, 16);
  }
  
  if (center && content.length() < 16) {
    int padding = (16 - content.length()) / 2;
    String centeredContent = "";
    for (int i = 0; i < padding; i++) centeredContent += " ";
    centeredContent += content;
    while (centeredContent.length() < 16) centeredContent += " ";
    content = centeredContent;
  } else {
    // Pad with spaces to clear any remaining characters
    while (content.length() < 16) content += " ";
  }
  
  // Only update if content has changed
  String* lastDisplayed = (line == 0) ? &lastDisplayedLine0 : &lastDisplayedLine1;
  if (*lastDisplayed != content) {
    lcd.setCursor(0, line);
    lcd.print(content);
    *lastDisplayed = content;
  }
}

bool isTrackScrollComplete(const String& fullText) {
  if (fullText.length() <= 16) {
    return true; // No scrolling needed, always complete
  }
  
  int maxPosition = fullText.length() - 16;
  // Consider complete when we've scrolled through the text and paused at the end
  return trackScrollPosition > maxPosition + 5; // Almost at reset point
}

String getScrolledTrackText(const String& fullText) {
  if (fullText.length() <= 16) {
    // Text fits, no scrolling needed
    trackScrollPosition = 0;
    return fullText;
  }
  
  // Check if it's time to scroll
  if (millis() - lastTrackScroll >= 300) { // Scroll every 300ms (faster)
    trackScrollPosition++;
    lastTrackScroll = millis();
    
    // Reset scroll position when we reach the end
    int maxPosition = fullText.length() - 16;
    if (trackScrollPosition > maxPosition + 6) { // +6 for longer pause at end
      trackScrollPosition = 0;
    }
  }
  
  // Get the substring to display
  if (trackScrollPosition <= fullText.length() - 16) {
    return fullText.substring(trackScrollPosition, trackScrollPosition + 16);
  } else {
    // At the end, show the last 16 characters
    return fullText.substring(fullText.length() - 16);
  }
}

void updateLCD() {
  // Handle WiFi configuration display separately - always update immediately
  if (wifiConfigMode) {
    forceImmediateLcdUpdate = false;  // Reset the flag for WiFi config mode too
    updateWiFiConfigDisplay();
    return;
  }
  
  // Handle temporary message display
  if (showTemporaryMessage) {
    if (millis() - temporaryMessageStart >= temporaryMessageDuration) {
      // Timeout reached, hide temporary message
      showTemporaryMessage = false;
      lcd.clear();
      lastDisplayedLine0 = "";
      lastDisplayedLine1 = "";
      forceImmediateLcdUpdate = true;
    } else {
      // Show temporary message
      updateLCDLine(0, "ALARM", true);
      updateLCDLine(1, temporaryMessage, true);
      return;
    }
  }
  
  // Check if we need immediate update or if it's time for regular update
  // During time editing, update more frequently for blinking effect
  unsigned long updateInterval = LCD_UPDATE_INTERVAL;
  if (editingTime && (editingHours || editingMinutes)) {
    updateInterval = 250; // Update every 250ms for smooth blinking
  }
  
  if (!forceImmediateLcdUpdate && (millis() - lastLcdUpdate < updateInterval)) return;
  
  lastLcdUpdate = millis();
  forceImmediateLcdUpdate = false;  // Reset the flag
  
  // Check for volume display timeout
  if (showVolumeDisplay && (millis() - lastVolumeChange > VOLUME_DISPLAY_TIMEOUT)) {
    showVolumeDisplay = false;
  }
  
  // If transitioning between volume display and normal display, clear screen once
  if (lastShowVolumeDisplay != showVolumeDisplay) {
    lcd.clear();
    lastDisplayedLine0 = "";
    lastDisplayedLine1 = "";
    lastShowVolumeDisplay = showVolumeDisplay;
  }
  
  // If transitioning into or out of menu mode, clear screen once
  if (lastInMenu != inMenu) {
    lcd.clear();
    lastDisplayedLine0 = "";
    lastDisplayedLine1 = "";
    lastInMenu = inMenu;
  }
  
  if (inMenu) {
    // For menu mode, use optimized display that only updates changed content
    displayCurrentMenuOptimized();
    return;
  }
  
  // Show volume display when adjusting volume outside menu
  if (showVolumeDisplay) {
    updateLCDLine(0, "Volume", true);
    updateLCDLine(1, "Level: " + String(volume), true);
    return;
  }
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return; // Skip update if time not available
  }
  
  char timeStr[6];
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
  
  // Get weather string for display
  String weatherStr = "";
  if (weatherApiKey.length() == 0) {
    weatherStr = ""; // No API key, show nothing
  } else if (currentWeather.valid) {
    weatherStr = formatTemperature(currentWeather.temperature) + currentWeather.icon;
  } else {
    weatherStr = "?" + String((char)1) + "C"; // ?°C using custom degree symbol
  }
  
  // Time + Weather layout: "12:34 🕐 Z 22°C☀" with fixed positions for indicators
  String timeWeatherLine = String(timeStr);
  
  // Add clock symbol (always reserve space for consistent positioning)
  if (hasEnabledAlarms()) {
    timeWeatherLine += " " + String((char)4); // Clock symbol (character 4)
  } else {
    timeWeatherLine += "  "; // Reserve 2 spaces (space + clock position)
  }
  
  // Add sleep timer indicator (always reserve space for consistent positioning)
  if (sleepTimerActive) {
    timeWeatherLine += " Z";
  } else {
    timeWeatherLine += "  "; // Reserve 2 spaces (space + Z position)
  }
  
  // Calculate spacing to right-align weather on 16-char display
  int remainingSpace = 16 - timeWeatherLine.length() - weatherStr.length();
  if (remainingSpace > 0) {
    for (int i = 0; i < remainingSpace; i++) {
      timeWeatherLine += " ";
    }
    timeWeatherLine += weatherStr;
  } else {
    // If everything doesn't fit, truncate weather
    int maxWeatherLen = 16 - timeWeatherLine.length() - 1;
    if (maxWeatherLen > 0) {
      timeWeatherLine += " " + weatherStr.substring(0, maxWeatherLen);
    }
  }
  
  // Check for active alarm or snoozing alarm first (highest priority display)
  if (activeAlarmIndex >= 0) {
    // Active alarm - show alarm info and controls
    updateLCDLine(0, "ALARM " + String(activeAlarmIndex + 1) + "  STOP" + String((char)5), false); // Character 5 is up arrow
    updateLCDLine(1, currentStreamName, true);
    return;
  }
  
  // Check for snoozing alarm
  int snoozingIndex = getSnoozingAlarmIndex();
  if (snoozingIndex >= 0) {
    // Calculate remaining snooze time
    unsigned long snoozeElapsed = millis() - alarms[snoozingIndex].snoozeStart;
    unsigned long snoozeTotal = ALARM_SNOOZE_MINUTES * 60 * 1000;
    if (snoozeElapsed < snoozeTotal) {
      unsigned long remaining = (snoozeTotal - snoozeElapsed) / 1000; // Convert to seconds
      int minutes = remaining / 60;
      int seconds = remaining % 60;
      char timeStr[6];
      sprintf(timeStr, "%02d:%02d", minutes, seconds);
      
      // Show time on first line, snooze countdown on second line
      updateLCDLine(0, timeWeatherLine, false);
      updateLCDLine(1, "SNOOZE    " + String(timeStr), false);
      return;
    }
  }
  
  if (radioPowerOn && isStreaming) {
    // Radio is on: Top line: Time + Weather
    updateLCDLine(0, timeWeatherLine, false);
    
    // Bottom line: Alternate between station name and track info (if available)
    String bottomLineText = currentStreamName;
    
    if (hasTrackInfo && currentTrackInfo.length() > 0) {
      // Check if it's time to toggle display
      bool shouldToggle = false;
      
      if (currentTrackInfo.length() <= 16) {
        // Short track name - use 10 second timer
        shouldToggle = (millis() - lastTrackToggle >= 10000);
      } else {
        // Long track name - wait for scroll to complete OR 20 seconds max
        shouldToggle = (isTrackScrollComplete(currentTrackInfo) && (millis() - lastTrackToggle >= 5000)) ||
                      (millis() - lastTrackToggle >= 20000);
      }
      
      if (shouldToggle) {
        showTrackInfo = !showTrackInfo;
        lastTrackToggle = millis();
        // Reset scroll position when switching display
        trackScrollPosition = 0;
        lastTrackScroll = millis();
      }
      
      if (showTrackInfo) {
        bottomLineText = getScrolledTrackText(currentTrackInfo);
      }
    } else {
      // No track info available, always show station name
      showTrackInfo = false;
    }
    
    updateLCDLine(1, bottomLineText, !showTrackInfo); // Center station name, don't center scrolling track info
  } else {
    // Radio is off or not streaming: Top line: Time + Weather
    updateLCDLine(0, timeWeatherLine, false);
    // Bottom line: Show radio status
    if (!radioPowerOn) {
      updateLCDLine(1, "Radio OFF", true);
    } else {
      updateLCDLine(1, "", false); // Empty if on but not streaming
    }
  }
}

void displayCurrentMenuOptimized() {
  String line0 = "";
  String line1 = "";
  
  switch (currentMenu) {
    case MENU_SLEEP: {
      line0 = "MENU: Sleep";
      switch (currentSleepMenu) {
        case SLEEP_MENU_BLANK:
          line1 = ""; // Blank line
          break;
        case SLEEP_MENU_OFF:
          line1 = "OFF";
          break;
        case SLEEP_MENU_15MIN:
          line1 = "15 minutes";
          break;
        case SLEEP_MENU_30MIN:
          line1 = "30 minutes";
          break;
        case SLEEP_MENU_60MIN:
          line1 = "60 minutes";
          break;
        case SLEEP_MENU_90MIN:
          line1 = "90 minutes";
          break;
        case SLEEP_MENU_5MIN:
          line1 = "5 minutes";
          break;
      }
      break;
    }
    case MENU_STREAMS: {
      line0 = "MENU: Station";
      String streamName = menuStreams[currentStream].name;
      if (streamName.length() > 16) {
        streamName = streamName.substring(0, 16);
      }
      line1 = streamName;
      break;
    }
    case MENU_BRIGHTNESS: {
      line0 = "MENU: Backlight";
      line1 = "Mode: " + String(backlightAlwaysOn ? "ALWAYS ON" : "AUTO OFF");
      break;
    }
    case MENU_WIFI: {
      if (showingConfirmation) {
        line0 = "Reset WiFi?";
        line1 = confirmationChoice ? "> YES    NO" : "  YES  > NO";
      } else {
        line0 = "MENU: WiFi";
        switch (currentWiFiMenu) {
          case WIFI_MENU_IP: {
            if (WiFi.status() == WL_CONNECTED) {
              String ip = WiFi.localIP().toString();
              if (ip.length() > 16) {
                // Truncate IP if too long
                ip = ip.substring(0, 16);
              }
              line1 = ip;
            } else {
              line1 = "Not connected";
            }
            break;
          }
          case WIFI_MENU_SSID: {
            String displaySSID = ssid;
            if (displaySSID.length() > 10) {
              displaySSID = displaySSID.substring(0, 10);
            }
            line1 = "SSID: " + displaySSID;
            break;
          }
          case WIFI_MENU_PASSWORD: {
            line1 = "PASS: ";
            if (password.length() > 0) {
              line1 += "*****";
            }
            break;
          }
          case WIFI_MENU_RESET: {
            line1 = "Reset WiFi";
            break;
          }
        }
      }
      break;
    }
    case MENU_WEATHER: {
      line0 = "MENU: Weather";
      switch (currentWeatherMenu) {
        case WEATHER_MENU_TEMPERATURE: {
          line1 = "TEMP: ";
          if (currentWeather.valid) {
            line1 += String((int)currentWeather.temperature) + "C";
          } else {
            line1 += "--";
          }
          break;
        }
        case WEATHER_MENU_HUMIDITY: {
          line1 = "HUM: ";
          if (currentWeather.valid) {
            line1 += String(currentWeather.humidity) + "%";
          } else {
            line1 += "--";
          }
          break;
        }
        case WEATHER_MENU_DESCRIPTION: {
          line1 = "DESC: ";
          if (currentWeather.valid) {
            String desc = currentWeather.description;
            if (desc.length() > 10) {
              desc = desc.substring(0, 10);
            }
            line1 += desc;
          } else {
            line1 += "--";
          }
          break;
        }
        case WEATHER_MENU_API_KEY: {
          line1 = "API: ";
          if (weatherApiKey.length() > 0) {
            line1 += "SET";
          } else {
            line1 += "NOT SET";
          }
          break;
        }
        case WEATHER_MENU_UPDATE: {
          line1 = "Update Weather";
          break;
        }
      }
      break;
    }
    case MENU_SYSTEM: {
      line0 = "MENU: System";
      switch (currentSystemMenu) {
        case SYSTEM_MENU_FIRMWARE: {
          line1 = "Firm: ";
          line1 += FIRMWARE_VERSION;
          break;
        }
        case SYSTEM_MENU_UPDATE: {
          line1 = "Update";
          break;
        }
      }
      break;
    }
    case MENU_ALARMS: {
      // Alarm menu display is now handled by the dedicated displayAlarmMenu() function
      displayAlarmMenu();
      return; // Exit early since displayAlarmMenu() handles the entire display
    }
  }
  
  updateLCDLine(0, line0, false);
  updateLCDLine(1, line1, false);
}

void showTemporaryLCDMessage(String message, unsigned long duration) {
  showTemporaryMessage = true;
  temporaryMessage = message;
  temporaryMessageStart = millis();
  temporaryMessageDuration = duration;
  forceImmediateLcdUpdate = true;
  
  // Wake up display if in auto-off mode
  if (!backlightAlwaysOn) {
    lastActivity = millis();
    lcd.backlight();
  }
}

