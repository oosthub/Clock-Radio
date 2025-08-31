#ifndef CONFIG_H
#define CONFIG_H

// Firmware version
#define FIRMWARE_VERSION "1.0.7"

// Hardware pin definitions
#define I2S_DOUT  2
#define I2S_BCLK  3
#define I2S_LRC   4

#define ENCODER_A    10
#define ENCODER_B    11
#define ENCODER_BTN  12

#define SDA_PIN  8
#define SCL_PIN  9

// Configuration constants
#define LCD_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

#define PULSES_PER_STEP 2
#define MENU_TIMEOUT 6000
#define LCD_UPDATE_INTERVAL 1000
#define VOLUME_DISPLAY_TIMEOUT 5000
#define BACKLIGHT_TIMEOUT 5000

// EEPROM settings
#define EEPROM_SIZE 512
#define SETTINGS_VERSION 6

// NTP settings
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 7200
#define DAYLIGHT_OFFSET_SEC 0

#endif
