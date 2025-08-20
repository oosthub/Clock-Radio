#include "weather.h"
#include "settings.h"
#include <asyncHTTPrequest.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Forward declarations for menu variables
extern bool inMenu;
extern int currentMenu;  // Using int instead of MenuState to avoid circular includes
extern bool forceImmediateLcdUpdate;

// Weather variables
String weatherLocation = "";
WeatherData currentWeather = {0, 0, "", "", false};
unsigned long lastWeatherUpdate = 0;
bool weatherDataReceived = false; // Track if we've successfully received weather data

// Weather update intervals
const unsigned long WEATHER_UPDATE_INTERVAL = 30 * 60 * 1000; // 30 minutes for regular updates
const unsigned long WEATHER_INITIAL_RETRY = 60 * 1000; // 1 minute for initial attempts
const unsigned long WEATHER_FAILED_RETRY = 5 * 60 * 1000; // 5 minutes for failed requests

// Async HTTP request objects
asyncHTTPrequest geoRequest;
asyncHTTPrequest weatherRequest;
bool geoRequestPending = false;
bool weatherRequestPending = false;

void initWeather() {
  // Initialize async HTTP request objects
  geoRequest.setTimeout(5000); // 5 second timeout
  weatherRequest.setTimeout(5000); // 5 second timeout
  
  // Set up callbacks
  geoRequest.onReadyStateChange(onGeoRequestComplete);
  weatherRequest.onReadyStateChange(onWeatherRequestComplete);
}

void updateWeather() {
  unsigned long currentTime = millis();
  
  // Determine appropriate update interval based on current state
  bool needsUpdate = false;
  unsigned long nextRetryInterval;
  
  if (!weatherDataReceived) {
    // Initial fetch: Retry every minute until successful
    nextRetryInterval = WEATHER_INITIAL_RETRY;
    needsUpdate = (currentTime - lastWeatherUpdate >= nextRetryInterval);
  } else if (currentWeather.valid) {
    // Regular updates: Every 30 minutes once working
    nextRetryInterval = WEATHER_UPDATE_INTERVAL;
    needsUpdate = (currentTime - lastWeatherUpdate >= nextRetryInterval);
  } else {
    // Failed requests: Retry in 5 minutes
    nextRetryInterval = WEATHER_FAILED_RETRY;
    needsUpdate = (currentTime - lastWeatherUpdate >= nextRetryInterval);
  }
  
  if (needsUpdate && WiFi.status() == WL_CONNECTED && weatherApiKey.length() > 0) {
    if (fetchWeatherData()) {
      lastWeatherUpdate = currentTime;
      weatherDataReceived = true; // Mark that we've successfully received data
    } else {
      lastWeatherUpdate = currentTime; // Set timer for retry logic
    }
  }
}

// Async callback for geolocation request
void onGeoRequestComplete(void* optParm, asyncHTTPrequest* request, int readyState) {
  if (readyState == 4) { // Request completed
    geoRequestPending = false;
    
    if (request->responseHTTPcode() == 200) {
      String response = request->responseText();
      
      // Parse geolocation response
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, response);
      
      if (!error) {
        float lat = doc["lat"];
        float lon = doc["lon"];
        String city = doc["city"];
        String country = doc["country"];
        
        weatherLocation = city + ", " + country;
        
        // Now make weather request with coordinates
        String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?lat=" + 
                           String(lat) + "&lon=" + String(lon) + 
                           "&appid=" + weatherApiKey + "&units=metric";
        
        
        weatherRequestPending = true;
        weatherRequest.open("GET", weatherUrl.c_str());
        weatherRequest.send();
      }
    }
  }
}

// Async callback for weather request
void onWeatherRequestComplete(void* optParm, asyncHTTPrequest* request, int readyState) {
  if (readyState == 4) { // Request completed
    weatherRequestPending = false;
    
    if (request->responseHTTPcode() == 200) {
      String response = request->responseText();
      
      // Parse weather response
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, response);
      
      if (!error) {
        currentWeather.temperature = doc["main"]["temp"];
        currentWeather.humidity = doc["main"]["humidity"];
        currentWeather.description = doc["weather"][0]["description"].as<String>();
        String iconCode = doc["weather"][0]["icon"].as<String>();
        currentWeather.icon = getWeatherIcon(iconCode);
        currentWeather.valid = true;
        weatherDataReceived = true;
        
        // Force display update when weather data arrives
        if (inMenu && currentMenu == 4) { // 4 = MENU_WEATHER
          forceImmediateLcdUpdate = true;
        } else {
          // Also update main display with new weather data
          forceImmediateLcdUpdate = true;
        }
        
      } else {
        currentWeather.valid = false;
      }
    } else {
      currentWeather.valid = false;
    }
  }
}

bool fetchWeatherData() {
  if (weatherApiKey.length() == 0) {
    Serial.println("Weather API key not configured");
    return false;
  }
  
  // Check if any request is already pending
  if (geoRequestPending || weatherRequestPending) {
    Serial.println("Weather request already in progress (async)");
    return false;
  }
  
  // Start with geolocation to get coordinates
  String geoUrl = "http://ip-api.com/json/?fields=status,lat,lon,city,country";
  
  Serial.println("Starting async geolocation request...");
  geoRequestPending = true;
  geoRequest.open("GET", geoUrl.c_str());
  geoRequest.send();
  
  return true; // Request started successfully
}

String getWeatherIcon(const String& iconCode) {
  // Map OpenWeatherMap icon codes to simple characters for LCD display
  // Using custom characters: 2=sun, 3=cloud, R=rain, S=snow, M=mist
  if (iconCode.startsWith("01")) return String((char)2); // clear sky (sun)
  if (iconCode.startsWith("02")) return String((char)3); // few clouds (cloud)
  if (iconCode.startsWith("03")) return String((char)3); // scattered clouds
  if (iconCode.startsWith("04")) return String((char)3); // broken clouds
  if (iconCode.startsWith("09")) return "R"; // shower rain
  if (iconCode.startsWith("10")) return "R"; // rain
  if (iconCode.startsWith("11")) return "T"; // thunderstorm
  if (iconCode.startsWith("13")) return "S"; // snow
  if (iconCode.startsWith("50")) return "M"; // mist
  return "?"; // unknown
}

String formatTemperature(float temp) {
  // Format temperature with custom degree symbol (character 1)
  return String((int)round(temp)) + String((char)1) + "C";
}

void forceWeatherUpdate() {
  if (WiFi.status() == WL_CONNECTED && weatherApiKey.length() > 0) {
    Serial.println("Forcing immediate weather update...");
    if (fetchWeatherData()) {
      lastWeatherUpdate = millis();
      weatherDataReceived = true;
      Serial.print("Weather updated: ");
      Serial.print(currentWeather.temperature);
      Serial.print("°C, ");
      Serial.println(currentWeather.description);
    } else {
      Serial.println("Failed to fetch weather data");
    }
  } else if (weatherApiKey.length() == 0) {
    Serial.println("Cannot update weather: API key not configured");
  } else {
    Serial.println("Cannot update weather: WiFi not connected");
  }
}
