#include "wifi_config.h"
#include "config.h"
#include "settings.h"
#include "display.h"
#include "encoder.h"
#include "menu.h"
#include "WiFi.h"
#include "time.h"

// WiFi configuration variables
bool wifiConfigMode = false;
bool configuringSSID = true;
int charIndex = 0;
int selectedChar = 0;
String inputSSID = "";
String inputPassword = "";
const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz!@#$%^&*()-_=+[]{}|;:,.<>?/ ";
const int charsetSize = sizeof(charset) - 1;

void updateSelectedCharForPosition() {
  if (configuringSSID) {
    if (charIndex < inputSSID.length()) {
      // Position has existing character, find its index in charset
      char existingChar = inputSSID.charAt(charIndex);
      for (int i = 0; i < charsetSize; i++) {
        if (charset[i] == existingChar) {
          selectedChar = i;
          return;
        }
      }
      // If character not found in charset (shouldn't happen), keep current selection
    }
    // Position is empty or character not found, keep current selectedChar
  } else {
    if (charIndex < inputPassword.length()) {
      // Position has existing character, find its index in charset
      char existingChar = inputPassword.charAt(charIndex);
      for (int i = 0; i < charsetSize; i++) {
        if (charset[i] == existingChar) {
          selectedChar = i;
          return;
        }
      }
      // If character not found in charset (shouldn't happen), keep current selection
    }
    // Position is empty or character not found, keep current selectedChar
  }
}

void resetWiFiConfig() {
  inputSSID = "";
  inputPassword = "";
  charIndex = 0;
  selectedChar = 0;
  configuringSSID = true;
}

void updateWiFiConfigDisplay() {
  lcd.clear();
  
  if (configuringSSID) {
    lcd.setCursor(0, 0);
    lcd.print("SSID: Hold 3s>OK");
    lcd.setCursor(0, 1);
    
    // Build display string with current selection at cursor position
    String displaySSID = inputSSID;
    
    // Ensure string is long enough for cursor position
    while (displaySSID.length() <= charIndex) {
      displaySSID += " ";
    }
    
    // Replace character at cursor position with selected character
    if (selectedChar == charsetSize - 1) {
      // Show backspace symbol at cursor position
      displaySSID.setCharAt(charIndex, char(0));  // Custom character 0 (backspace)
    } else {
      displaySSID.setCharAt(charIndex, charset[selectedChar]);
    }
    
    // Truncate if too long for display
    if (displaySSID.length() > 16) {
      int start = max(0, charIndex - 15);
      displaySSID = displaySSID.substring(start, start + 16);
    }
    lcd.print(displaySSID);
    
    // Show cursor at current position
    int displayCursorPos = min(15, charIndex);
    if (charIndex >= 16) {
      displayCursorPos = charIndex - (charIndex - 15);
    }
    lcd.setCursor(displayCursorPos, 1);
    lcd.cursor();
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Pass: Hold 3s>OK");
    lcd.setCursor(0, 1);
    
    // Build display string with current selection at cursor position
    String displayPwd = inputPassword;
    
    // Ensure string is long enough for cursor position
    while (displayPwd.length() <= charIndex) {
      displayPwd += " ";
    }
    
    // Replace character at cursor position with selected character
    if (selectedChar == charsetSize - 1) {
      // Show backspace symbol at cursor position
      displayPwd.setCharAt(charIndex, char(0));  // Custom character 0 (backspace)
    } else {
      displayPwd.setCharAt(charIndex, charset[selectedChar]);
    }
    
    // Truncate if too long for display
    if (displayPwd.length() > 16) {
      int start = max(0, charIndex - 15);
      displayPwd = displayPwd.substring(start, start + 16);
    }
    lcd.print(displayPwd);
    
    // Show cursor at current position
    int displayCursorPos = min(15, charIndex);
    if (charIndex >= 16) {
      displayCursorPos = charIndex - (charIndex - 15);
    }
    lcd.setCursor(displayCursorPos, 1);
    lcd.cursor();
  }
}

bool connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
    return true;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connection Failed");
    lcd.setCursor(0, 1);
    lcd.print("Check & correct");
    delay(3000);
    return false;
  }
}

void configureWiFi() {
  wifiConfigMode = true;
  lcd.noCursor();
  resetWiFiConfig();
  updateSelectedCharForPosition();
  
  Serial.println("Entering WiFi configuration mode");
  updateWiFiConfigDisplay();
  
  while (wifiConfigMode) {
    // Check if encoder changed and update display immediately
    if (forceImmediateLcdUpdate) {
      forceImmediateLcdUpdate = false;
      updateWiFiConfigDisplay();
    }
    
    // Handle long button press for confirmation
    if (checkLongButtonPress()) {
      if (configuringSSID) {
        if (inputSSID.length() > 0) {
          ssid = inputSSID;
          configuringSSID = false;
          charIndex = 0;
          updateSelectedCharForPosition();
          Serial.println("SSID confirmed, moving to password");
        }
      } else {
        if (inputPassword.length() > 0) {
          password = inputPassword;
          lcd.noCursor();
          
          Serial.println("Password confirmed, attempting connection");
          if (connectToWiFi()) {
            saveSettings();
            wifiConfigMode = false;
            Serial.println("WiFi configured successfully");
          } else {
            configuringSSID = true;
            charIndex = 0;
            updateSelectedCharForPosition();
            Serial.println("WiFi connection failed - returning to SSID configuration with preserved values");
          }
        }
      }
      updateWiFiConfigDisplay();
    }
    
    // Handle short button press for character selection
    if (checkButtonPress()) {
      if (configuringSSID) {
        if (selectedChar == charsetSize - 1) {
          // Backspace functionality for SSID
          if (charIndex < inputSSID.length()) {
            inputSSID = inputSSID.substring(0, charIndex) + inputSSID.substring(charIndex + 1);
            if (charIndex > 0) charIndex--;
            updateSelectedCharForPosition();
            Serial.println("Backspace: removed character from SSID at position");
          } else if (charIndex > 0) {
            charIndex--;
            updateSelectedCharForPosition();
          }
        } else {
          // Set character at current position
          if (charIndex < inputSSID.length()) {
            inputSSID.setCharAt(charIndex, charset[selectedChar]);
          } else {
            while (inputSSID.length() < charIndex) {
              inputSSID += " ";
            }
            inputSSID += charset[selectedChar];
          }
          charIndex++;
          updateSelectedCharForPosition();
        }
      } else {
        if (selectedChar == charsetSize - 1) {
          // Backspace functionality for password
          if (charIndex < inputPassword.length()) {
            inputPassword = inputPassword.substring(0, charIndex) + inputPassword.substring(charIndex + 1);
            if (charIndex > 0) charIndex--;
            updateSelectedCharForPosition();
            Serial.println("Backspace: removed character from password at position");
          } else if (charIndex > 0) {
            charIndex--;
            updateSelectedCharForPosition();
          }
        } else {
          // Set character at current position
          if (charIndex < inputPassword.length()) {
            inputPassword.setCharAt(charIndex, charset[selectedChar]);
          } else {
            while (inputPassword.length() < charIndex) {
              inputPassword += " ";
            }
            inputPassword += charset[selectedChar];
          }
          charIndex++;
          updateSelectedCharForPosition();
        }
      }
      updateWiFiConfigDisplay();
    }
    
    delay(50);
  }
  
  lcd.noCursor();
}

void setupTime() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  Serial.println("Getting time from NTP server...");
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  } else {
    Serial.println("Time synchronized with NTP server");
  }
}

bool chooseWiFiConfigMethod() {
  int selectedOption = WIFI_CONFIG_HOTSPOT; // Start with hotspot as default
  bool optionSelected = false;
  unsigned long lastEncoderActivity = 0;
  
  // Temporarily set inMenu to true so encoder works properly
  bool wasInMenu = inMenu;
  inMenu = true;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Setup Mode:");
  
  while (!optionSelected) {
    // Display current option
    lcd.setCursor(0, 1);
    if (selectedOption == WIFI_CONFIG_HOTSPOT) {
      lcd.print("> Hotspot Mode  ");
    } else {
      lcd.print("> Manual Config ");
    }
    
    // Check for encoder activity by monitoring the lastMenuActivity
    // which gets updated by the main encoder handler
    if (lastMenuActivity > lastEncoderActivity) {
      // Encoder was turned - toggle selection
      selectedOption = (selectedOption == WIFI_CONFIG_HOTSPOT) ? WIFI_CONFIG_MANUAL : WIFI_CONFIG_HOTSPOT;
      lastEncoderActivity = lastMenuActivity;
    }
    
    // Handle button press
    if (checkButtonPress()) {
      optionSelected = true;
      break;
    }
    
    delay(50);
  }
  
  // Restore original inMenu state
  inMenu = wasInMenu;
  
  return (selectedOption == WIFI_CONFIG_HOTSPOT);
}

void startWiFiHotspot() {
  Serial.println("Starting WiFi hotspot for configuration...");
  
  // Stop any existing WiFi connection
  WiFi.disconnect();
  delay(100);
  
  // Configure hotspot
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(HOTSPOT_IP, HOTSPOT_GATEWAY, HOTSPOT_SUBNET);
  
  // Create hotspot - open network if no password, secured if password exists
  if (strlen(HOTSPOT_PASSWORD) == 0) {
    WiFi.softAP(HOTSPOT_SSID);  // Open network
  } else {
    WiFi.softAP(HOTSPOT_SSID, HOTSPOT_PASSWORD);  // Secured network
  }
  
  delay(2000); // Give time for AP to start
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Hotspot started. Connect to: ");
  Serial.println(HOTSPOT_SSID);
  
  if (strlen(HOTSPOT_PASSWORD) == 0) {
    Serial.println("Password: (Open network - no password required)");
  } else {
    Serial.print("Password: ");
    Serial.println(HOTSPOT_PASSWORD);
  }
  
  Serial.print("Then open: http://");
  Serial.println(IP);
  Serial.print("Hotspot IP confirmed: ");
  Serial.println(IP);
  Serial.println("Web server should be accessible at this IP");
  
  // Verify hotspot is actually running
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    Serial.println("WiFi AP mode confirmed active");
  } else {
    Serial.println("ERROR: WiFi AP mode not active!");
  }
}

void showHotspotInstructions() {
  static unsigned long lastScreenChange = 0;
  static int currentScreen = 0;
  
  unsigned long currentTime = millis();
  
  // Change screen every 4 seconds
  if (currentTime - lastScreenChange >= 4000) {
    lcd.clear();
    
    switch (currentScreen) {
      case 0:
        lcd.setCursor(0, 0);
        lcd.print("Connect to WiFi:");
        lcd.setCursor(0, 1);
        lcd.print("OOSIE-Radio");
        break;
        
      case 1:
        lcd.setCursor(0, 0);
        lcd.print("Open browser:");
        lcd.setCursor(0, 1);
        lcd.print("192.168.4.1");
        break;
        
      case 2:
        lcd.setCursor(0, 0);
        lcd.print("Configure WiFi");
        lcd.setCursor(0, 1);
        lcd.print("via web page");
        break;
    }
    
    currentScreen = (currentScreen + 1) % 3; // Cycle through 3 screens (was 4)
    lastScreenChange = currentTime;
  }
}

void stopWiFiHotspot() {
  if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    WiFi.softAPdisconnect(true);
    Serial.println("WiFi hotspot stopped");
  }
}

bool isHotspotActive() {
  return (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA);
}
