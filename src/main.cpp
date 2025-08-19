//ESP32 Internet Radio
//https://en.https://polluxlabs.io/wp-content/uploads/2020/02/ESP8266-Webserver-small.jpg.net
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "time.h"
#include "EEPROM.h"

// Connections ESP32 <-> Amplifier
#define I2S_DOUT  2
#define I2S_BCLK  3
#define I2S_LRC   4

// Rotary Encoder pins
#define ENCODER_A    10
#define ENCODER_B    11
#define ENCODER_BTN  12

// I2C LCD pins
#define SDA_PIN  8
#define SCL_PIN  9

Audio audio;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address, columns, rows

// Wifi Credentials
// String ssid =    "OOSIE";
// String password = "N0t4UF@tB0y";
String ssid = "";
String password = "";

// Encoder variables
volatile int volume = 5;  // Start volume at 5
volatile bool encoderA_last;
volatile bool encoderA_current;
volatile unsigned long lastEncoderTime = 0;
volatile int encoderCounter = 0;  // Counter for encoder pulses
const int PULSES_PER_STEP = 2;    // Require 4 pulses for 1 volume change
bool radioOn = true;  // Radio state

// Menu system variables
enum MenuState {
  MENU_VOLUME = 0,
  MENU_STREAMS = 1,
  MENU_BRIGHTNESS = 2,
  MENU_WIFI = 3,
  MENU_COUNT = 4
};

MenuState currentMenu = MENU_VOLUME;
bool inMenu = false;
unsigned long lastMenuActivity = 0;
const unsigned long MENU_TIMEOUT = 5000;  // 5 seconds timeout
int currentStreamIndex = 0;
bool brightnessChanged = false; // Flag to update backlight in main loop

// WiFi menu variables
enum WiFiMenuState {
  WIFI_MENU_SSID = 0,
  WIFI_MENU_PASSWORD = 1,
  WIFI_MENU_RESET = 2,
  WIFI_MENU_COUNT = 3
};

WiFiMenuState currentWiFiMenu = WIFI_MENU_SSID;
bool inWiFiMenu = false;
bool showingConfirmation = false;
bool confirmationChoice = false; // false = No (default), true = Yes

// Stream definitions
struct RadioStream {
  const char* name;
  const char* url;
};

RadioStream streams[] = {
  {"Jacaranda FM", "https://edge.iono.fm/xice/jacarandafm_live_medium.aac"},
  {"Pretoria FM", "https://edge.iono.fm/xice/362_medium.aac"},
  {"Lekker FM", "https://zas3.ndx.co.za:8002/stream"},
  {"Groot FM", "https://edge.iono.fm/xice/330_medium.aac"},
  {"RSG", "https://28553.live.streamtheworld.com/RSGAAC.aac"}
};

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


int currentStream = 0;
int playingStream = 0;  // Track which stream is actually playing
const int streamCount = sizeof(streams) / sizeof(streams[0]);

// Button handling variables (non-volatile for better control)
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastButtonChange = 0;
const unsigned long BUTTON_DEBOUNCE = 50;    // 50ms debounce
const unsigned long BUTTON_HOLD_TIME = 100;  // Minimum hold time

// LCD and time variables
unsigned long lastLcdUpdate = 0;
const unsigned long LCD_UPDATE_INTERVAL = 1000;  // Update LCD every second
bool isStreaming = false;
String currentStreamName = "";
bool forceImmediateLcdUpdate = false;  // Flag to force immediate LCD update

// Volume display variables
bool showVolumeDisplay = false;
unsigned long lastVolumeChange = 0;
const unsigned long VOLUME_DISPLAY_TIMEOUT = 5000;  // 5 seconds

// Backlight auto-off variables
unsigned long lastActivity = 0;
const unsigned long BACKLIGHT_TIMEOUT = 5000;  // 5 seconds
bool backlightAlwaysOn = true;  // true = always on, false = auto-off mode

// WiFi configuration variables
bool wifiConfigMode = false;
bool configuringSSID = true;  // true = SSID, false = Password
int charIndex = 0;  // Current character position in string
int selectedChar = 0;  // Current selected character from alphabet
String inputSSID = "";
String inputPassword = "";
const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz!@#$%^&*()-_=+[]{}|;:,.<>?/ ";
const int charsetSize = sizeof(charset) - 1;

// NTP time settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7200;     // GMT+2 for South Africa (adjust as needed)
const int daylightOffset_sec = 0;    // No daylight saving

// EEPROM settings
#define EEPROM_SIZE 512
#define SETTINGS_VERSION 2  // Increment version for new WiFi fields
#define EEPROM_ADDR_VERSION 0
#define EEPROM_ADDR_VOLUME 1
#define EEPROM_ADDR_STREAM 2
#define EEPROM_ADDR_BACKLIGHT 3

struct Settings {
  byte version;
  int volume;
  int currentStream;
  bool backlightAlwaysOn;
  char wifiSSID[32];
  char wifiPassword[64];
};

// Forward declarations
void printCurrentMenu();
void enterMenu();
void exitMenu();
void nextMenuItem();
void selectStream();
void updateLCD();
void setupLCD();
void setupTime();
void saveSettings();
void loadSettings();
void initializeEEPROM();
void configureWiFi();
void updateWiFiConfigDisplay();
bool connectToWiFi();
void resetWiFiConfig();
bool checkButtonPress();
bool checkLongButtonPress();
void updateSelectedCharForPosition();
String maskPassword(String pwd);
void enterWiFiMenu();
void exitWiFiMenu();
void nextWiFiMenuItem();
void handleWiFiMenuSelection();
void resetWiFiSettings();

// Function to mask password showing only last 2 characters
String maskPassword(String pwd) {
  if (pwd.length() <= 2) {
    return pwd; // Show short passwords completely
  }
  String masked = "";
  for (int i = 0; i < pwd.length() - 2; i++) {
    masked += "*";
  }
  masked += pwd.substring(pwd.length() - 2);
  return masked;
}

// Function to update selected character based on current position
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

// LCD Functions
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

void setupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Getting time from NTP server...");
  
  // Wait for time to be set
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  } else {
    Serial.println("Time synchronized with NTP server");
  }
}

// EEPROM Functions
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
    
    // Load WiFi credentials
    ssid = String(settings.wifiSSID);
    password = String(settings.wifiPassword);
    
    // Validate loaded values
    if (volume < 0) volume = 5;
    if (volume > 80) volume = 80;
    if (currentStream < 0 || currentStream >= streamCount) currentStream = 0;
    
    Serial.println("Settings loaded from EEPROM:");
    Serial.print("  Volume: ");
    Serial.println(volume);
    Serial.print("  Stream: ");
    Serial.println(currentStream);
    Serial.print("  Backlight Always On: ");
    Serial.println(backlightAlwaysOn ? "true" : "false");
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
    ssid = "";
    password = "";
    saveSettings();
  }
}

// Function to check button state (polling instead of interrupt)
bool checkButtonPress() {
  bool currentButtonState = digitalRead(ENCODER_BTN);
  
  // Check for state change
  if (currentButtonState != lastButtonState) {
    lastButtonState = currentButtonState;
    
    // Trigger on press (HIGH to LOW)
    if (currentButtonState == LOW) {
      Serial.println("Button pressed!");
      return true;
    }
  }
  
  return false;
}

// Function to check for long button press (3 seconds)
bool checkLongButtonPress() {
  static unsigned long buttonPressStart = 0;
  static bool buttonWasPressed = false;
  
  bool currentButtonState = digitalRead(ENCODER_BTN);
  
  if (currentButtonState == LOW && !buttonWasPressed) {
    // Button just pressed, start timing
    buttonPressStart = millis();
    buttonWasPressed = true;
  } else if (currentButtonState == HIGH && buttonWasPressed) {
    // Button released
    buttonWasPressed = false;
    buttonPressStart = 0;
  } else if (currentButtonState == LOW && buttonWasPressed) {
    // Button still pressed, check duration
    if (millis() - buttonPressStart >= 3000) {
      // Long press detected
      buttonWasPressed = false; // Reset to avoid multiple triggers
      buttonPressStart = 0;
      Serial.println("Long button press detected!");
      return true;
    }
  }
  
  return false;
}

// WiFi Configuration Functions
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
  updateSelectedCharForPosition();  // Set initial character selection
  
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
          // Confirm SSID and move to password
          ssid = inputSSID;
          configuringSSID = false;
          charIndex = 0;
          updateSelectedCharForPosition();  // Set character based on password content
          Serial.println("SSID confirmed, moving to password");
        }
      } else {
        if (inputPassword.length() > 0) {
          // Confirm password and attempt connection
          password = inputPassword;
          lcd.noCursor();
          
          Serial.println("Password confirmed, attempting connection");
          Serial.print("SSID: ");
          Serial.println(ssid);
          Serial.print("Password: ");
          Serial.println(password);

          // Attempt connection
          if (connectToWiFi()) {
            // Success - save settings and exit
            saveSettings();
            wifiConfigMode = false;
            Serial.println("WiFi configured successfully");
          } else {
            // Failed - go back to SSID configuration but keep entered values
            configuringSSID = true;
            charIndex = 0;
            updateSelectedCharForPosition();  // Set character based on SSID content
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
            // Remove character at current position and move cursor back
            inputSSID = inputSSID.substring(0, charIndex) + inputSSID.substring(charIndex + 1);
            if (charIndex > 0) charIndex--;
            updateSelectedCharForPosition();
            Serial.println("Backspace: removed character from SSID at position");
          } else if (charIndex > 0) {
            // At end of string, just move cursor back
            charIndex--;
            updateSelectedCharForPosition();
          }
        } else {
          // Set character at current position
          if (charIndex < inputSSID.length()) {
            // Replace existing character
            inputSSID.setCharAt(charIndex, charset[selectedChar]);
          } else {
            // Extend string with spaces if needed, then set character
            while (inputSSID.length() < charIndex) {
              inputSSID += " ";
            }
            inputSSID += charset[selectedChar];
          }
          // Move to next position
          charIndex++;
          updateSelectedCharForPosition();
        }
      } else {
        if (selectedChar == charsetSize - 1) {
          // Backspace functionality for password
          if (charIndex < inputPassword.length()) {
            // Remove character at current position and move cursor back
            inputPassword = inputPassword.substring(0, charIndex) + inputPassword.substring(charIndex + 1);
            if (charIndex > 0) charIndex--;
            updateSelectedCharForPosition();
            Serial.println("Backspace: removed character from password at position");
          } else if (charIndex > 0) {
            // At end of string, just move cursor back
            charIndex--;
            updateSelectedCharForPosition();
          }
        } else {
          // Set character at current position
          if (charIndex < inputPassword.length()) {
            // Replace existing character
            inputPassword.setCharAt(charIndex, charset[selectedChar]);
          } else {
            // Extend string with spaces if needed, then set character
            while (inputPassword.length() < charIndex) {
              inputPassword += " ";
            }
            inputPassword += charset[selectedChar];
          }
          // Move to next position
          charIndex++;
          updateSelectedCharForPosition();
        }
      }
      updateWiFiConfigDisplay();
    }
    
    delay(50);  // Small delay for responsiveness
  }
  
  lcd.noCursor();
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
  
  lcd.clear();
  
  // Check for volume display timeout
  if (showVolumeDisplay && (millis() - lastVolumeChange > VOLUME_DISPLAY_TIMEOUT)) {
    showVolumeDisplay = false;
  }
  
  if (inMenu) {
    // Display current menu
    switch (currentMenu) {
      case MENU_VOLUME: {
        lcd.setCursor(0, 0);
        lcd.print("MENU: Volume");
        lcd.setCursor(0, 1);
        lcd.print("Level: ");
        lcd.print(volume);
        break;
      }
      case MENU_STREAMS: {
        lcd.setCursor(0, 0);
        lcd.print("MENU: Station");
        lcd.setCursor(0, 1);
        String streamName = streams[currentStream].name;
        if (streamName.length() > 16) {
          streamName = streamName.substring(0, 16);
        }
        lcd.print(streamName);
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
    }
    return;
  }
  
  // Show volume display when adjusting volume outside menu
  if (showVolumeDisplay) {
    lcd.setCursor(0, 0);
    lcd.print("Volume");
    lcd.setCursor(0, 1);
    lcd.print("Level: ");
    lcd.print(volume);
    return;
  }
  
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return; // Skip update if time not available
  }
  
  char timeStr[6];
  strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
  
  if (isStreaming) {
    // Top line: Time only (centered)
    lcd.setCursor((16 - strlen(timeStr)) / 2, 0);
    lcd.print(timeStr);
    
    // Bottom line: Current stream name (truncate if too long and center)
    lcd.setCursor(0, 1);
    String displayName = currentStreamName;
    if (displayName.length() > 16) {
      displayName = displayName.substring(0, 16);
    }
    // Center the stream name
    int padding = (16 - displayName.length()) / 2;
    lcd.setCursor(padding, 1);
    lcd.print(displayName);
  } else {
    // Top line: Time only (centered)
    lcd.setCursor((16 - strlen(timeStr)) / 2, 0);
    lcd.print(timeStr);
    // Bottom line: Empty
  }
}

// Encoder interrupt functions
void IRAM_ATTR handleEncoder() {
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderTime < 5) return; // Debounce
  
  encoderA_current = digitalRead(ENCODER_A);
  if (encoderA_current != encoderA_last) {
    if (digitalRead(ENCODER_B) != encoderA_current) {
      encoderCounter++;  // Clockwise
    } else {
      encoderCounter--;  // Counter-clockwise
    }
    
    // Only change value after enough pulses
    if (encoderCounter >= PULSES_PER_STEP) {
      if (wifiConfigMode) {
        // In WiFi config mode: navigate character selection
        selectedChar++;
        if (selectedChar >= charsetSize) selectedChar = 0;
        // Don't force LCD update in interrupt - set flag instead
        forceImmediateLcdUpdate = true;
      } else if (!inMenu) {
        // Default mode: control volume (don't update backlight activity)
        volume++;
        if (volume > 80) volume = 80;
        showVolumeDisplay = true;
        lastVolumeChange = currentTime;
        forceImmediateLcdUpdate = true;  // Force immediate LCD update
      } else {
        // In menu mode: control menu selection
        if (currentMenu == MENU_WIFI) {
          if (showingConfirmation) {
            // In confirmation dialog: toggle Yes/No
            confirmationChoice = !confirmationChoice;
            forceImmediateLcdUpdate = true;
          } else {
            // In WiFi menu: cycle through WiFi items
            currentWiFiMenu = (WiFiMenuState)((currentWiFiMenu + 1) % WIFI_MENU_COUNT);
            forceImmediateLcdUpdate = true;
          }
        } else if (currentMenu == MENU_STREAMS) {
          currentStream++;
          if (currentStream >= streamCount) currentStream = 0;
          forceImmediateLcdUpdate = true;  // Force immediate LCD update
        } else if (currentMenu == MENU_VOLUME) {
          // In volume menu: control volume
          volume++;
          if (volume > 80) volume = 80;
          forceImmediateLcdUpdate = true;  // Force immediate LCD update
        } else if (currentMenu == MENU_BRIGHTNESS) {
          // In brightness menu: toggle backlight mode
          backlightAlwaysOn = !backlightAlwaysOn;
          brightnessChanged = true; // Set flag for main loop
          lastActivity = currentTime; // Update activity time
          forceImmediateLcdUpdate = true;  // Force immediate LCD update
        }
      }
      encoderCounter = 0;  // Reset counter
      lastMenuActivity = currentTime;  // Update menu activity
    } else if (encoderCounter <= -PULSES_PER_STEP) {
      if (wifiConfigMode) {
        // In WiFi config mode: navigate character selection
        selectedChar--;
        if (selectedChar < 0) selectedChar = charsetSize - 1;
        // Don't force LCD update in interrupt - set flag instead
        forceImmediateLcdUpdate = true;
      } else if (!inMenu) {
        // Default mode: control volume (don't update backlight activity)
        volume--;
        if (volume < 0) volume = 0;
        showVolumeDisplay = true;
        lastVolumeChange = currentTime;
        forceImmediateLcdUpdate = true;  // Force immediate LCD update
      } else {
        // In menu mode: control menu selection
        if (currentMenu == MENU_WIFI) {
          if (showingConfirmation) {
            // In confirmation dialog: toggle Yes/No
            confirmationChoice = !confirmationChoice;
            forceImmediateLcdUpdate = true;
          } else {
            // In WiFi menu: cycle through WiFi items
            currentWiFiMenu = (WiFiMenuState)((currentWiFiMenu - 1 + WIFI_MENU_COUNT) % WIFI_MENU_COUNT);
            forceImmediateLcdUpdate = true;
          }
        } else if (currentMenu == MENU_STREAMS) {
          currentStream--;
          if (currentStream < 0) currentStream = streamCount - 1;
          forceImmediateLcdUpdate = true;  // Force immediate LCD update
        } else if (currentMenu == MENU_VOLUME) {
          // In volume menu: control volume
          volume--;
          if (volume < 0) volume = 0;
          forceImmediateLcdUpdate = true;  // Force immediate LCD update
        } else if (currentMenu == MENU_BRIGHTNESS) {
          // In brightness menu: toggle backlight mode
          backlightAlwaysOn = !backlightAlwaysOn;
          brightnessChanged = true; // Set flag for main loop
          lastActivity = currentTime; // Update activity time
          forceImmediateLcdUpdate = true;  // Force immediate LCD update
        }
      }
      encoderCounter = 0;  // Reset counter
      lastMenuActivity = currentTime;  // Update menu activity
    }
    
    lastEncoderTime = currentTime;
  }
  encoderA_last = encoderA_current;
}

// Menu functions
void enterMenu() {
  inMenu = true;
  lastMenuActivity = millis();
  printCurrentMenu();
  forceImmediateLcdUpdate = true;  // Force immediate LCD update when entering menu
}

void exitMenu() {
  inMenu = false;
  showingConfirmation = false;  // Clear any confirmation dialogs
  Serial.println("Exiting menu - Volume control active");
  forceImmediateLcdUpdate = true;  // Force immediate LCD update when exiting menu
}

void printCurrentMenu() {
  switch (currentMenu) {
    case MENU_VOLUME:
      Serial.println("MENU: Volume");
      Serial.print("Current Volume: ");
      Serial.println(volume);
      break;
    case MENU_STREAMS:
      Serial.println("MENU: Streams");
      Serial.print("Current Stream: ");
      Serial.print(streams[currentStream].name);
      if (currentStream == playingStream) {
        Serial.println(" (PLAYING)");
      } else {
        Serial.println("");
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
  }
}

void nextMenuItem() {
  currentMenu = (MenuState)((currentMenu + 1) % MENU_COUNT);
  printCurrentMenu();
  forceImmediateLcdUpdate = true;  // Force immediate LCD update when changing menu
}

void selectStream() {
  Serial.print("Connecting to: ");
  Serial.println(streams[currentStream].name);
  audio.connecttohost(streams[currentStream].url);
  playingStream = currentStream;  // Update the playing stream index
  isStreaming = true;  // Set streaming status
  currentStreamName = streams[currentStream].name;  // Update current stream name
  saveSettings(); // Save stream selection
}

// WiFi menu functions
void enterWiFiMenu() {
  inWiFiMenu = true;
  currentWiFiMenu = WIFI_MENU_SSID;
  showingConfirmation = false;
  confirmationChoice = false;
  forceImmediateLcdUpdate = true;
  Serial.println("Entered WiFi submenu");
}

void exitWiFiMenu() {
  inWiFiMenu = false;
  showingConfirmation = false;
  forceImmediateLcdUpdate = true;
  Serial.println("Exited WiFi submenu");
}

void nextWiFiMenuItem() {
  if (showingConfirmation) {
    confirmationChoice = !confirmationChoice; // Toggle Yes/No
  } else {
    currentWiFiMenu = (WiFiMenuState)((currentWiFiMenu + 1) % WIFI_MENU_COUNT);
  }
  forceImmediateLcdUpdate = true;
  Serial.print("WiFi Menu Item: ");
  if (showingConfirmation) {
    Serial.println(confirmationChoice ? "YES" : "NO");
  } else {
    switch (currentWiFiMenu) {
      case WIFI_MENU_SSID: Serial.println("SSID"); break;
      case WIFI_MENU_PASSWORD: Serial.println("PASSWORD"); break;
      case WIFI_MENU_RESET: Serial.println("RESET"); break;
    }
  }
}

void handleWiFiMenuSelection() {
  if (showingConfirmation) {
    if (confirmationChoice) {
      // User selected YES - reset WiFi
      resetWiFiSettings();
    } else {
      // User selected NO - return to WiFi menu
      showingConfirmation = false;
      forceImmediateLcdUpdate = true;
    }
  } else {
    switch (currentWiFiMenu) {
      case WIFI_MENU_SSID:
      case WIFI_MENU_PASSWORD:
        // Informative items - move to next WiFi menu item
        nextWiFiMenuItem();
        break;
      case WIFI_MENU_RESET:
        // Show confirmation dialog
        showingConfirmation = true;
        confirmationChoice = false; // Default to NO
        forceImmediateLcdUpdate = true;
        Serial.println("Showing WiFi reset confirmation");
        break;
    }
  }
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
  lastButtonState = digitalRead(ENCODER_BTN);
  buttonState = lastButtonState;
  
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
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  
  // Setup time after WiFi connection
  setupTime();
  
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // Volume (use loaded value from EEPROM)
  audio.setVolume(volume);  // Use loaded volume
  Serial.print("Initial Volume: ");
  Serial.println(volume);
  // Radio stream (use loaded stream from EEPROM)
  audio.connecttohost(streams[currentStream].url);  // Use loaded stream
  playingStream = currentStream;  // Initialize playing stream
  isStreaming = true;  // Set streaming status
  currentStreamName = streams[currentStream].name;  // Set current stream name
  Serial.print("Connected to: ");
  Serial.println(streams[currentStream].name);
}
void loop()
{
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
      // Navigate to next menu item or execute action
      if (currentMenu == MENU_WIFI) {
        if (showingConfirmation) {
          // Handle confirmation dialog
          if (confirmationChoice) {
            // User selected YES - reset WiFi
            resetWiFiSettings();
          } else {
            // User selected NO - return to WiFi menu
            showingConfirmation = false;
            forceImmediateLcdUpdate = true;
          }
        } else {
          // Handle WiFi menu items
          switch (currentWiFiMenu) {
            case WIFI_MENU_SSID:
            case WIFI_MENU_PASSWORD:
              // Informative items - move to next main menu
              nextMenuItem();
              break;
            case WIFI_MENU_RESET:
              // Show confirmation dialog
              showingConfirmation = true;
              confirmationChoice = false; // Default to NO
              forceImmediateLcdUpdate = true;
              Serial.println("Showing WiFi reset confirmation");
              break;
          }
        }
      } else if (currentMenu == MENU_STREAMS) {
        // In streams menu, button press behavior depends on selection
        if (currentStream == playingStream) {
          // If selected stream is the current playing stream, go to next menu
          nextMenuItem();
        } else {
          // If different stream selected, connect to it and exit menu
          selectStream();
          exitMenu();  // Return to volume control after selection
        }
      } else {
        // Move to next menu item for other menus
        nextMenuItem();
      }
    }
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
  
  // Process audio
  audio.loop();
  
  // Update LCD display
  updateLCD();
}
// Print station info
void audio_info(const char *info) {
  Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info) { //id3 metadata
  Serial.print("id3data     "); Serial.println(info);
}
void audio_eof_mp3(const char *info) { //end of file
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
void audio_commercial(const char *info) { //duration in sec
  Serial.print("commercial  "); Serial.println(info);
}
void audio_icyurl(const char *info) { //homepage
  Serial.print("icyurl      "); Serial.println(info);
}
void audio_lasthost(const char *info) { //stream URL played
  Serial.print("lasthost    "); Serial.println(info);
}
void audio_eof_speech(const char *info) {
  Serial.print("eof_speech  "); Serial.println(info);
}