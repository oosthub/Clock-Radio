#ifndef WEATHER_H
#define WEATHER_H

#include "Arduino.h"
#include <asyncHTTPrequest.h>

// Weather data structure
struct WeatherData {
  float temperature;
  int humidity;
  String description;
  String icon;
  bool valid;
};

// Weather configuration
extern String weatherLocation;
extern WeatherData currentWeather;
extern unsigned long lastWeatherUpdate;

// Function declarations
void initWeather();
void updateWeather();
bool fetchWeatherData();
String getWeatherIcon(const String& iconCode);
String formatTemperature(float temp);
void forceWeatherUpdate();
void onGeoRequestComplete(void* optParm, asyncHTTPrequest* request, int readyState);
void onWeatherRequestComplete(void* optParm, asyncHTTPrequest* request, int readyState);

#endif
