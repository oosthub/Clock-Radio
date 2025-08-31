# OTA Update System - Testing Guide

## Features Added

### 1. System Menu
- New main menu: **System** (accessible via rotary encoder)
- Two sub-options:
  - **Firm: v1.0.1** (displays current firmware version, navigational only)
  - **Update** (checks for and installs firmware updates)

### 2. OTA Update Process
1. **Check for Updates**: Queries GitHub API for latest release
2. **Version Comparison**: Compares current vs latest version using semantic versioning
3. **Download**: Downloads firmware binary from GitHub release assets
4. **Install**: Writes new firmware to flash memory with progress display
5. **Reboot**: Automatically restarts with new firmware

### 3. User Experience
- **No Update Available**: Shows "Up to Date" message for 2 seconds
- **Update Available**: Shows "Update found! Installing..." and progress
- **Update Progress**: Real-time percentage and progress bar on LCD
- **Error Handling**: Shows appropriate error messages for network/update failures

## Testing the System

### 1. Menu Navigation Test
```
1. Short press encoder button → Enter menu
2. Rotate to navigate to "MENU: System"
3. Rotate encoder within System menu:
   - "Firm: v1.0.1" → Shows current version (info only)
   - "Update" → Triggers update check
```

### 2. Update Process Test
```
With WiFi connected:
1. Navigate to System → Update
2. Press button to start update check
3. System will:
   - Show "Checking for updates..."
   - Query GitHub for latest release
   - Compare versions
   - Show result (Up to Date or start download)
```

### 3. Simulating New Version
To test the update process:
1. Create a new release (v1.0.2) on GitHub
2. Upload firmware binary named "clock-radio-firmware-v1.0.2.bin"
3. Radio will detect and download the new version

## GitHub Release Requirements

For OTA updates to work, releases must:
1. **Follow semantic versioning**: v1.0.1, v1.0.2, etc.
2. **Include firmware binary**: Named "clock-radio-firmware-vX.X.X.bin"
3. **Be public releases**: Not drafts or pre-releases

## Files Modified/Created

### New Files:
- `include/ota_update.h` - OTA update header
- `src/ota_update.cpp` - OTA update implementation

### Modified Files:
- `include/menu.h` - Added System menu enums
- `src/menu.cpp` - Added System menu handling
- `src/display.cpp` - Added System menu display
- `src/main.cpp` - Added OTA initialization
- `include/config.h` - Added FIRMWARE_VERSION

## Technical Details

### Version Checking
- Connects to: `https://api.github.com/repos/oosthub/Clock-Radio/releases/latest`
- Parses JSON response for `tag_name`
- Compares using semantic versioning logic

### Download Process
- Downloads firmware from GitHub release assets
- Writes directly to flash memory using ESP32 Update library
- Shows progress on LCD with percentage and progress bar
- Handles network timeouts and write failures

### Safety Features
- Validates content length before starting
- Aborts update on any error
- Preserves existing firmware if update fails
- Only reboots after successful verification

## Network Requirements
- Active WiFi connection
- Access to api.github.com (port 443)
- Sufficient bandwidth for firmware download (~1-2MB)

## Error Messages
- **"Update Error / Check WiFi"**: Network connectivity issues
- **"Update Failed / Try again later"**: Download or installation error  
- **"Up to Date"**: No newer version available
- **"Not enough space"**: Insufficient flash memory (rare)

This system provides a professional OTA update experience similar to commercial IoT devices!
