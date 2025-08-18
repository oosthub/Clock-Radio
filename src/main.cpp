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

// Button handling variables (non-volatile for better control)
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastButtonChange = 0;
const unsigned long BUTTON_DEBOUNCE = 50;    // 50ms debounce
const unsigned long BUTTON_HOLD_TIME = 100;  // Minimum hold time

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
    
    // Only change volume after enough pulses
    if (encoderCounter >= PULSES_PER_STEP) {
      volume++;
      if (volume > 80) volume = 80;  // Max volume 80
      encoderCounter = 0;  // Reset counter
    } else if (encoderCounter <= -PULSES_PER_STEP) {
      volume--;
      if (volume < 0) volume = 0;   // Min volume 0
      encoderCounter = 0;  // Reset counter
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
  audio.connecttohost("https://edge.iono.fm/xice/jacarandafm_live_medium.aac");
}
void loop()
{
  static int lastVolume = volume;
  
  // Handle button press (power on/off) - using polling instead of interrupt
  if (checkButtonPress()) {
    Serial.println("Button Press detected");
    radioOn = !radioOn;
    
    if (radioOn) {
      Serial.println("Radio ON");
      audio.setVolume(volume);
    } else {
      Serial.println("Radio OFF");
      audio.setVolume(0);
    }
  }
  
  // Handle volume change (only when radio is on)
  if (volume != lastVolume && radioOn) {
    audio.setVolume(volume);
    Serial.print("Volume: ");
    Serial.println(volume);
    lastVolume = volume;
  }
  
  // Only process audio when radio is on
  if (radioOn) {
    audio.loop();
  }
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