#include "ota_update.h"
#include "config.h"
#include "display.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include "Update.h"

// Version comparison helper
bool isNewerVersion(const String& current, const String& latest) {
  // Simple version comparison for semantic versioning (v1.2.3)
  // Remove 'v' prefix if present
  String currentClean = current;
  String latestClean = latest;
  
  if (currentClean.startsWith("v")) currentClean = currentClean.substring(1);
  if (latestClean.startsWith("v")) latestClean = latestClean.substring(1);
  
  // Split versions by dots
  int currentParts[3] = {0, 0, 0};
  int latestParts[3] = {0, 0, 0};
  
  // Parse current version
  int partIndex = 0;
  int lastIndex = 0;
  for (int i = 0; i <= currentClean.length() && partIndex < 3; i++) {
    if (i == currentClean.length() || currentClean.charAt(i) == '.') {
      if (i > lastIndex) {
        currentParts[partIndex] = currentClean.substring(lastIndex, i).toInt();
      }
      partIndex++;
      lastIndex = i + 1;
    }
  }
  
  // Parse latest version
  partIndex = 0;
  lastIndex = 0;
  for (int i = 0; i <= latestClean.length() && partIndex < 3; i++) {
    if (i == latestClean.length() || latestClean.charAt(i) == '.') {
      if (i > lastIndex) {
        latestParts[partIndex] = latestClean.substring(lastIndex, i).toInt();
      }
      partIndex++;
      lastIndex = i + 1;
    }
  }
  
  // Compare versions
  for (int i = 0; i < 3; i++) {
    if (latestParts[i] > currentParts[i]) return true;
    if (latestParts[i] < currentParts[i]) return false;
  }
  
  return false; // Versions are equal
}

void initOTA() {
  // OTA initialization if needed
  Serial.println("OTA Update system initialized");
}

String getCurrentVersion() {
  return FIRMWARE_VERSION;
}

String getLatestVersion() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected for version check");
    return "";
  }
  
  HTTPClient http;
  http.begin(GITHUB_API_URL);
  http.addHeader("User-Agent", "ESP32-Clock-Radio");
  
  int httpResponseCode = http.GET();
  String version = "";
  
  if (httpResponseCode == 200) {
    String payload = http.getString();
    Serial.print("GitHub API response size: ");
    Serial.println(payload.length());
    
    // Try to find tag_name directly in the string to avoid JSON parsing issues
    int tagStart = payload.indexOf("\"tag_name\":\"");
    if (tagStart != -1) {
      tagStart += 12; // Move past "tag_name":"
      int tagEnd = payload.indexOf("\"", tagStart);
      if (tagEnd != -1) {
        version = payload.substring(tagStart, tagEnd);
        Serial.print("Latest version from GitHub (string parsing): ");
        Serial.println(version);
        http.end();
        return version;
      }
    }
    
    // Fallback to JSON parsing if string parsing fails
    Serial.println("String parsing failed, trying JSON...");
    // Parse JSON response - increase memory allocation for GitHub API response
    DynamicJsonDocument doc(8192);  // Increased from 2048 to 8192 bytes
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      const char* tagName = doc["tag_name"].as<const char*>();
      if (tagName) {
        version = String(tagName);
        Serial.print("Latest version from GitHub: ");
        Serial.println(version);
      } else {
        Serial.println("No tag_name found in response");
      }
    } else {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP error code: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
  return version;
}

OTAResult checkForUpdate() {
  Serial.println("Checking for firmware updates...");
  
  // Show checking message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Checking for");
  lcd.setCursor(0, 1);
  lcd.print("updates...");
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return OTA_NETWORK_ERROR;
  }
  
  String latestVersion = getLatestVersion();
  if (latestVersion.length() == 0) {
    Serial.println("Failed to get latest version");
    return OTA_NETWORK_ERROR;
  }
  
  String currentVersion = getCurrentVersion();
  Serial.print("Current version: ");
  Serial.println(currentVersion);
  Serial.print("Latest version: ");
  Serial.println(latestVersion);
  
  if (isNewerVersion(currentVersion, latestVersion)) {
    Serial.println("New version available!");
    return OTA_SUCCESS; // New version available
  } else {
    Serial.println("No update needed");
    return OTA_NO_UPDATE;
  }
}

void displayUpdateProgress(int percentage) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Updating...");
  lcd.setCursor(0, 1);
  lcd.print(percentage);
  lcd.print("%");
  
  // Simple progress bar
  int barLength = 12; // Leave space for percentage
  int filledLength = (percentage * barLength) / 100;
  String progressBar = "";
  for (int i = 0; i < filledLength; i++) {
    progressBar += "#";
  }
  for (int i = filledLength; i < barLength; i++) {
    progressBar += "-";
  }
  
  // If percentage is 100%, show completion
  if (percentage >= 100) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Update Complete");
    lcd.setCursor(0, 1);
    lcd.print("Rebooting...");
  }
}

bool downloadAndInstallUpdate() {
  Serial.println("Starting firmware download and installation...");
  
  // Get the latest release info
  HTTPClient http;
  http.begin(GITHUB_API_URL);
  http.addHeader("User-Agent", "ESP32-Clock-Radio");
  
  int httpResponseCode = http.GET();
  if (httpResponseCode != 200) {
    Serial.print("Failed to get release info: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
  
  String payload = http.getString();
  http.end();
  
  // Parse JSON to get download URL - increase memory allocation
  DynamicJsonDocument doc(8192);  // Increased from 4096 to 8192 bytes
  DeserializationError error = deserializeJson(doc, payload);
  
  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    return false;
  }
  
  // Find the firmware binary in assets
  String downloadUrl = "";
  JsonArray assets = doc["assets"];
  
  for (JsonObject asset : assets) {
    const char* name = asset["name"].as<const char*>();
    if (name && String(name).startsWith("clock-radio-firmware-v") && String(name).endsWith(".bin")) {
      downloadUrl = asset["browser_download_url"].as<const char*>();
      break;
    }
  }
  
  if (downloadUrl.length() == 0) {
    Serial.println("Firmware binary not found in release assets");
    return false;
  }
  
  Serial.print("Downloading firmware from: ");
  Serial.println(downloadUrl);
  
  // Download and install firmware
  http.begin(downloadUrl);
  http.addHeader("User-Agent", "ESP32-Clock-Radio");
  
  httpResponseCode = http.GET();
  if (httpResponseCode != 200) {
    Serial.print("Failed to download firmware: ");
    Serial.println(httpResponseCode);
    http.end();
    return false;
  }
  
  int contentLength = http.getSize();
  Serial.print("Firmware size: ");
  Serial.println(contentLength);
  
  if (contentLength <= 0) {
    Serial.println("Invalid content length");
    http.end();
    return false;
  }
  
  // Initialize update
  if (!Update.begin(contentLength)) {
    Serial.println("Not enough space for OTA update");
    http.end();
    return false;
  }
  
  WiFiClient* client = http.getStreamPtr();
  int downloaded = 0;
  int lastProgress = -1;
  
  displayUpdateProgress(0);
  
  // Download and write firmware
  while (downloaded < contentLength) {
    size_t bytesToRead = min(1024, contentLength - downloaded);
    uint8_t buffer[1024];
    
    size_t bytesRead = client->readBytes(buffer, bytesToRead);
    if (bytesRead == 0) {
      Serial.println("Download timeout or connection lost");
      Update.abort();
      http.end();
      return false;
    }
    
    if (Update.write(buffer, bytesRead) != bytesRead) {
      Serial.println("Failed to write firmware data");
      Update.abort();
      http.end();
      return false;
    }
    
    downloaded += bytesRead;
    int progress = (downloaded * 100) / contentLength;
    
    if (progress != lastProgress && progress % 5 == 0) { // Update every 5%
      displayUpdateProgress(progress);
      lastProgress = progress;
      Serial.print("Progress: ");
      Serial.print(progress);
      Serial.println("%");
    }
    
    // Keep WiFi alive
    yield();
  }
  
  http.end();
  
  // Finish update
  if (Update.end(true)) {
    Serial.println("Update successful!");
    displayUpdateProgress(100);
    delay(2000);
    
    // Reboot
    ESP.restart();
    return true;
  } else {
    Serial.print("Update failed: ");
    Serial.println(Update.errorString());
    return false;
  }
}
