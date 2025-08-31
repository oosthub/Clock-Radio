# ESP32 Clock Radio

An internet radio with alarm functionality built for the ESP32-S3, featuring a 16x2 LCD display, rotary encoder control, and comprehensive web configuration.

## Features

- **Internet Radio Streaming**: Access thousands of online radio stations
- **5-Alarm System**: Multiple daily, weekday, weekend, and one-time alarms with fade-in
- **LCD Display**: 16x2 character display showing time, weather, alarms, and menu information
- **Rotary Encoder Control**: Navigate menus and adjust settings
- **Sleep Timer**: Automatic shut-off functionality (15, 30, 60, 90 minutes)
- **Weather Integration**: Real-time weather display with OpenWeatherMap API
- **Web Interface**: Configure streams and settings through your browser
- **Auto Backlight**: Intelligent display backlight management
- **Now Playing Info**: Displays track information with intelligent scrolling

## Hardware Requirements

- **ESP32-S3 DevKit** (with 8MB+ Flash, PSRAM recommended)
- **16x2 I2C LCD Display** 
- **Rotary Encoder** with push button
- **I2S Audio DAC** (e.g., PCM5102, MAX98357A)
- **Speaker** or headphone output

### Pin Configuration

```cpp
// I2S Audio
#define I2S_DOUT  2   // Data Out
#define I2S_BCLK  3   // Bit Clock
#define I2S_LRC   4   // Left/Right Clock

// Rotary Encoder
#define ENCODER_A    10
#define ENCODER_B    11  
#define ENCODER_BTN  12

// I2C Display
#define SDA_PIN  8
#define SCL_PIN  9
```

## Quick Start

### Option 1: Flash Pre-built Firmware (Recommended)

1. **Download Latest Release**
   - Go to [Releases](../../releases)
   - Download the latest `clock-radio-vX.X.X.zip`

2. **Install ESP32 Tools**
   ```bash
   pip install esptool
   ```

3. **Flash Firmware**
   - Connect ESP32-S3 via USB
   - Put device in download mode (hold BOOT button while pressing RESET)
   - Extract ZIP and run the flash script:
     - **Windows**: `flash_firmware.bat`
     - **Linux/Mac**: `./flash_firmware.sh`

### Option 2: Build from Source

1. **Install PlatformIO**
   ```bash
   pip install platformio
   ```

2. **Clone and Build**
   ```bash
   git clone https://github.com/oosthub/Clock-Radio.git
   cd Clock-Radio
   pio run --target upload
   ```

## First Time Setup

1. **WiFi Configuration**
   - On first boot, device enters WiFi setup mode
   - Use rotary encoder to enter network credentials
   - Rotate to select characters, press to confirm
   - Hold button for 3 seconds to complete SSID/password entry

2. **Web Configuration**
   - Find device IP in WiFi menu or serial output
   - Open `http://[device-ip]` in browser
   - Add radio stations and configure weather API

3. **Weather Setup** (Optional)
   - Get free API key from [OpenWeatherMap](https://openweathermap.org/api)
   - Enter API key in web interface

## Usage

### Basic Controls
- **Rotate Encoder**: Adjust volume (outside menu) or navigate (in menu)
- **Short Press**: Enter menu or confirm selection
- **Long Press** (3+ sec): Power radio ON/OFF

### Menu System
Access with short press, navigate with encoder:
- **Sleep Timer**: 15/30/60/90 minute auto-off
- **Alarms**: Configure 5 independent alarms
- **Stations**: Select radio station
- **Backlight**: Always-on or auto-off mode
- **WiFi**: View connection info, reset credentials
- **Weather**: View conditions, manual update

### Alarm Features
- **5 Independent Alarms**: Each with separate settings
- **Schedule Types**: Daily, weekdays, weekends, or once
- **Fade-in Audio**: Gentle 60-second volume increase  
- **Snooze**: 10-minute snooze with cancellation
- **Station Selection**: Different station per alarm

## Web Interface

Configure your radio through the web browser:

- **Stream Management**: Add/edit/delete radio stations
- **Weather API**: Configure OpenWeatherMap integration
- **Real-time Updates**: Changes applied immediately

Access at: `http://[device-ip-address]`

## Development

### Building Releases

Use the included PowerShell script to create release packages:

```powershell
.\create_release.ps1 -Version "1.0.0"
```

This generates:
- Compiled firmware binaries
- Flash scripts for Windows/Linux/Mac
- Release documentation
- ZIP package ready for GitHub release

### Project Structure

```
Clock-Radio/
├── src/                 # Source code
│   ├── main.cpp        # Main application
│   ├── display.cpp     # LCD display handling
│   ├── menu.cpp        # Menu system
│   ├── alarm.cpp       # Alarm functionality
│   ├── webserver.cpp   # Web interface
│   └── ...
├── include/            # Header files
├── platformio.ini      # PlatformIO configuration
└── USER_MANUAL.md      # Detailed user guide
```

## Documentation

- **[User Manual](USER_MANUAL.md)**: Complete usage guide
- **[Hardware Setup](docs/hardware.md)**: Wiring diagrams (coming soon)
- **[API Reference](docs/api.md)**: Web interface API (coming soon)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is open source. See LICENSE file for details.

## Support

- **Issues**: Report bugs or request features via GitHub Issues
- **Documentation**: Check USER_MANUAL.md for detailed instructions
- **Serial Debug**: Connect at 115200 baud for debugging output

---

**Version**: v1.0.0  
**Compatible Hardware**: ESP32-S3 (8MB+ Flash recommended)  
**Dependencies**: PlatformIO, ESP32 Arduino Core
