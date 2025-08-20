#ifndef ENCODER_H
#define ENCODER_H

#include "Arduino.h"

// Encoder variables
extern volatile bool encoderA_last;
extern volatile unsigned long lastEncoderTime;
extern volatile int encoderCounter;

// Function declarations
void IRAM_ATTR handleEncoder();
bool checkButtonPress();
bool checkLongButtonPress();

#endif
