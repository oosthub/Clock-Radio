#ifndef DISPLAY_H
#define DISPLAY_H

#include "Arduino.h"
#include "LiquidCrystal_I2C.h"

// Display-related variables
extern LiquidCrystal_I2C lcd;
extern unsigned long lastLcdUpdate;
extern bool isStreaming;
extern String currentStreamName;
extern bool forceImmediateLcdUpdate;
extern bool showVolumeDisplay;
extern unsigned long lastVolumeChange;
extern unsigned long lastActivity;
extern bool displayJustWokenUp;
extern unsigned long displayWakeTime;

// Radio startup display control
extern bool radioJustTurnedOn;
extern bool waitingForStreamStart;
extern unsigned long radioTurnOnTime;

// Import menu variables for time editing check
extern bool editingTime;
extern bool editingHours;
extern bool editingMinutes;

// Now Playing Info variables
extern String currentTrackInfo;
extern bool hasTrackInfo;
extern bool showTrackInfo;
extern unsigned long lastTrackToggle;
extern int trackScrollPosition;
extern unsigned long lastTrackScroll;

// Temporary message display variables
extern bool showTemporaryMessage;
extern String temporaryMessage;
extern unsigned long temporaryMessageStart;
extern unsigned long temporaryMessageDuration;

// Custom characters
extern byte backspaceSymbol[8];

// Function declarations
void scanI2C();
void setupLCD();
void updateLCD();
void updateLCDLine(int line, String content, bool center = false);
void displayCurrentMenuOptimized();
void showTemporaryLCDMessage(String message, unsigned long duration = 3000);
bool hasEnabledAlarms();

#endif
