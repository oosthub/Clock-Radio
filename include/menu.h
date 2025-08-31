#ifndef MENU_H
#define MENU_H

#include "Arduino.h"
#include "settings.h"

// Menu system enums and variables
enum MenuState {
  MENU_SLEEP = 0,
  MENU_STREAMS = 1,
  MENU_BRIGHTNESS = 2,
  MENU_WIFI = 3,
  MENU_WEATHER = 4,
  MENU_ALARMS = 5,
  MENU_SYSTEM = 6,
  MENU_COUNT = 7
};

enum SleepMenuState {
  SLEEP_MENU_BLANK = 0,
  SLEEP_MENU_OFF = 1,
  SLEEP_MENU_15MIN = 2,
  SLEEP_MENU_30MIN = 3,
  SLEEP_MENU_60MIN = 4,
  SLEEP_MENU_90MIN = 5,
  SLEEP_MENU_5MIN = 6,
  SLEEP_MENU_COUNT = 7
};

enum WiFiMenuState {
  WIFI_MENU_IP = 0,
  WIFI_MENU_SSID = 1,
  WIFI_MENU_PASSWORD = 2,
  WIFI_MENU_RESET = 3,
  WIFI_MENU_COUNT = 4
};

enum WeatherMenuState {
  WEATHER_MENU_TEMPERATURE = 0,
  WEATHER_MENU_HUMIDITY = 1,
  WEATHER_MENU_DESCRIPTION = 2,
  WEATHER_MENU_API_KEY = 3,
  WEATHER_MENU_UPDATE = 4,
  WEATHER_MENU_COUNT = 5
};

enum SystemMenuState {
  SYSTEM_MENU_FIRMWARE = 0,
  SYSTEM_MENU_UPDATE = 1,
  SYSTEM_MENU_COUNT = 2
};

enum AlarmMenuState {
  ALARM_MENU_SLOT1 = 0,
  ALARM_MENU_SLOT2 = 1,
  ALARM_MENU_SLOT3 = 2,
  ALARM_MENU_SLOT4 = 3,
  ALARM_MENU_SLOT5 = 4,
  ALARM_MENU_BLANK = 5,
  ALARM_MENU_COUNT = 6
};

enum AlarmSubMenuState {
  ALARM_SUB_BACK = 0,
  ALARM_SUB_ENABLED = 1,
  ALARM_SUB_TIME = 2,
  ALARM_SUB_STATION = 3,
  ALARM_SUB_SCHEDULE = 4,
  ALARM_SUB_VOLUME = 5,
  ALARM_SUB_AUTO_OFF = 6,
  ALARM_SUB_COUNT = 7
};

// Stream structure - changed to support dynamic allocation
struct RadioStream {
  char name[17];   // 16 characters + null terminator (same as WebRadioStream)
  char url[256];   // URL for the stream (same as WebRadioStream)
};

// Maximum number of radio streams
#define MAX_MENU_STREAMS 20

// Global menu variables
extern MenuState currentMenu;
extern bool inMenu;
extern unsigned long lastMenuActivity;
extern SleepMenuState currentSleepMenu;
extern WiFiMenuState currentWiFiMenu;
extern WeatherMenuState currentWeatherMenu;
extern SystemMenuState currentSystemMenu;
extern AlarmMenuState currentAlarmMenu;
extern AlarmSubMenuState currentAlarmSubMenu;
extern int currentAlarmSlot;
extern bool inAlarmSubMenu;
extern bool editingAlarmOption;
extern bool editingTime;
extern bool editingHours;
extern bool editingMinutes;
extern bool showingConfirmation;
extern bool confirmationChoice;
extern bool brightnessChanged;
extern RadioStream menuStreams[MAX_MENU_STREAMS];
extern int menuStreamCount;
extern int playingStream;

// Sleep timer variables
extern unsigned long sleepTimerStart;
extern unsigned long sleepTimerDuration;
extern bool sleepTimerActive;

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
void loadMenuStreamsFromFile();
void createDefaultStreamsFile();
void loadDefaultStreamsToMemory();
void displayWeatherMenu();
void handleWeatherMenuButtonPress();
void handleSystemMenuButtonPress();
void handleSleepMenuButtonPress();
void setSleepTimer(int minutes);
void checkSleepTimer();
void displayAlarmMenu();
void handleAlarmMenuButtonPress();

#endif
