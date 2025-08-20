#include "display.h"
#include "config.h"
#include "settings.h"
#include "menu.h"
#include "wifi_config.h"
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

// Variables to track what's currently displayed to avoid unnecessary updates
String lastDisplayedLine0 = "";
String lastDisplayedLine1 = "";
bool lastShowVolumeDisplay = false;
bool lastInMenu = false;

// Custom character for backspace
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
  
  // Test if LCD is responding
  lcd.setCursor(0, 0);
  lcd.print("ESP32 Radio");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  
  Serial.println("LCD initialized");
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

void updateLCD() {
  // Handle WiFi configuration display separately - always update immediately
  if (wifiConfigMode) {
    forceImmediateLcdUpdate = false;  // Reset the flag for WiFi config mode too
    updateWiFiConfigDisplay();
    return;
  }
  
  // Check if we need immediate update or if it's time for regular update
  if (!forceImmediateLcdUpdate && (millis() - lastLcdUpdate < LCD_UPDATE_INTERVAL)) return;
  
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
  
  if (radioPowerOn && isStreaming) {
    // Radio is on: Top line: Time only (centered)
    updateLCDLine(0, String(timeStr), true);
    
    // Bottom line: Current stream name (centered)
    updateLCDLine(1, currentStreamName, true);
  } else {
    // Radio is off or not streaming: Top line: Time only (centered)
    updateLCDLine(0, String(timeStr), true);
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
    case MENU_VOLUME: {
      line0 = "MENU: Volume";
      line1 = "Level: " + String(volume);
      break;
    }
    case MENU_STREAMS: {
      line0 = "MENU: Station";
      String streamName = streams[currentStream].name;
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
  }
  
  updateLCDLine(0, line0, false);
  updateLCDLine(1, line1, false);
}
