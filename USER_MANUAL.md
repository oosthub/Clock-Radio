# OOSIE Internet Radio - User Manual

## Table of Contents
1. [Introduction](#introduction)
2. [Hardware Overview](#hardware-overview)
3. [First Startup & Initial Configuration](#first-startup--initial-configuration)
4. [Basic Operation](#basic-operation)
5. [Menu System](#menu-system)
6. [Web Configuration Interface](#web-configuration-interface)
7. [Sleep Timer](#sleep-timer)
8. [Weather Information](#weather-information)
9. [Troubleshooting](#troubleshooting)
10. [Technical Specifications](#technical-specifications)

---

## 1. Introduction

Welcome to your OOSIE Internet Radio! This device streams internet radio stations and displays the time, weather information, and provides various configuration options through both physical controls and a web interface.

### Key Features:
- **Internet Radio Streaming**: Access thousands of online radio stations
- **LCD Display**: 16x2 character display showing time, weather, and menu information
- **Rotary Encoder Control**: Navigate menus and adjust settings
- **Sleep Timer**: Automatic shut-off functionality
- **Weather Integration**: Real-time weather display with OpenWeatherMap API
- **Web Interface**: Configure streams and settings through your browser
- **Auto Backlight**: Intelligent display backlight management

---

## 2. Hardware Overview

### Physical Components:
- **LCD Display**: 16x2 character display with backlight
- **Rotary Encoder**: Rotate to navigate, press to select
- **ESP32-S3**: Main processing unit with WiFi capability
- **Audio Output**: I2S digital audio output
- **Power Supply**: USB-C or external power adapter

### Controls:
- **Rotate Encoder**: Navigate between menu items or adjust volume
- **Short Press** (< 1 second): Enter menu or confirm selection
- **Long Press** (3+ seconds): Power radio ON/OFF

---

## 3. First Startup & Initial Configuration

### 3.1 Power On
Connect power to your radio. You'll see the startup sequence:

```
┌────────────────┐
│OOSIE Radio     │
│Starting...     │
└────────────────┘
```

### 3.2 WiFi Configuration
If no WiFi credentials are stored, the radio will attempt to connect using built-in defaults. If this fails, it will enter WiFi configuration mode:

```
┌────────────────┐
│WiFi Setup      │
│SSID: [____]    │
└────────────────┘
```

**To configure WiFi:**
1. Rotate encoder to select characters
2. Short press to confirm each character
3. Use the backspace symbol (←) to delete characters
4. After entering the last character, hold button for 3 seconds to confirm
5. Repeat process for password

### 3.3 Time Setup
Once connected to WiFi, the radio automatically synchronizes time using NTP servers. No manual configuration required.

### 3.4 Ready to Use
After successful setup, you'll see the main display:

```
┌────────────────┐
│12:34      22°C☀│
│Jacaranda FM    │
└────────────────┘
```

---

## 4. Basic Operation

### 4.1 Main Display Layout

**Top Line**: Time + Sleep Indicator (if Set) + Weather
- Time format: HH:MM (24-hour)
- Weather: Temperature + weather icon
- Sleep indicator: "Zz" appears when sleep timer is active

**Bottom Line**: Radio Status
- **Radio ON + Streaming**: Current station name
- **Radio ON + Not Streaming**: Empty
- **Radio OFF**: "Radio OFF"

### 4.2 Power Control
- **Turn ON**: Long press (3+ seconds) when radio is OFF
- **Turn OFF**: Long press (3+ seconds) when radio is ON
- **Status**: LED or display indicates current state

### 4.3 Volume Control
- **Adjust Volume**: Rotate encoder when NOT in menu mode
- **Volume Range**: 0-80
- **Visual Feedback**: Brief volume display overlay

### 4.4 Backlight Management
The display backlight has two modes:
- **Always On**: Backlight stays on continuously
- **Auto Off**: Backlight turns off after 5 seconds of inactivity

**Wake Up Display** (Auto Off mode):
1. First button press: Wakes display for 5 seconds
2. Second press within 5 seconds: Enters menu system

---

## 5. Menu System

Access the menu system with a short press of the encoder button.

### 5.1 Menu Navigation

```
┌────────────────┐
│MENU: Sleep     │
│                │  ← Blank option for navigation
└────────────────┘
```

**Navigation**:
- **Rotate Encoder**: Move between menu items
- **Short Press on Blank**: Move to next main menu
- **Short Press on Option**: Sets the timer to the selected value

### 5.2 Sleep Timer Menu

```
┌────────────────┐
│MENU: Sleep     │
│[Blank]         │  → Navigate to next menu
└────────────────┘

┌────────────────┐
│MENU: Sleep     │
│OFF             │  → Turn off active timer
└────────────────┘

┌────────────────┐
│MENU: Sleep     │
│15 minutes      │  → Set 15-minute timer
└────────────────┘

┌────────────────┐
│MENU: Sleep     │
│30 minutes      │  → Set 30-minute timer
└────────────────┘

┌────────────────┐
│MENU: Sleep     │
│60 minutes      │  → Set 60-minute timer
└────────────────┘

┌────────────────┐
│MENU: Sleep     │
│90 minutes      │  → Set 90-minute timer
└────────────────┘

┌────────────────┐
│MENU: Sleep     │
│5 minutes       │  → Set 5-minute timer (testing)
└────────────────┘
```

**Sleep Timer Operation**:
- Select any time option to start countdown
- Radio automatically turns OFF when timer expires
- "Zz" indicator appears on main display when active
```
    ┌────────────────┐
    │12:34 Zz   22°C☀│
    │Jacaranda FM    │
    └────────────────┘
```
- Manual radio power-off cancels active timer


### 5.3 Station Selection Menu

```
┌────────────────┐
│MENU: Station   │
│Jacaranda FM    │
└────────────────┘
```

**Station Menu**:
- Rotate to browse available stations
- Short press to select and start playing
- Currently playing station shows in bottom line of main display

### 5.4 Backlight Menu

```
┌────────────────┐
│MENU: Backlight │
│Mode: AUTO OFF  │
└────────────────┘

┌────────────────┐
│MENU: Backlight │
│Mode: ALWAYS ON │
└────────────────┘
```

**Backlight Options**:
- **AUTO OFF**: Backlight turns off after 30 seconds
- **ALWAYS ON**: Backlight remains on continuously

### 5.5 WiFi Information Menu

```
┌────────────────┐
│MENU: WiFi      │
│192.168.1.100   │  → IP Address
└────────────────┘

┌────────────────┐
│MENU: WiFi      │
│SSID: MyNetwork │  → Network name
└────────────────┘

┌────────────────┐
│MENU: WiFi      │
│PASS: *****     │  → Password (masked)
└────────────────┘

┌────────────────┐
│MENU: WiFi      │
│Reset WiFi      │  → Reset WiFi settings
└────────────────┘
```

**WiFi Reset Process**:
1. Select "Reset WiFi"
2. Confirmation screen appears:
```
┌────────────────┐
│Reset WiFi?     │
│> YES    NO     │  → Use encoder to choose
└────────────────┘
```

### 5.6 Weather Menu

```
┌────────────────┐
│MENU: Weather   │
│TEMP: 22C       │  → Current temperature
└────────────────┘

┌────────────────┐
│MENU: Weather   │
│HUM: 65%        │  → Humidity percentage
└────────────────┘

┌────────────────┐
│MENU: Weather   │
│DESC: clear sky │  → Weather description
└────────────────┘

┌────────────────┐
│MENU: Weather   │
│API: SET        │  → API key status
└────────────────┘

┌────────────────┐
│MENU: Weather   │
│Update Weather  │  → Manually force weather refresh
└────────────────┘
```

**Weather Update System**:
- **Automatic Updates**: Weather refreshes every 30 minutes
- **Manual Update**: Force immediate refresh when needed
- **Use Cases**: 
  - Check current conditions immediately after setup
  - Refresh after network connectivity issues
  - Update after changing location/traveling

---

## 6. Web Configuration Interface

Access the web interface by connecting to the same WiFi network and opening `http://[radio-ip-address]` in your browser.

### 6.1 Finding the IP Address
The radio's IP address is displayed in the WiFi menu

### 6.2 Web Interface Layout

```
🎵 OOSIE Radio
Internet Radio - Stream Manager

┌─ Stream Management ────────────────────────────────────────┐
│                                                            │
│ Name (max 16 chars)    │ URL                    │ Actions  │
│ ─────────────────────── ─────────────────────── ───────── │
│ Jacaranda FM           │ https://edge.iono.fm/  │ Delete   │
│ Pretoria FM            │ https://edge.iono.fm/  │ Delete   │
│ [New Stream Name]      │ [Stream URL]           │ Add      │
│                                                            │
│                    [💾 Save All Changes]                   │
└────────────────────────────────────────────────────────────┘

┌─ Weather Configuration ────────────────────────────────────┐
│                                                            │
│ Weather API Key: [abc***********xyz]                      │
│                                                            │
│               [💾 Save Weather Settings]                   │
└────────────────────────────────────────────────────────────┘
```

### 6.3 Managing Radio Stations

**Adding Stations**:
1. Enter station name (maximum 16 characters)
2. Enter stream URL (must start with http:// or https://)
3. Click "Add Stream"
4. Click "Save All Changes" to apply

**Editing Stations**:
1. Modify name or URL directly in the table
2. Click "Save All Changes" to apply

**Deleting Stations**:
1. Click "Delete" button next to unwanted station
2. Confirm deletion
3. Click "Save All Changes" to apply

**Important Notes**:
- Changes are not applied until "Save All Changes" is clicked
- Radio will restart automatically after saving streams
- At least one stream must remain in the list

### 6.4 Weather Configuration

**Setting Up Weather**:
1. Visit [OpenWeatherMap.org](https://openweathermap.org/api)
2. Create free account and obtain API key
3. Enter API key in the web interface
4. Click "Save Weather Settings"

**API Key Security**:
- After saving, API key is masked: `abc***********xyz`
- First 3 and last 3 characters remain visible
- To change: clear field and enter new key

---

## 7. Sleep Timer

### 7.1 Setting Sleep Timer

**Through Menu**:
1. Enter menu system (short press)
2. Navigate to Sleep menu
3. Select desired duration: 15, 30, 60, 90, or 5 minutes
4. Timer starts immediately and menu exits

**Visual Indicators**:
- "Zz" appears between time and weather on main display
- Sleep timer resets to "blank" option when re-entering menu

### 7.2 Sleep Timer Behavior

**Normal Operation**:
- Radio continues playing during countdown
- Automatic shutdown when timer reaches zero
- Display shows "Radio OFF" after timeout

**Manual Override**:
- Long press (3+ seconds) turns off radio and cancels timer
- Selecting "OFF" in sleep menu cancels active timer
- Power loss or restart cancels timer

**Timer Options**:
- **5 minutes**: Quick testing option
- **15 minutes**: Short nap
- **30 minutes**: Standard sleep duration
- **60 minutes**: Extended listening
- **90 minutes**: Full sleep cycle

---

## 8. Weather Information

### 8.1 Weather Display

**Main Screen Weather**:
- Shows current temperature and weather icon
- Updates automatically every 30 minutes
- Location auto-detected via IP geolocation

**Weather Icons**:
- ☀ Sunny/Clear
- ☁ Cloudy
- 🌧 Rainy
- ❄ Snow
- 🌫 Fog/Mist

### 8.2 Weather Menu Details

Access detailed weather information through the Weather menu:

**Temperature**: Current temperature in Celsius
**Humidity**: Relative humidity percentage
**Description**: Weather condition in text
**API Status**: Shows if API key is configured
**Manual Update**: Force immediate weather refresh

### 8.3 Weather Configuration

**Requirements**:
- Active internet connection
- Valid OpenWeatherMap API key
- GPS/location services (automatic)

**API Key Setup**:
1. Register at openweathermap.org
2. Generate free API key
3. Enter key via web interface
4. Weather updates begin automatically

---

## 9. Troubleshooting

### 9.1 Common Issues

**No WiFi Connection**:
- Check WiFi credentials in WiFi menu
- Ensure network is 2.4GHz (5GHz not supported)
- Try WiFi reset if connection fails
- Check router settings (WPA/WPA2 supported)

**No Audio Output**:
- Verify radio is powered ON (check display)
- Check volume level (rotate encoder outside menu)
- Ensure stream URL is valid and accessible
- Try different radio station

**Display Issues**:
- **Blank Display**: Check power connection and backlight settings
- **Garbled Text**: Reset radio by power cycling
- **No Response**: Check encoder connections

**Weather Not Updating**:
- Verify API key is configured correctly
- Check internet connection
- Wait up to 10 minutes for automatic update
- Use manual update in Weather menu

### 9.2 Reset Procedures

**WiFi Reset**:
1. Access WiFi menu
2. Select "Reset WiFi"
3. Confirm selection
4. Radio will restart and enter WiFi setup mode

**Factory Reset**:
1. Power off radio
2. Hold encoder button while powering on
3. Release after 10 seconds
4. Radio returns to factory defaults

**Soft Reset**:
- Power cycle the radio (unplug and reconnect)
- All settings preserved, temporary issues cleared

### 9.3 Error Messages

**"No streams available"**:
- Configure streams via web interface
- Check internet connectivity
- Verify stream URLs are accessible

**"WiFi connection failed"**:
- Check network credentials
- Ensure 2.4GHz network availability
- Try moving closer to router

**Weather API errors**:
- Verify API key validity
- Check API usage limits
- Ensure internet connectivity

---

## 10. Technical Specifications

### 10.1 Hardware Specifications

**Processor**: ESP32-S3 WROOM-1-N16R8
- **CPU**: Dual-core Tensilica LX7 @ 240MHz
- **RAM**: 512KB SRAM
- **Flash**: 16MB
- **PSRAM**: 8MB

**Display**: 16x2 Character LCD with I2C Interface
- **Resolution**: 16 characters × 2 lines
- **Backlight**: LED with auto-off capability
- **Interface**: I2C (SDA/SCL)

**Audio**: I2S Digital Audio Output
- **Format**: I2S digital audio
- **Sample Rates**: 8kHz to 96kHz
- **Bit Depth**: 16/24-bit

**Input**: Rotary Encoder
- **Type**: Incremental rotary encoder with push button
- **Resolution**: 20 pulses per revolution
- **Debouncing**: Hardware and software debouncing

**Connectivity**: WiFi 802.11 b/g/n
- **Frequency**: 2.4GHz
- **Security**: WPA/WPA2/WPA3
- **Range**: Standard WiFi range

### 10.2 Software Specifications

**Operating System**: FreeRTOS
**Development Framework**: Arduino/ESP-IDF
**Audio Library**: ESP32-audioI2S
**Web Server**: ESPAsyncWebServer
**JSON Processing**: ArduinoJson
**File System**: SPIFFS

**Supported Audio Formats**:
- MP3
- AAC
- FLAC
- WAV
- M4A
- OGG

**Streaming Protocols**:
- HTTP/HTTPS
- Icecast
- Shoutcast

### 10.3 Power Requirements

**Input Voltage**: 5V DC
**Current Consumption**:
- **Idle**: 150mA
- **Playing**: 200mA
- **Peak**: 300mA

**Power Connector**: USB-C or 2.1mm DC jack

### 10.4 Environmental Specifications

**Operating Temperature**: 0°C to 50°C
**Storage Temperature**: -20°C to 70°C
**Humidity**: 10% to 80% non-condensing
**Dimensions**: 120mm × 80mm × 40mm (approximate)
**Weight**: 200g (approximate)

---

## Appendix A: Default Radio Stations

The radio comes pre-configured with these South African radio stations:

1. **Jacaranda FM** - Contemporary music and talk
2. **Pretoria FM** - Local community radio
3. **Lekker FM** - Afrikaans music and entertainment
4. **Groot FM** - Traditional Afrikaans content
5. **RSG** - Talk radio and news

Additional stations can be added via the web interface.

---

## Appendix B: API Key Sources

**Weather Service**: OpenWeatherMap
- Website: https://openweathermap.org/api
- Free Tier: 1000 calls/day
- Registration: Required
- Cost: Free with limits, paid plans available

---

## Appendix C: Supported Stream Formats

**Direct Stream URLs**:
- `.mp3` - MP3 audio streams
- `.aac` - AAC audio streams
- `.m3u8` - HLS playlist format
- `.pls` - Playlist format
- `.m3u` - Extended M3U playlist

**Streaming Services**:
- Icecast servers
- Shoutcast servers
- HTTP live streaming
- Direct HTTP audio streams

---

## Support and Contact

For technical support or questions about your OOSIE Internet Radio:

- **Documentation**: This user manual
- **Web Interface**: http://[radio-ip]/
- **Serial Console**: 115200 baud for debugging
- **Reset Options**: WiFi reset, soft reset, factory reset

---

*OOSIE Internet Radio User Manual v1.0*
*Last Updated: August 2025*

