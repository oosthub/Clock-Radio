#ifndef MENU_H
#define MENU_H

#include "Arduino.h"

// Menu system enums and variables
enum MenuState {
  MENU_VOLUME = 0,
  MENU_STREAMS = 1,
  MENU_BRIGHTNESS = 2,
  MENU_WIFI = 3,
  MENU_COUNT = 4
};

enum WiFiMenuState {
  WIFI_MENU_IP = 0,
  WIFI_MENU_SSID = 1,
  WIFI_MENU_PASSWORD = 2,
  WIFI_MENU_RESET = 3,
  WIFI_MENU_COUNT = 4
};

// Stream structure
struct RadioStream {
  const char* name;
  const char* url;
};

// Global menu variables
extern MenuState currentMenu;
extern bool inMenu;
extern unsigned long lastMenuActivity;
extern WiFiMenuState currentWiFiMenu;
extern bool showingConfirmation;
extern bool confirmationChoice;
extern bool brightnessChanged;
extern RadioStream streams[];
extern const int streamCount;
extern int playingStream;

// Function declarations
void enterMenu();
void exitMenu();
void nextMenuItem();
void printCurrentMenu();
void displayCurrentMenu();
void selectStream();
void handleMenuEncoderClockwise(unsigned long currentTime);
void handleMenuEncoderCounterClockwise(unsigned long currentTime);
void handleMenuButtonPress();
void resetWiFiSettings();

#endif
