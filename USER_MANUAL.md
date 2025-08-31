# OOSIE Internet Radio - User Manual

## Table of Contents
1. [Introduction](#introduction)
2. [Hardware Overview](#hardware-overview)
3. [First Startup & Initial Configuration](#first-startup--initial-configuration)
4. [Basic Operation](#basic-operation)
5. [Menu System](#menu-system)
6. [Alarm System](#alarm-system)
7. [Web Configuration Interface](#web-configuration-interface)
   - 7.1 [WiFi Hotspot Setup](#71-wifi-hotspot-setup)
   - 7.2 [Finding the IP Address](#72-finding-the-ip-address)
   - 7.3 [Web Interface Layout](#73-web-interface-layout)
   - 7.4 [Tab Navigation](#74-tab-navigation)
   - 7.5 [Managing Radio Stations](#75-managing-radio-stations-stream-manager-tab)
   - 7.6 [Weather Configuration](#76-weather-configuration-weather-settings-tab)
   - 7.7 [WiFi Settings](#77-wifi-settings-wifi-settings-tab)
   - 7.8 [User Manual](#78-user-manual-user-manual-tab)
8. [Sleep Timer](#sleep-timer)
9. [Weather Information](#weather-information)
10. [Over-the-Air (OTA) Updates](#over-the-air-ota-updates)
11. [Troubleshooting](#troubleshooting)
12. [Technical Specifications](#technical-specifications)

---

## 1. Introduction

Welcome to your OOSIE Internet Radio! This device streams internet radio stations and displays the time, weather information, and provides various configuration options through both physical controls and a web interface.

### Key Features:
- **Internet Radio Streaming**: Access thousands of online radio stations
- **5-Alarm System**: Multiple daily, weekday, weekend, and one-time alarms with fade-in
- **Now Playing Info**: Displays track information with intelligent scrolling for long titles
- **LCD Display**: 16x2 character display showing time, weather, alarms, and menu information
- **Rotary Encoder Control**: Navigate menus and adjust settings
- **Sleep Timer**: Automatic shut-off functionality
- **Weather Integration**: Real-time weather display with OpenWeatherMap API
- **Web Interface**: Configure streams and settings through your browser
- **Auto Backlight**: Intelligent display backlight management
- **OTA Firmware Updates**: Over-the-air updates directly from GitHub releases
- **Visual Indicators**: Clock symbol for active alarms, sleep timer indicator

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
- **Long Press** (3+ seconds): Power radio ON/OFF, stop active alarm, or cancel snooze

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

When no WiFi credentials are stored or if connection fails, the radio will present two configuration options:

```
┌────────────────┐
│WiFi Setup Mode:│
│> Hotspot Mode  │
└────────────────┘
```

**Option 1: Hotspot Mode (Recommended)**
1. Select "Hotspot Mode" and press the encoder button
2. The radio creates a WiFi hotspot named "OOSIE-Radio" (no password required)
3. Instructions cycle on the LCD display every 4 seconds:
   - "Connect to WiFi: OOSIE-Radio"
   - "Open browser: 192.168.4.1" 
   - "Configure WiFi via web page"
4. Connect your phone/computer to "OOSIE-Radio" network
5. Open browser and navigate to `192.168.4.1`
6. Click the "📶 WiFi Settings" tab
7. Enter your WiFi network name and password
8. Click "💾 Save & Restart"
9. Radio automatically restarts and connects to your network

**Option 2: Manual Configuration**
1. Select "Manual Config" and press the encoder button
2. Use encoder to select characters for WiFi network name:

```
┌────────────────┐
│WiFi Setup      │
│SSID: [____]    │
└────────────────┘
```

3. Rotate encoder to select characters
4. Short press to confirm each character
5. Use the backspace symbol (←) to delete characters
6. After entering the last character, hold button for 3 seconds to confirm
7. Repeat process for password

**Hotspot Mode Advantages:**
- Easier to use with phone/tablet
- Visual web interface with validation
- Faster input for complex passwords
- Works with any device with a web browser
- No need to navigate character-by-character

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

**Top Line**: Time + Alarm Indicator + Sleep Indicator + Weather
- Time format: HH:MM (24-hour)
- Alarm indicator: Clock symbol (⚐) appears when any alarm is enabled
- Sleep indicator: "Z" appears when sleep timer is active
- Weather: Temperature + weather icon

**Bottom Line**: Radio Status & Now Playing Info
- **Radio ON + Streaming**: Alternates between station name and track information
- **Radio ON + Not Streaming**: Empty
- **Radio OFF**: "Radio OFF"
- **Alarm Active**: Shows alarm information and controls

### 4.3 Now Playing Information

When a radio station provides track metadata (artist and song information), the bottom line will alternate between the station name and the current track information:

**Station Name Display** (10 seconds):
```
┌────────────────┐
│12:34      22°C☀│
│  Jacaranda FM  │  ← Station name (centered)
└────────────────┘
```

**Track Info Display** (varies):
```
┌────────────────┐
│12:34      22°C☀│
│The Beatles - He│  ← Track info (scrolling if needed)
└────────────────┘
```

**Now Playing Behavior**:
- **Short Track Names** (≤16 characters): Display for 10 seconds, then switch to station name
- **Long Track Names** (>16 characters): Scroll completely through the text before switching back
- **Scrolling Speed**: Updates every 300ms for smooth scrolling
- **Maximum Display Time**: 20 seconds for very long track names
- **No Track Info**: Shows only station name when metadata unavailable

**Track Information Examples**:
- `"Artist - Song Title"`
- `"BBC News at 6pm"`
- `"The Beatles - Hey Jude (Remastered 2009)"`

### 4.4 Alarm System

The radio features a comprehensive 5-alarm system with various scheduling options:

**Main Display with Active Alarm**:
```
┌────────────────┐
│12:34 ⚐ Z 22°C☀│  ← Clock symbol shows enabled alarm
│Jacaranda FM    │
└────────────────┘
```

**Alarm Features**:
- **5 Independent Alarms**: Each can be configured separately
- **Multiple Schedules**: Daily, Weekdays, Weekends, or Once
- **Fade-in Audio**: Gentle volume increase over 60 seconds
- **Snooze Function**: 10-minute snooze with long-press to cancel
- **Station Selection**: Each alarm can use a different radio station
- **Visual Feedback**: Confirmation messages for all alarm actions

**Alarm Operation**:
- **When Alarm Triggers**: Audio fades in gradually from quiet to full volume
- **Stop Alarm**: Long press (3+ seconds) during alarm
- **Snooze Alarm**: Short press during alarm (10-minute snooze)
- **Cancel Snooze**: Long press during snooze period

**Alarm Display During Active Alarm**:
```
┌────────────────┐
│ALARM 1  STOP▲  │  ← Shows which alarm, stop option
│Jacaranda FM    │
└────────────────┘
```

**Snooze Display**:
```
┌────────────────┐
│12:34 ⚐   22°C☀│  ← Time + indicators + weather
│SNOOZE    09:45 │  ← Snooze with remaining time (MM:SS)
└────────────────┘
```

### 4.5 Power Control
- **Turn ON**: Long press (3+ seconds) when radio is OFF
- **Turn OFF**: Long press (3+ seconds) when radio is ON
- **During Alarm**: Long press stops alarm and keeps radio off
- **During Snooze**: Long press cancels snooze and turns radio on
- **Status**: LED or display indicates current state

### 4.6 Volume Control
- **Adjust Volume**: Rotate encoder when NOT in menu mode
- **Volume Range**: 0-80
- **Visual Feedback**: Brief volume display overlay
- **Alarm Volume**: Independent from radio volume, preserves user settings

### 4.7 Backlight Management
The display backlight has two modes:
- **Always On**: Backlight stays on continuously
- **Auto Off**: Backlight turns off after 5 seconds of inactivity

**Wake Up Display** (Auto Off mode):
1. First button press: Wakes display for 5 seconds
2. Second press within 5 seconds: Enters menu system

**Automatic Wake**: Display automatically wakes for alarm messages and confirmations

---

## 5. Menu System

Access the menu system with a short press of the encoder button. The menu system features a 6-second timeout.

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
- **Menu Timeout**: Automatically exits after 6 seconds of inactivity

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
- "Z" indicator appears on main display when active
```
    ┌────────────────┐
    │12:34 ⚐ Z 22°C☀│
    │Jacaranda FM    │
    └────────────────┘
```
- Manual radio power-off cancels active timer

**Timer Options**:
- **5 minutes**: Quick testing option
- **15 minutes**: Short nap
- **30 minutes**: Standard sleep duration
- **60 minutes**: Extended listening
- **90 minutes**: Full sleep cycle

### 5.3 Alarm Menu System

The alarm system features a hierarchical menu with two levels of navigation:

**Level 1: Alarm Slot Selection**
```
┌────────────────┐
│MENU: Alarms    │
│                │  ← Blank option (navigate to next menu)
└────────────────┘

┌────────────────┐
│MENU: Alarms    │
│ALARM 1         │  ← Select alarm slot 1
└────────────────┘

┌────────────────┐
│MENU: Alarms    │
│ALARM 2         │  ← Select alarm slot 2
└────────────────┘
```

**Level 2: Alarm Configuration** (after selecting an alarm slot)
```
┌────────────────┐
│ALARM 1         │
│< BACK          │  ← Return to alarm slot selection
└────────────────┘

┌────────────────┐
│ALARM 1         │
│ENABLED: YES    │  ← Toggle alarm on/off
└────────────────┘

┌────────────────┐
│ALARM 1         │
│TIME: 07:30     │  ← Set alarm time (with blinking)
└────────────────┘

┌────────────────┐
│ALARM 1         │
│SCHEDULE: DAILY │  ← Set schedule type
└────────────────┘

┌────────────────┐
│ALARM 1         │
│STATION: FM1    │  ← Select radio station
└────────────────┘
```

**Alarm Menu Navigation**:
1. **Main Menu**: Rotate to cycle through alarm slots (1-5) and blank option
2. **Blank Option**: Click to move to next main menu section
3. **Alarm Slot**: Click to enter alarm configuration for that slot
4. **Configuration**: Rotate to cycle through settings, click to edit values
5. **Back Option**: Click to return to alarm slot selection

**Time Setting with Visual Feedback**:
When setting alarm time, hours and minutes blink to indicate editing mode:
```
┌────────────────┐
│ALARM 1         │
│TIME: 07:30     │  ← Hours blink when editing hours
└────────────────┘
```

**Schedule Options**:
- **DAILY**: Alarm triggers every day
- **WEEKDAYS**: Monday through Friday only
- **WEEKENDS**: Saturday and Sunday only
- **ONCE**: Single trigger, then automatically disabled

**Station Selection**:
- Rotate through all available radio stations
- Each alarm can use a different station
- Station names display as configured in web interface

### 5.4 Station Selection Menu

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

### 5.5 Backlight Menu

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

### 5.6 WiFi Information Menu

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

### 5.7 Weather Menu

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

### 5.8 System Menu

The System menu provides access to firmware information and over-the-air (OTA) updates:

```
┌────────────────┐
│MENU: System    │
│Firm: 1.0.2     │  → Current firmware version (info only)
└────────────────┘

┌────────────────┐
│MENU: System    │
│Update          │  → Check for and install firmware updates
└────────────────┘
```

**System Menu Options**:

#### Firmware Version Display
- **Purpose**: Shows current firmware version for reference
- **Action**: Informational only - pressing moves to next main menu
- **Format**: "Firm: X.X.X" (e.g., "Firm: 1.0.2")

#### OTA Firmware Updates
- **Purpose**: Download and install firmware updates from GitHub
- **Requirements**: Active WiFi connection
- **Process**: Automatic detection, download, and installation

**Update Process Flow**:

1. **Check for Updates**:
```
┌────────────────┐
│Checking for    │
│updates...      │
└────────────────┘
```

2. **No Update Available**:
```
┌────────────────┐
│Firmware        │
│Up to Date      │  ← Displayed for 2 seconds
└────────────────┘
```

3. **Update Available**:
```
┌────────────────┐
│Update found!   │
│Installing...   │
└────────────────┘
```

4. **Download Progress**:
```
┌────────────────┐
│Updating...     │
│35%             │  ← Real-time percentage
└────────────────┘
```

5. **Installation Complete**:
```
┌────────────────┐
│Update Complete │
│Rebooting...    │  ← Device restarts automatically
└────────────────┘
```

**Update Features**:
- **Automatic Version Checking**: Compares current vs latest GitHub release
- **Progress Display**: Real-time download and installation progress
- **Error Handling**: Clear messages for network or installation issues
- **Safety**: Update only proceeds if download is verified
- **Automatic Reboot**: Device restarts with new firmware after successful update

**Error Messages**:
- **"Update Error / Check WiFi"**: Network connectivity problems
- **"Update Failed / Try again later"**: Download or installation error
- **"Up to Date"**: Current firmware is already the latest version

**Technical Details**:
- Updates downloaded from: `https://github.com/oosthub/Clock-Radio/releases`
- Requires internet access to api.github.com
- Supports semantic versioning (v1.0.1, v1.0.2, etc.)
- Preserves all settings and configurations during update
- Automatic rollback if update fails

---

## 6. Alarm System

The OOSIE Internet Radio features a comprehensive alarm system with 5 independent alarms, each configurable with different schedules, stations, and settings.

### 6.1 Alarm Overview

**Alarm Features**:
- **5 Independent Alarms**: Each alarm operates separately with its own settings
- **Multiple Schedules**: Daily, Weekdays (Mon-Fri), Weekends (Sat-Sun), or Once
- **Fade-in Audio**: Gradual volume increase over 60 seconds for gentle wake-up
- **Station Selection**: Each alarm can use a different radio station
- **Snooze Function**: 10-minute snooze with easy cancellation
- **Visual Indicators**: Clock symbol shows when alarms are enabled
- **Volume Preservation**: Alarm volume doesn't affect your saved radio volume

### 6.2 Setting Up Alarms

**Accessing Alarm Settings**:
1. Short press encoder button to enter menu
2. Rotate to "MENU: Alarms"
3. Select desired alarm slot (ALARM 1-5)
4. Configure each setting:
   - **ENABLED**: Turn alarm on/off
   - **TIME**: Set wake-up time (hours blink, then minutes blink)
   - **SCHEDULE**: Choose trigger pattern
   - **STATION**: Select radio station for alarm

**Schedule Types**:
- **DAILY**: Triggers every day at set time
- **WEEKDAYS**: Monday through Friday only
- **WEEKENDS**: Saturday and Sunday only  
- **ONCE**: Single trigger, then automatically disables

### 6.3 Alarm Operation

**When Alarm Triggers**:
```
┌────────────────┐
│ALARM 1  STOP▲  │  ← Shows active alarm and stop option
│Jacaranda FM    │  ← Station name playing
└────────────────┘
```

**Alarm Audio Behavior**:
- Starts very quietly and fades in over 60 seconds
- Uses the radio station configured for that alarm
- Volume increases gradually to full alarm volume
- Your regular radio volume setting is preserved

**Stopping an Alarm**:
- **Short Press**: Snooze for 10 minutes
- **Long Press**: Stop alarm completely

### 6.4 Snooze Function

**Snooze Display**:
```
┌────────────────┐
│12:34 ⚐   22°C☀│  ← Time + indicators + weather
│SNOOZE    09:45 │  ← Snooze with remaining time (MM:SS)
└────────────────┘
```

**Snooze Operation**:
- **Duration**: 10 minutes
- **Activation**: Short press during alarm
- **Cancellation**: Long press during snooze
- **Display**: Shows countdown timer in MM:SS format
- **Auto-restart**: Alarm resumes after snooze period

**Canceling Snooze**:
- Long press (3+ seconds) during snooze period
- Radio can be turned on after canceling snooze
- Snooze cancel shows "ALARM STOPPED" confirmation

### 6.5 Alarm Indicators

**Main Display Indicators**:
```
┌────────────────┐
│12:34 ⚐ Z 22°C☀│  ← Clock symbol (⚐) = enabled alarm
│Jacaranda FM    │     Sleep indicator (Z) = sleep timer
└────────────────┘
```

**Visual Feedback**:
- **Clock Symbol (⚐)**: Appears when any alarm is enabled
- **Confirmation Messages**: "STOPPED", "SNOOZED", "ALARM STOPPED"
- **Backlight Activation**: Display automatically wakes for alarm messages

### 6.6 Alarm Scheduling

**Daily Alarms**: Perfect for regular wake-up times
- Triggers every day at the same time
- Remains enabled until manually turned off

**Weekday Alarms**: Ideal for work schedules  
- Monday through Friday only
- Automatically skips weekends

**Weekend Alarms**: For leisurely mornings
- Saturday and Sunday only
- Lets you sleep in on weekdays

**Once Alarms**: For special occasions
- Single trigger only
- Automatically disables after triggering
- Perfect for appointments or one-time reminders

### 6.7 Advanced Features

**Time Editing with Visual Feedback**:
- Hours blink when editing hour value
- Minutes blink when editing minute value
- Clear indication of which field is being modified

**Volume Preservation**:
- Alarm volume is independent of radio volume
- Your saved volume setting is never modified
- After alarm ends, radio returns to your preferred volume

**Multiple Alarm Management**:
- All 5 alarms can be enabled simultaneously
- Each alarm maintains separate settings
- Easy navigation between alarm slots

**Smart Triggering**:
- Alarms check current time every minute
- Prevents false triggers during time setting
- Reliable operation across power cycles

---

## 7. Web Configuration Interface

### 7.1 WiFi Hotspot Setup

The WiFi hotspot mode provides an easy way to configure your radio's WiFi settings using any web browser:

**Activating Hotspot Mode:**
1. When prompted for WiFi configuration, select "Hotspot Mode"
2. Radio creates an open WiFi network named "OOSIE-Radio"
3. LCD displays rotating instructions every 4 seconds:

```
┌────────────────┐
│Connect to WiFi:│
│OOSIE-Radio     │  ← Connect to this network
└────────────────┘

┌────────────────┐
│Open browser:   │
│192.168.4.1     │  ← Navigate to this address
└────────────────┘

┌────────────────┐
│Configure WiFi  │
│via web page    │  ← Use web interface
└────────────────┘
```

**Connecting to Hotspot:**
1. On your phone/computer, connect to "OOSIE-Radio" WiFi network
2. No password required (open network)
3. Open web browser and go to `192.168.4.1`
4. You'll see the full web configuration interface

**WiFi Configuration via Hotspot:**
1. Click the "📶 WiFi Settings" tab
2. Enter your home WiFi network name (SSID)
3. Enter your WiFi password
4. Optional: Click "🔍 Test Connection" to verify settings
5. Click "💾 Save & Restart"
6. Radio automatically restarts and connects to your network

**Hotspot Features:**
- **No Password Required**: Easy connection from any device
- **Continuous Instructions**: LCD cycles through connection steps
- **Test Before Save**: Verify WiFi credentials before applying
- **Automatic Restart**: Radio restarts and connects to new network
- **Error Recovery**: If connection fails, hotspot mode is available again

### 7.2 Finding the IP Address
The radio's IP address is displayed in the WiFi menu

### 7.2 Web Interface Layout

The web interface features a modern tabbed design with four main sections:

```
🎵 OOSIE Radio
Internet Radio - Management Interface

┌─ Tab Navigation ──────────────────────────────────────────────────────────┐
│ [📻 Stream Manager] [🌤️ Weather Settings] [� WiFi Settings] [📖 Manual] │
└───────────────────────────────────────────────────────────────────────────┘

📻 STREAM MANAGER TAB:
┌─ Stream Management ────────────────────────────────────────┐
│                                                            │
│ Name (max 16 chars)    │ URL                    │ Actions  │
│ ─────────────────────── ─────────────────────── ─────────  │
│ Jacaranda FM           │ https://edge.iono.fm/  │ Delete   │
│ Pretoria FM            │ https://edge.iono.fm/  │ Delete   │
│ [New Stream Name]      │ [Stream URL]           │ Add      │
│                                                            │
│                    [💾 Save All Changes]                   │
└────────────────────────────────────────────────────────────┘

🌤️ WEATHER SETTINGS TAB:
┌─ Weather Configuration ────────────────────────────────────┐
│                                                            │
│ Get your free API key from OpenWeatherMap                  │
│                                                            │
│ Weather API Key: [abc***********xyz]                       │
│                                                            │
│               [💾 Save Weather Settings]                   │
└────────────────────────────────────────────────────────────┘

📖 USER MANUAL TAB:
┌─ User Manual ──────────────────────────────────────────────┐
│                                    [🔄 Refresh Manual]     │
│ ┌─ Manual Content ─────────────────────────────────────────┐
│ │ # OOSIE Internet Radio - User Manual                   │ │
│ │                                                        │ │
│ │ ## Table of Contents                                   │ │
│ │ 1. Introduction                                        │ │
│ │ 2. Hardware Overview                                   │ │
│ │ ...                                                    │ │
│ │ (Complete user manual content from GitHub)             │ │
│ └────────────────────────────────────────────────────────┘ │
└────────────────────────────────────────────────────────────┘
```

### 7.3 Tab Navigation

**Switching Between Tabs**:
- Click any tab button to switch sections
- Active tab is highlighted in green
- Each tab maintains its own content and state

**Tab Features**:
- **Stream Manager**: Add, edit, delete radio stations
- **Weather Settings**: Configure OpenWeatherMap API key
- **WiFi Settings**: Configure WiFi network credentials with testing
- **User Manual**: View complete documentation (fetched live from GitHub)

### 7.4 Managing Radio Stations (Stream Manager Tab)

**Adding Stations**:
1. Click the "📻 Stream Manager" tab if not already selected
2. Enter station name (maximum 16 characters) in the bottom row
3. Enter stream URL (must start with http:// or https://)
4. Click "Add Stream"
5. Click "💾 Save All Changes" to apply

**Editing Stations**:
1. Modify name or URL directly in the table rows
2. Character count is shown below name fields (e.g., "12/16")
3. Click "💾 Save All Changes" to apply

**Deleting Stations**:
1. Click "Delete" button next to unwanted station
2. Confirm deletion in popup
3. Click "💾 Save All Changes" to apply

**Important Notes**:
- Changes are not applied until "💾 Save All Changes" is clicked
- Radio will restart automatically after saving streams
- At least one stream must remain in the list
- Stream names exceeding 16 characters will show red warning

### 7.5 Weather Configuration (Weather Settings Tab)

**Setting Up Weather**:
1. Click the "🌤️ Weather Settings" tab
2. Visit [OpenWeatherMap.org](https://openweathermap.org/api) to create free account
3. Obtain your API key from the dashboard
4. Enter API key in the web interface
5. Click "💾 Save Weather Settings"

**API Key Security**:
- After saving, API key is masked for security: `abc***********xyz`
- First 3 and last 3 characters remain visible
- To change: clear field and enter new key
- Masked keys show placeholder: "API key is configured (masked for security)"

### 7.7 WiFi Settings (WiFi Settings Tab)

**Configuring WiFi via Web Interface**:
1. Click the "📶 WiFi Settings" tab
2. Enter your WiFi network name (SSID) in the first field
3. Enter your WiFi password in the second field
4. Optional: Check "Show password" to verify entry
5. Click "🔍 Test Connection" to verify settings before saving
6. Click "💾 Save & Restart" to apply and restart

**WiFi Configuration Features**:
- **Connection Testing**: Verify credentials before committing changes
- **Password Visibility**: Toggle to show/hide password during entry
- **Automatic Restart**: Radio restarts and connects to new network
- **Error Handling**: Clear feedback if connection fails
- **Security**: Existing passwords are masked when loading settings

**Connection Test Results**:
- **Success**: Shows "✅ Connection test successful! You can now save these settings."
- **Failure**: Shows "❌ Connection failed: [specific error message]"
- **No SSID**: Shows warning to enter network name

**WiFi Settings Security**:
- Current WiFi password is never displayed (shows placeholder text)
- Only network name (SSID) is shown when loading existing settings
- New passwords are hidden by default with show/hide option

### 7.8 User Manual (User Manual Tab)

**Accessing Documentation**:
1. Click the "📖 User Manual" tab
2. Manual loads automatically from GitHub repository
3. Content is formatted with proper headings, lists, and code blocks
4. Scrollable content area for easy navigation

**Manual Features**:
- **Live Content**: Always shows the latest version from GitHub
- **Refresh Button**: Click "🔄 Refresh Manual" to reload content
- **Offline Fallback**: Shows error message if GitHub is unavailable
- **Formatted Display**: Markdown content converted to readable HTML

**Manual Sections Include**:
- Complete setup instructions
- Menu system documentation
- Alarm configuration guide
- Troubleshooting tips
- Technical specifications

---

## 8. Sleep Timer

### 8.1 Setting Sleep Timer

**Through Menu**:
1. Enter menu system (short press)
2. Navigate to Sleep menu
3. Select desired duration: 15, 30, 60, 90, or 5 minutes
4. Timer starts immediately and menu exits

**Visual Indicators**:
- "Z" appears between time and weather on main display
- Sleep timer resets to "blank" option when re-entering menu

### 8.2 Sleep Timer Behavior

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

## 9. Weather Information

### 9.1 Weather Display

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

### 9.2 Weather Menu Details

Access detailed weather information through the Weather menu:

**Temperature**: Current temperature in Celsius
**Humidity**: Relative humidity percentage
**Description**: Weather condition in text
**API Status**: Shows if API key is configured
**Manual Update**: Force immediate weather refresh

### 9.3 Weather Configuration

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

## 10. Troubleshooting

### 10.1 Common Issues

**No WiFi Connection**:
- **First Time Setup**: Use hotspot mode for easier configuration
- **Existing Setup**: Check WiFi credentials in WiFi menu or web interface
- **Network Issues**: Ensure network is 2.4GHz (5GHz not supported)
- **Reset Options**: Try WiFi reset if connection fails, or use hotspot mode
- **Router Settings**: Check WPA/WPA2 support, verify network password
- **Hotspot Recovery**: If normal WiFi fails, radio automatically offers hotspot mode

**No Audio Output**:
- Verify radio is powered ON (check display)
- Check volume level (rotate encoder outside menu)
- Ensure stream URL is valid and accessible
- Try different radio station
- Check alarm volume preservation isn't interfering

**Display Issues**:
- **Blank Display**: Check power connection and backlight settings
- **Garbled Text**: Reset radio by power cycling
- **No Response**: Check encoder connections

**Weather Not Updating**:
- Verify API key is configured correctly
- Check internet connection
- Wait up to 10 minutes for automatic update
- Use manual update in Weather menu

**Alarm Issues**:
- **Alarm Not Triggering**: Check enabled status and schedule type
- **Wrong Station Playing**: Verify station selection in alarm settings
- **Volume Too Loud/Quiet**: Alarm volume is independent of radio volume
- **Clock Symbol Missing**: Ensure at least one alarm is enabled

### 10.2 Reset Procedures

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

### 10.3 Error Messages

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

**Alarm-related errors**:
- "ALARM STOPPED": Confirmation that alarm was manually stopped
- "SNOOZED": Confirmation that alarm is in snooze mode
- Missing clock symbol: Check that at least one alarm is enabled

**OTA Update errors**:
- "Update Error / Check WiFi": Network connectivity issues or GitHub API unreachable
- "Update Failed / Try again later": Download interrupted or installation error
- "JSON parse error: NoMemory": Insufficient memory for update process (restart device)
- "Up to Date": No newer firmware version available (normal message)

### 10.4 OTA Update Troubleshooting

**Update Process Fails**:
- Ensure stable WiFi connection throughout update
- Check that device has sufficient free memory (restart if needed)
- Verify internet access to api.github.com and github.com
- Try update during low network usage periods

**Memory Issues During Update**:
- Restart the radio before attempting update
- Close any unnecessary processes or applications on your network
- Ensure device has been running stably before update attempt

**Network Connectivity for Updates**:
- Verify WiFi signal strength is adequate
- Test internet connectivity by checking weather updates first
- Ensure firewall/router allows HTTPS connections to GitHub
- Check if GitHub is accessible from your network

---

## 11. Technical Specifications

### 11.1 Hardware Specifications

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
- **Hotspot Mode**: Can create "OOSIE-Radio" access point for configuration
- **Web Interface**: Built-in web server for remote configuration

### 11.2 Software Specifications

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

### 11.3 Power Requirements

**Input Voltage**: 5V DC
**Current Consumption**:
- **Idle**: 150mA
- **Playing**: 200mA
- **Peak**: 300mA

**Power Connector**: USB-C or 2.1mm DC jack

### 11.4 Environmental Specifications

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

*OOSIE Internet Radio User Manual v1.1*
*Last Updated: August 2025*
*Added: OTA Firmware Update System*



