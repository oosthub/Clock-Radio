#include "encoder.h"
#include "config.h"
#include "settings.h"
#include "display.h"
#include "menu.h"
#include "wifi_config.h"

// Encoder variables
volatile bool encoderA_last;
volatile unsigned long lastEncoderTime = 0;
volatile int encoderCounter = 0;

void IRAM_ATTR handleEncoder() {
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderTime < 5) return; // Debounce
  
  bool encoderA_current = digitalRead(ENCODER_A);
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
        handleMenuEncoderClockwise(currentTime);
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
        handleMenuEncoderCounterClockwise(currentTime);
      }
      encoderCounter = 0;  // Reset counter
      lastMenuActivity = currentTime;  // Update menu activity
    }
    
    lastEncoderTime = currentTime;
  }
  encoderA_last = encoderA_current;
}

bool checkButtonPress() {
  static unsigned long buttonPressStart = 0;
  static bool buttonWasPressed = false;
  static bool shortPressReported = false;
  
  bool currentButtonState = digitalRead(ENCODER_BTN);
  
  if (currentButtonState == LOW && !buttonWasPressed) {
    // Button just pressed, start timing
    buttonPressStart = millis();
    buttonWasPressed = true;
    shortPressReported = false;
  } else if (currentButtonState == HIGH && buttonWasPressed) {
    // Button released
    unsigned long pressDuration = millis() - buttonPressStart;
    buttonWasPressed = false;
    buttonPressStart = 0;
    
    // Only report short press if it was less than 3 seconds and not already reported
    if (pressDuration < 3000 && !shortPressReported) {
      Serial.println("Short button press detected!");
      return true;
    }
  } else if (currentButtonState == LOW && buttonWasPressed && !shortPressReported) {
    // Button still pressed, check for long press
    if (millis() - buttonPressStart >= 3000) {
      shortPressReported = true; // Prevent short press from triggering
      // Don't return true here - this will be handled by checkLongButtonPress
    }
  }
  
  return false;
}

bool checkLongButtonPress() {
  static unsigned long buttonPressStart = 0;
  static bool buttonWasPressed = false;
  static bool longPressReported = false;
  
  bool currentButtonState = digitalRead(ENCODER_BTN);
  
  if (currentButtonState == LOW && !buttonWasPressed) {
    // Button just pressed, start timing
    buttonPressStart = millis();
    buttonWasPressed = true;
    longPressReported = false;
  } else if (currentButtonState == HIGH && buttonWasPressed) {
    // Button released
    buttonWasPressed = false;
    buttonPressStart = 0;
    longPressReported = false;
  } else if (currentButtonState == LOW && buttonWasPressed && !longPressReported) {
    // Button still pressed, check duration
    if (millis() - buttonPressStart >= 3000) {
      // Long press detected
      longPressReported = true; // Prevent multiple triggers
      Serial.println("Long button press detected!");
      return true;
    }
  }
  
  return false;
}
