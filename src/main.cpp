//ESP32 Internet Radio
//https://en.https://polluxlabs.io/wp-content/uploads/2020/02/ESP8266-Webserver-small.jpg.net
#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
// Connections ESP32 <-> Amplifier
#define I2S_DOUT  2
#define I2S_BCLK  3
#define I2S_LRC   4
Audio audio;
// Wifi Credentials
String ssid =    "OOSIE";
String password = "N0t4UF@tB0y";
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
  audio.setVolume(5);
  // Radio stream, e.g. Byte.fm
  //audio.connecttohost("https://edge.iono.fm/xice/362_medium.aac");
  audio.connecttohost("https://edge.iono.fm/xice/jacarandafm_live_medium.aac");
}
void loop()
{
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