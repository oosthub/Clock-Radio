# ESP32 Clock Radio - Release Builder
# This script builds the firmware and packages it for release

param(
    [Parameter(Mandatory=$true)]
    [string]$Version
)

Write-Host "Building ESP32 Clock Radio v$Version..." -ForegroundColor Green

# Build the project
Write-Host "Building firmware..." -ForegroundColor Yellow
& "C:\Users\berto\.platformio\penv\Scripts\platformio.exe" run

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}

# Create release directory
$releaseDir = "release\v$Version"
if (Test-Path $releaseDir) {
    Remove-Item $releaseDir -Recurse -Force
}
New-Item -ItemType Directory -Path $releaseDir -Force | Out-Null

# Copy binary files
Write-Host "Packaging release files..." -ForegroundColor Yellow
Copy-Item ".pio\build\esp32-s3-devkitc-1\firmware.bin" "$releaseDir\clock-radio-firmware-v$Version.bin"
Copy-Item ".pio\build\esp32-s3-devkitc-1\bootloader.bin" "$releaseDir\bootloader.bin"
Copy-Item ".pio\build\esp32-s3-devkitc-1\partitions.bin" "$releaseDir\partitions.bin"

# Copy documentation
Write-Host "Including documentation..." -ForegroundColor Yellow
Copy-Item "USER_MANUAL.md" "$releaseDir\USER_MANUAL.md"
Copy-Item "README.md" "$releaseDir\PROJECT_README.md"

# Create flash script for Windows
$flashScript = @"
@echo off
echo ESP32 Clock Radio v$Version - Flash Script
echo.
echo Make sure your ESP32 is connected via USB and in download mode
echo Press any key to start flashing...
pause >nul

echo.
echo Flashing firmware...
esptool.py --chip esp32s3 --port COM3 --baud 921600 write_flash -z 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 clock-radio-firmware-v$Version.bin

echo.
echo Flashing complete!
pause
"@

Set-Content -Path "$releaseDir\flash_firmware.bat" -Value $flashScript

# Create flash script for Linux/Mac
$flashScriptUnix = @"
#!/bin/bash
echo "ESP32 Clock Radio v$Version - Flash Script"
echo ""
echo "Make sure your ESP32 is connected via USB and in download mode"
echo "Press Enter to start flashing..."
read

echo ""
echo "Flashing firmware..."
esptool.py --chip esp32s3 --port /dev/ttyUSB0 --baud 921600 write_flash -z 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 clock-radio-firmware-v$Version.bin

echo ""
echo "Flashing complete!"
"@

Set-Content -Path "$releaseDir\flash_firmware.sh" -Value $flashScriptUnix

# Create README for the release
$releaseReadme = @"
# ESP32 Clock Radio v$Version

## Quick Flash Instructions

### Windows Users:
1. Install ESP32 tools: ``pip install esptool``
2. Connect your ESP32-S3 via USB
3. Put ESP32 in download mode (hold BOOT button while pressing RESET)
4. Run ``flash_firmware.bat``
5. Adjust COM port in the batch file if needed (default: COM3)

### Linux/Mac Users:
1. Install ESP32 tools: ``pip install esptool``
2. Connect your ESP32-S3 via USB
3. Put ESP32 in download mode (hold BOOT button while pressing RESET)
4. Run ``chmod +x flash_firmware.sh && ./flash_firmware.sh``
5. Adjust port in the script if needed (default: /dev/ttyUSB0)

### Manual Flash Command:
````
esptool.py --chip esp32s3 --port [YOUR_PORT] --baud 921600 write_flash -z 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 clock-radio-firmware-v$Version.bin
````

## Files Included:
- ``clock-radio-firmware-v$Version.bin`` - Main firmware
- ``bootloader.bin`` - ESP32 bootloader
- ``partitions.bin`` - Partition table
- ``flash_firmware.bat`` - Windows flash script
- ``flash_firmware.sh`` - Linux/Mac flash script
- ``USER_MANUAL.md`` - Complete user manual and setup guide
- ``PROJECT_README.md`` - Project overview and development info

## Documentation:
- **USER_MANUAL.md**: Complete usage instructions, menu system, alarms, OTA updates
- **PROJECT_README.md**: Hardware requirements, development setup, contribution guide

## First Time Setup:
After flashing, the device will start WiFi configuration mode. Follow the on-screen instructions to connect to your WiFi network.

For detailed usage instructions, alarms, OTA updates, and troubleshooting, see USER_MANUAL.md.
For hardware setup and development info, see PROJECT_README.md.
"@

Set-Content -Path "$releaseDir\README.md" -Value $releaseReadme

# Create ZIP archive
Write-Host "Creating ZIP archive..." -ForegroundColor Yellow
$zipPath = "clock-radio-v$Version.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath -Force
}

Compress-Archive -Path "$releaseDir\*" -DestinationPath $zipPath

Write-Host "Release package created successfully!" -ForegroundColor Green
Write-Host "Files:"
Write-Host "  - Release folder: $releaseDir"
Write-Host "  - ZIP archive: $zipPath"
Write-Host ""
Write-Host "Upload the ZIP file to your GitHub release." -ForegroundColor Cyan
