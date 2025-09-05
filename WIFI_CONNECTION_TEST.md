# WiFi Connection Stability Test Guide

This document describes how to test the WiFi connection stability improvements implemented to prevent the ESP32 from losing WiFi connection during extended periods when the radio is off.

## Problem Addressed

The radio would lose WiFi connection when turned off for extended periods, requiring manual reconfiguration upon startup.

## Solution Implemented

1. **WiFi Power Management Configuration**
   - Disabled WiFi sleep mode (`WiFi.setSleep(false)`)
   - Enabled automatic reconnection (`WiFi.setAutoReconnect(true)`)
   - Enabled persistent credentials (`WiFi.persistent(true)`)

2. **Connection Monitoring**
   - Status check every 30 seconds
   - Automatic reconnection attempts every 60 seconds
   - Non-blocking reconnection with 15-second timeout

## How to Test

### Test 1: Basic WiFi Stability
1. Configure WiFi on the radio
2. Turn the radio OFF (long press)
3. Wait for at least 1 hour
4. Turn the radio ON (long press)
5. **Expected**: Radio should connect immediately without needing reconfiguration

### Test 2: Network Interruption Recovery
1. Configure WiFi and ensure radio is connected
2. Temporarily disable your WiFi router or change WiFi password
3. Watch serial output for "WiFi connection lost" messages
4. Re-enable router or restore WiFi password
5. **Expected**: Radio should automatically reconnect within 60 seconds

### Test 3: Extended Offline Test
1. Configure WiFi on the radio
2. Turn radio OFF
3. Leave overnight (8+ hours)
4. Turn radio ON the next day
5. **Expected**: Radio should start and connect without reconfiguration

## Serial Output to Monitor

Look for these messages in the serial console:

### Successful WiFi Setup
```
WiFi connected
IP address: 192.168.x.x
WiFi power management configured - sleep disabled, auto-reconnect enabled
```

### Connection Monitoring
```
WiFi connection lost - attempting reconnection...
WiFi reconnected successfully
IP address: 192.168.x.x
WiFi power management configured - sleep disabled, auto-reconnect enabled
```

### Reconnection Timeout
```
WiFi reconnection timeout, will retry later
```

## Verification Points

- [ ] WiFi power management is configured after each connection
- [ ] Connection monitoring runs every 30 seconds
- [ ] Reconnection attempts are limited to every 60 seconds
- [ ] System remains responsive during reconnection
- [ ] Radio functionality is unaffected when WiFi is stable
- [ ] No blocking delays in main loop
- [ ] Settings persist across power cycles

## Implementation Files Changed

- `src/main.cpp`: Added monitoring loop and power management setup
- `src/wifi_config.cpp`: Added power management function and configuration
- `include/wifi_config.h`: Added function declaration

## Expected Behavior

With these changes, the radio should:
1. Never enter WiFi sleep mode
2. Automatically attempt reconnection if connection is lost
3. Maintain connection even when radio is "off" for extended periods
4. Reconnect without user intervention after network outages
5. Not require WiFi reconfiguration unless credentials actually change

## Troubleshooting

If WiFi still disconnects:
1. Check router power saving settings
2. Verify WiFi signal strength is adequate
3. Check for router-side connection timeouts
4. Monitor serial output for error messages
5. Verify power supply stability to the ESP32

The implementation prioritizes connection stability over power consumption, which is appropriate for a mains-powered internet radio device.