//ESP32 Internet Radio
//https://en.https://polluxlabs.io/wp-content/uploads/2020/02/ESP8266-Webserver-small.jpg.net
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"

// Connections ESP32 <-> Amplifier
#define I2S_DOUT  2
#define I2S_BCLK  3
#define I2S_LRC   4

// Rotary Encoder pins
#define ENCODER_A    10
#define ENCODER_B    11
#define ENCODER_BTN  12

Audio audio;

// Wifi Credentials
String ssid =    "OOSIE";
String password = "N0t4UF@tB0y";

// Encoder variables
volatile int volume = 5;  // Start volume at 5
volatile bool encoderA_last;
volatile bool encoderA_current;
volatile unsigned long lastEncoderTime = 0;
volatile int encoderCounter = 0;  // Counter for encoder pulses
const int PULSES_PER_STEP = 4;    // Require 4 pulses for 1 volume change
bool radioOn = true;  // Radio state

// Menu system variables
enum MenuState {
  MENU_VOLUME = 0,
  MENU_STREAMS = 1,
  MENU_COUNT = 2
};

MenuState currentMenu = MENU_VOLUME;
bool inMenu = false;
unsigned long lastMenuActivity = 0;
const unsigned long MENU_TIMEOUT = 5000;  // 5 seconds timeout

// Stream definitions
struct RadioStream {
  const char* name;
  const char* url;
};

RadioStream streams[] = {
  {"Jacaranda FM", "https://edge.iono.fm/xice/jacarandafm_live_medium.aac"},
  {"Pretoria FM", "https://edge.iono.fm/xice/362_medium.aac"},
  {"Lekker FM", "https://zas3.ndx.co.za:8002/stream"},
  {"Groot FM", "https://edge.iono.fm/xice/330_medium.aac"},
  {"RSG", "https://28553.live.streamtheworld.com/RSGAAC.aac"}
};

int currentStream = 0;
int playingStream = 0;  // Track which stream is actually playing
const int streamCount = sizeof(streams) / sizeof(streams[0]);

// Button handling variables (non-volatile for better control)
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastButtonChange = 0;
const unsigned long BUTTON_DEBOUNCE = 50;    // 50ms debounce
const unsigned long BUTTON_HOLD_TIME = 100;  // Minimum hold time

// Forward declarations
void printCurrentMenu();
void enterMenu();
void exitMenu();
void nextMenuItem();
void selectStream();

// Encoder interrupt functions
void IRAM_ATTR handleEncoder() {
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderTime < 5) return; // Debounce
  
  encoderA_current = digitalRead(ENCODER_A);
  if (encoderA_current != encoderA_last) {
    if (digitalRead(ENCODER_B) != encoderA_current) {
      encoderCounter++;  // Clockwise
    } else {
      encoderCounter--;  // Counter-clockwise
    }
    
    // Only change value after enough pulses
    if (encoderCounter >= PULSES_PER_STEP) {
      if (!inMenu) {
        // Default mode: control volume
        volume++;
        if (volume > 80) volume = 80;
      } else {
        // In menu mode: control menu selection
        if (currentMenu == MENU_STREAMS) {
          currentStream++;
          if (currentStream >= streamCount) currentStream = 0;
        } else if (currentMenu == MENU_VOLUME) {
          // In volume menu: control volume
          volume++;
          if (volume > 80) volume = 80;
        }
      }
      encoderCounter = 0;  // Reset counter
      lastMenuActivity = currentTime;  // Update menu activity
    } else if (encoderCounter <= -PULSES_PER_STEP) {
      if (!inMenu) {
        // Default mode: control volume
        volume--;
        if (volume < 0) volume = 0;
      } else {
        // In menu mode: control menu selection
        if (currentMenu == MENU_STREAMS) {
          currentStream--;
          if (currentStream < 0) currentStream = streamCount - 1;
        } else if (currentMenu == MENU_VOLUME) {
          // In volume menu: control volume
          volume--;
          if (volume < 0) volume = 0;
        }
      }
      encoderCounter = 0;  // Reset counter
      lastMenuActivity = currentTime;  // Update menu activity
    }
    
    lastEncoderTime = currentTime;
  }
  encoderA_last = encoderA_current;
}

// Function to check button state (polling instead of interrupt)
bool checkButtonPress() {
  bool currentButtonState = digitalRead(ENCODER_BTN);
  
  // Check for state change
  if (currentButtonState != lastButtonState) {
    lastButtonState = currentButtonState;
    
    // Trigger on press (HIGH to LOW)
    if (currentButtonState == LOW) {
      Serial.println("Button pressed!");
      return true;
    }
  }
  
  return false;
}

// Menu functions
void enterMenu() {
  inMenu = true;
  lastMenuActivity = millis();
  printCurrentMenu();
}

void exitMenu() {
  inMenu = false;
  Serial.println("Exiting menu - Volume control active");
}

void printCurrentMenu() {
  switch (currentMenu) {
    case MENU_VOLUME:
      Serial.println("MENU: Volume");
      Serial.print("Current Volume: ");
      Serial.println(volume);
      break;
    case MENU_STREAMS:
      Serial.println("MENU: Streams");
      Serial.print("Current Stream: ");
      Serial.print(streams[currentStream].name);
      if (currentStream == playingStream) {
        Serial.println(" (PLAYING)");
      } else {
        Serial.println("");
      }
      break;
  }
}

void nextMenuItem() {
  currentMenu = (MenuState)((currentMenu + 1) % MENU_COUNT);
  printCurrentMenu();
}

void selectStream() {
  Serial.print("Connecting to: ");
  Serial.println(streams[currentStream].name);
  audio.connecttohost(streams[currentStream].url);
  playingStream = currentStream;  // Update the playing stream index
}
void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3 Internet Radio Starting...");
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("Flash Size: ");
  Serial.println(ESP.getFlashChipSize());
  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("PSRAM Size: ");
  Serial.println(ESP.getPsramSize());
  Serial.print("Free PSRAM: ");
  Serial.println(ESP.getFreePsram());
  Serial.println("");
  
  // Setup encoder pins
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(ENCODER_BTN, INPUT_PULLUP);
  
  // Setup interrupts (only for encoder rotation, not button)
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), handleEncoder, CHANGE);
  
  // Initialize encoder state
  encoderA_last = digitalRead(ENCODER_A);
  lastButtonState = digitalRead(ENCODER_BTN);
  buttonState = lastButtonState;
  
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // Volume (0-100)
  audio.setVolume(volume);  // Use variable volume instead of hardcoded 5
  Serial.print("Initial Volume: ");
  Serial.println(volume);
  // Radio stream, e.g. Byte.fm
  //audio.connecttohost("https://edge.iono.fm/xice/362_medium.aac");
  audio.connecttohost(streams[currentStream].url);  // Use default stream
  playingStream = currentStream;  // Initialize playing stream
  Serial.print("Connected to: ");
  Serial.println(streams[currentStream].name);
}
void loop()
{
  static int lastVolume = volume;
  static int lastStream = currentStream;
  
  // Check for menu timeout
  if (inMenu && (millis() - lastMenuActivity > MENU_TIMEOUT)) {
    exitMenu();
  }
  
  // Handle button press (menu navigation)
  if (checkButtonPress()) {
    lastMenuActivity = millis();
    
    if (!inMenu) {
      // Enter menu mode
      enterMenu();
    } else {
      // Navigate to next menu item or execute action
      if (currentMenu == MENU_STREAMS) {
        // In streams menu, button press behavior depends on selection
        if (currentStream == playingStream) {
          // If selected stream is the current playing stream, go to next menu
          nextMenuItem();
        } else {
          // If different stream selected, connect to it and exit menu
          selectStream();
          exitMenu();  // Return to volume control after selection
        }
      } else {
        // Move to next menu item
        nextMenuItem();
      }
    }
  }
  
  // Handle volume change (only when not in menu or in volume menu)
  if (volume != lastVolume && (!inMenu || currentMenu == MENU_VOLUME)) {
    audio.setVolume(volume);
    Serial.print("Volume: ");
    Serial.println(volume);
    lastVolume = volume;
    
    if (inMenu && currentMenu == MENU_VOLUME) {
      lastMenuActivity = millis();  // Reset timeout when adjusting volume in menu
    }
  }
  
  // Handle stream change (when in streams menu)
  if (currentStream != lastStream && inMenu && currentMenu == MENU_STREAMS) {
    Serial.print("Selected Stream: ");
    Serial.println(streams[currentStream].name);
    lastStream = currentStream;
  }
  
  // Process audio
  audio.loop();
}
// Print station info
void audio_info(const char *info) {
  Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info) { //id3 metadata
  Serial.print("id3data     "); Serial.println(info);
}
void audio_eof_mp3(const char *info) { //end of file
  Serial.print("eof_mp3     "); Serial.println(info);
}
void audio_showstation(const char *info) {
  Serial.print("station     "); Serial.println(info);
}
void audio_showstreaminfo(const char *info) {
  Serial.print("streaminfo  "); Serial.println(info);
}
void audio_showstreamtitle(const char *info) {
  Serial.print("streamtitle "); Serial.println(info);
}
void audio_bitrate(const char *info) {
  Serial.print("bitrate     "); Serial.println(info);
}
void audio_commercial(const char *info) { //duration in sec
  Serial.print("commercial  "); Serial.println(info);
}
void audio_icyurl(const char *info) { //homepage
  Serial.print("icyurl      "); Serial.println(info);
}
void audio_lasthost(const char *info) { //stream URL played
  Serial.print("lasthost    "); Serial.println(info);
}
void audio_eof_speech(const char *info) {
  Serial.print("eof_speech  "); Serial.println(info);
}