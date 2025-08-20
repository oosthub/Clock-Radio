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

// Custom characters
extern byte backspaceSymbol[8];

// Function declarations
void scanI2C();
void setupLCD();
void updateLCD();
void updateLCDLine(int line, String content, bool center = false);
void displayCurrentMenuOptimized();

#endif
