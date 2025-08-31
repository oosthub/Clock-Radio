#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include "Arduino.h"

// OTA update result codes
enum OTAResult {
  OTA_SUCCESS = 0,
  OTA_NO_UPDATE = 1,
  OTA_DOWNLOAD_FAILED = 2,
  OTA_INSTALL_FAILED = 3,
  OTA_NETWORK_ERROR = 4,
  OTA_PARSE_ERROR = 5
};

// Function declarations
void initOTA();
OTAResult checkForUpdate();
String getCurrentVersion();
String getLatestVersion();
bool downloadAndInstallUpdate();
void displayUpdateProgress(int percentage);

// GitHub repository information
#define GITHUB_OWNER "oosthub"
#define GITHUB_REPO "Clock-Radio"
#define GITHUB_API_URL "https://api.github.com/repos/oosthub/Clock-Radio/releases/latest"

#endif
