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

// Now Playing Info variables
extern String currentTrackInfo;
extern bool hasTrackInfo;
extern bool showTrackInfo;
extern unsigned long lastTrackToggle;
extern int trackScrollPosition;
extern unsigned long lastTrackScroll;

// Custom characters
extern byte backspaceSymbol[8];

// Function declarations
void scanI2C();
void setupLCD();
void updateLCD();
void updateLCDLine(int line, String content, bool center = false);
void displayCurrentMenuOptimized();

#endif
