#ifndef ALARM_H
#define ALARM_H

#include "Arduino.h"
#include "settings.h"

// Alarm system functions
void initializeAlarms();
void checkAlarms();
void stopAlarm();
void snoozeAlarm();
void cancelSnooze();
int getSnoozingAlarmIndex();
bool isAlarmTime(const Alarm& alarm);
void startAlarm(int alarmIndex);
void updateAlarmFade();

// Stream connection helper (defined in main.cpp)
void connectToStream(int streamIndex);

// Alarm system variables
extern bool alarmSystemActive;
extern int activeAlarmIndex;
extern unsigned long alarmFadeStart;
extern int alarmCurrentVolume;

// Menu editing state variables (from menu.cpp)
extern bool editingTime;
extern int currentAlarmSlot;

#endif
