#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "Arduino.h"
#include "WiFi.h"
#include "AsyncTCP.h"

// HTTP method constants (missing from ESPAsyncWebServer)
#ifndef HTTP_GET
#define HTTP_GET     1
#define HTTP_POST    2
#define HTTP_PUT     3
#define HTTP_DELETE  4
#define HTTP_PATCH   5
#define HTTP_HEAD    6
#define HTTP_OPTIONS 7
#define HTTP_ANY     255
#endif

#include "ESPAsyncWebServer.h"
#include "ArduinoJson.h"
#include "FS.h"
#include "SPIFFS.h"

// Maximum number of radio streams for file storage
#define MAX_WEB_STREAMS 20

// Web stream structure for JSON storage
struct WebRadioStream {
    char name[17];   // 16 characters + null terminator
    char url[256];   // URL for the stream
};

// Web server functions
void initWebServer();
void handleWebServer();
void loadStreamsFromFile();
void saveStreamsToFile();

// External web server object
extern AsyncWebServer server;

// External stream storage for web interface
extern WebRadioStream webStreams[MAX_WEB_STREAMS];
extern int webStreamCount;

#endif
