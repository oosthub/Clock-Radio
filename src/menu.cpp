#include "menu.h"
#include "config.h"
#include "settings.h"
#include "display.h"
#include "Audio.h"
#include "WiFi.h"

// Menu variables
MenuState currentMenu = MENU_VOLUME;
bool inMenu = false;
unsigned long lastMenuActivity = 0;
WiFiMenuState currentWiFiMenu = WIFI_MENU_IP;
bool showingConfirmation = false;
bool confirmationChoice = false;
bool brightnessChanged = false;
int playingStream = 0;

// Stream definitions
RadioStream streams[] = {
  {"Jacaranda FM", "https://edge.iono.fm/xice/jacarandafm_live_medium.aac"},
  {"Pretoria FM", "https://edge.iono.fm/xice/362_medium.aac"},
  {"Lekker FM", "https://zas3.ndx.co.za:8002/stream"},
  {"Groot FM", "https://edge.iono.fm/xice/330_medium.aac"},
  {"RSG", "https://28553.live.streamtheworld.com/RSGAAC.aac"}
};

const int streamCount = sizeof(streams) / sizeof(streams[0]);

// External audio object (defined in main.cpp)
extern Audio audio;

void enterMenu() {
  inMenu = true;
  lastMenuActivity = millis();
  printCurrentMenu();
  forceImmediateLcdUpdate = true;
}

void exitMenu() {
  inMenu = false;
  showingConfirmation = false;
  Serial.println("Exiting menu - Volume control active");
  forceImmediateLcdUpdate = true;
}

void nextMenuItem() {
  currentMenu = (MenuState)((currentMenu + 1) % MENU_COUNT);
  printCurrentMenu();
  forceImmediateLcdUpdate = true;
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

void displayCurrentMenu() {
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
  }
}

void selectStream() {
  Serial.print("Connecting to: ");
  Serial.println(streams[currentStream].name);
  
  // Only actually connect if radio is powered on
  if (radioPowerOn) {
    audio.connecttohost(streams[currentStream].url);
    playingStream = currentStream;
    isStreaming = true;
    currentStreamName = streams[currentStream].name;
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
  } else if (currentMenu == MENU_STREAMS) {
    currentStream++;
    if (currentStream >= streamCount) currentStream = 0;
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_VOLUME) {
    volume++;
    if (volume > 80) volume = 80;
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
  } else if (currentMenu == MENU_STREAMS) {
    currentStream--;
    if (currentStream < 0) currentStream = streamCount - 1;
    forceImmediateLcdUpdate = true;
  } else if (currentMenu == MENU_VOLUME) {
    volume--;
    if (volume < 0) volume = 0;
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
