#ifndef SETTINGS_H
#define SETTINGS_H

#include "Arduino.h"

// Forward declaration for alarm structures
enum AlarmSchedule {
  ALARM_DAILY = 0,
  ALARM_WEEKDAYS = 1,
  ALARM_WEEKENDS = 2,
  ALARM_ONCE = 3,
  ALARM_SCHEDULE_COUNT = 4
};

enum AlarmAutoOff {
  AUTO_OFF_NO = 0,
  AUTO_OFF_15MIN = 1,
  AUTO_OFF_30MIN = 2,
  AUTO_OFF_60MIN = 3,
  AUTO_OFF_90MIN = 4,
  AUTO_OFF_5MIN = 5,
  AUTO_OFF_COUNT = 6
};

struct Alarm {
  bool enabled;
  int hour;          // 0-23
  int minute;        // 0-59
  int stationIndex;  // Index into menuStreams array
  AlarmSchedule schedule;
  int maxVolume;     // 1-80, volume to fade up to
  AlarmAutoOff autoOff; // Auto-off timer after alarm triggers
  bool isActive;     // Currently ringing
  bool isSnoozing;   // In snooze mode
  unsigned long snoozeStart; // When snooze started
  unsigned long alarmStart;  // When alarm first triggered (for timeout)
  char label[17];    // 16 chars + null terminator
  
  // Constructor for default values
  Alarm() : enabled(false), hour(6), minute(0), stationIndex(0), 
            schedule(ALARM_DAILY), maxVolume(20), autoOff(AUTO_OFF_NO),
            isActive(false), isSnoozing(false), snoozeStart(0), alarmStart(0) {
    strcpy(label, "Alarm");
  }
};

#define MAX_ALARMS 5
#define ALARM_SNOOZE_MINUTES 10
#define ALARM_FADE_SECONDS 30
#define ALARM_TIMEOUT_MINUTES 5

// Settings structure for EEPROM storage
struct Settings {
  byte version;
  int volume;
  int currentStream;
  bool backlightAlwaysOn;
  bool radioPowerOn;
  char wifiSSID[32];
  char wifiPassword[64];
  char weatherApiKey[64];
  Alarm alarms[5];  // Array of 5 alarms
};

// Global settings variables
extern String ssid;
extern String password;
extern String weatherApiKey;
extern volatile int volume;
extern int currentStream;
extern bool backlightAlwaysOn;
extern bool radioPowerOn;

// Global alarm variables
extern Alarm alarms[5];

// Function declarations
void initializeEEPROM();
void saveSettings();
void loadSettings();
void resetAlarm(int alarmIndex);

#endif
