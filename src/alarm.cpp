#include "alarm.h"
#include "config.h"
#include "settings.h"
#include "display.h"
#include "menu.h"
#include "Audio.h"
#include "time.h"

// Alarm system variables
bool alarmSystemActive = false;
int activeAlarmIndex = -1;
unsigned long alarmFadeStart = 0;
int alarmCurrentVolume = 0;
int userOriginalVolume = 0; // Store user's volume before alarm

// External variables
extern Audio audio;
extern bool radioPowerOn;
extern bool isStreaming;
extern String currentStreamName;
extern volatile int volume;
extern int currentStream;
extern RadioStream menuStreams[];
extern int menuStreamCount;
extern bool forceImmediateLcdUpdate;

void initializeAlarms() {
  // Initialize all alarms to default state
  for (int i = 0; i < MAX_ALARMS; i++) {
    alarms[i].enabled = false;
    alarms[i].hour = 7;    // Default to 7:00 AM
    alarms[i].minute = 0;
    alarms[i].stationIndex = 0; // First station
    alarms[i].schedule = ALARM_WEEKDAYS;
    alarms[i].maxVolume = 40;   // Moderate volume
    alarms[i].autoOff = AUTO_OFF_NO; // Default no auto-off
    alarms[i].isActive = false;
    alarms[i].isSnoozing = false;
    alarms[i].snoozeStart = 0;
    alarms[i].alarmStart = 0;
  }
  
  // Load saved alarms from EEPROM
  loadSettings();
  
  Serial.println("Alarm system initialized");
}

void checkAlarms() {
  if (!alarmSystemActive) return;
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;
  
  // Check for alarm timeout (5 minutes after alarm starts without user interaction)
  if (activeAlarmIndex >= 0) {
    unsigned long alarmDuration = millis() - alarms[activeAlarmIndex].alarmStart;
    if (alarmDuration >= (ALARM_TIMEOUT_MINUTES * 60 * 1000)) {
      // Alarm has been active for 5 minutes - automatically stop and optionally set sleep timer
      Serial.println("Alarm timeout - automatically stopping alarm");
      
      // Check if this alarm has auto-off configured
      if (alarms[activeAlarmIndex].autoOff != AUTO_OFF_NO) {
        // Set sleep timer based on auto-off setting
        int minutes = 0;
        switch (alarms[activeAlarmIndex].autoOff) {
          case AUTO_OFF_5MIN: minutes = 5; break;
          case AUTO_OFF_15MIN: minutes = 15; break;
          case AUTO_OFF_30MIN: minutes = 30; break;
          case AUTO_OFF_60MIN: minutes = 60; break;
          case AUTO_OFF_90MIN: minutes = 90; break;
          default: break;
        }
        
        if (minutes > 0) {
          setSleepTimer(minutes);
          Serial.print("Auto-off sleep timer set for ");
          Serial.print(minutes);
          Serial.println(" minutes");
        }
      }
      
      // Stop the alarm and return to normal operation
      alarms[activeAlarmIndex].isActive = false;
      activeAlarmIndex = -1;
      alarmCurrentVolume = 0;
      
      // Restore user's original volume
      volume = userOriginalVolume;
      audio.setVolume(volume);
      
      forceImmediateLcdUpdate = true;
      return;
    }
  }
  
  // Check each alarm
  for (int i = 0; i < MAX_ALARMS; i++) {
    if (!alarms[i].enabled) continue;
    
    // Skip alarm that is currently being edited (time editing)
    if (editingTime && i == currentAlarmSlot) {
      Serial.print("Skipping alarm ");
      Serial.print(i + 1);
      Serial.println(" - currently being edited");
      continue;
    }
    
    // Check if alarm is currently snoozing
    if (alarms[i].isSnoozing) {
      if (millis() - alarms[i].snoozeStart >= (ALARM_SNOOZE_MINUTES * 60 * 1000)) {
        // Snooze time ended, trigger alarm again
        alarms[i].isSnoozing = false;
        startAlarm(i);
      }
      continue;
    }
    
    // Skip if alarm is already active
    if (alarms[i].isActive) continue;
    
    // Check if current time matches alarm time
    if (isAlarmTime(alarms[i])) {
      startAlarm(i);
      break; // Only trigger one alarm at a time
    }
  }
  
  // Update alarm fade if an alarm is active
  if (activeAlarmIndex >= 0) {
    updateAlarmFade();
  }
}

bool isAlarmTime(const Alarm& alarm) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return false;
  
  // Check time match
  if (timeinfo.tm_hour != alarm.hour || timeinfo.tm_min != alarm.minute) {
    return false;
  }
  
  // Check schedule
  switch (alarm.schedule) {
    case ALARM_DAILY:
      return true;
      
    case ALARM_WEEKDAYS:
      // Monday = 1, Friday = 5 (tm_wday: Sunday = 0, Saturday = 6)
      return (timeinfo.tm_wday >= 1 && timeinfo.tm_wday <= 5);
      
    case ALARM_WEEKENDS:
      // Saturday = 6, Sunday = 0
      return (timeinfo.tm_wday == 0 || timeinfo.tm_wday == 6);
      
    case ALARM_ONCE:
      // For "once" alarms, disable after triggering
      return true;
  }
  
  return false;
}

void startAlarm(int alarmIndex) {
  if (alarmIndex < 0 || alarmIndex >= MAX_ALARMS) return;
  
  activeAlarmIndex = alarmIndex;
  alarms[alarmIndex].isActive = true;
  alarms[alarmIndex].alarmStart = millis(); // Record when alarm started
  alarmFadeStart = millis();
  alarmCurrentVolume = 5; // Start very quiet
  
  // Save user's current volume before alarm
  userOriginalVolume = volume;
  
  // Turn on radio if it's off
  if (!radioPowerOn) {
    radioPowerOn = true;
  }
  
  // Start playing the alarm station
  if (alarms[alarmIndex].stationIndex < menuStreamCount) {
    connectToStream(alarms[alarmIndex].stationIndex);  // Use helper function for consistent behavior
  }
  
  // Set initial alarm volume (don't modify global volume variable)
  audio.setVolume(alarmCurrentVolume);
  
  // If this was a "once" alarm, disable it
  if (alarms[alarmIndex].schedule == ALARM_ONCE) {
    alarms[alarmIndex].enabled = false;
    saveSettings();
  }
  
  forceImmediateLcdUpdate = true;
  
  Serial.print("Alarm ");
  Serial.print(alarmIndex + 1);
  Serial.println(" triggered!");
}

void updateAlarmFade() {
  if (activeAlarmIndex < 0) return;
  
  unsigned long fadeTime = millis() - alarmFadeStart;
  unsigned long totalFadeTime = ALARM_FADE_SECONDS * 1000;
  
  if (fadeTime < totalFadeTime) {
    // Calculate fade volume (5 to maxVolume over ALARM_FADE_SECONDS)
    int maxVol = alarms[activeAlarmIndex].maxVolume;
    int volumeRange = maxVol - 5;
    alarmCurrentVolume = 5 + (volumeRange * fadeTime) / totalFadeTime;
    
    // Update audio volume directly (don't modify global volume variable)
    audio.setVolume(alarmCurrentVolume);
  } else {
    // Fade complete, set to max volume
    alarmCurrentVolume = alarms[activeAlarmIndex].maxVolume;
    audio.setVolume(alarmCurrentVolume);
  }
}

void stopAlarm() {
  if (activeAlarmIndex >= 0) {
    alarms[activeAlarmIndex].isActive = false;
    alarms[activeAlarmIndex].isSnoozing = false;
    activeAlarmIndex = -1;
    alarmCurrentVolume = 0;
    
    // Restore user's original volume
    volume = userOriginalVolume;
    audio.setVolume(volume);
    
    // Show confirmation message
    showTemporaryLCDMessage("STOPPED", 2000);
    
    Serial.println("Alarm stopped");
  }
}

void snoozeAlarm() {
  if (activeAlarmIndex >= 0) {
    alarms[activeAlarmIndex].isActive = false;
    alarms[activeAlarmIndex].isSnoozing = true;
    alarms[activeAlarmIndex].snoozeStart = millis();
    // Keep alarmStart time for timeout tracking
    
    // Turn off radio during snooze
    radioPowerOn = false;
    audio.stopSong();
    isStreaming = false;
    
    activeAlarmIndex = -1;
    
    // Show confirmation message
    showTemporaryLCDMessage("SNOOZED 10MIN", 2000);
    
    Serial.println("Alarm snoozed for 10 minutes");
  }
}

// Get the index of the alarm that is currently snoozing (-1 if none)
int getSnoozingAlarmIndex() {
  for (int i = 0; i < MAX_ALARMS; i++) {
    if (alarms[i].isSnoozing) {
      return i;
    }
  }
  return -1;
}

// Cancel snoozing alarm
void cancelSnooze() {
  int snoozingIndex = getSnoozingAlarmIndex();
  if (snoozingIndex >= 0) {
    alarms[snoozingIndex].isSnoozing = false;
    alarms[snoozingIndex].snoozeStart = 0;
    alarms[snoozingIndex].isActive = false;
    
    // Save settings to persist the cancellation
    saveSettings();
    
    // Show confirmation message
    showTemporaryLCDMessage("ALARM STOPPED", 2000);
    
    Serial.print("Alarm ");
    Serial.print(snoozingIndex + 1);
    Serial.println(" snooze cancelled");
  }
}
