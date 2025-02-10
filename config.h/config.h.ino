// *************************************************************
//  CONFIG.H - Global Configuration File
// *************************************************************
//  This file centralizes all global definitions, pin assignments,
//  color definitions, and constants. It allows for easy modifications
//  and ensures that all configuration parameters are kept consistent.
// *************************************************************

#ifndef CONFIG_H
#define CONFIG_H  // Prevent multiple inclusions of this file

// *************************************************************
//  INCLUDE NECESSARY LIBRARIES
// *************************************************************
//  - Adafruit_GFX.h & MCUFRIEND_kbv.h: Used for TFT display control
//  - TouchScreen.h: Handles touchscreen input
//  - SD.h & SPI.h: Required for SD card operations
//  - math.h: Provides functions like NAN (for invalid sensor data)
//  - DHT.h: Manages DHT21 temperature & humidity sensor
//  - RTClib.h: Provides Real-Time Clock (RTC) functionality
// *************************************************************

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <SD.h>
#include <SPI.h>
#include <math.h>
#include <DHT.h>
#include <RTClib.h>

// *************************************************************
//  COLOR DEFINITIONS (16-bit RGB565 format)
// *************************************************************
//  These colors are used throughout the UI. If changes are needed,
//  modifying them here will automatically update the entire system.
// *************************************************************

#define TFT_BROWN 0xA145        // Dark brown color for buttons
#define TFT_BLUE 0x5C9F         // Light blue color for selection highlights
#define TFT_BLACK 0x0000        // Black, used for background
#define TFT_WHITE 0xFFFF        // White, used for text
#define TFT_YELLOW 0xFFE0       // Yellow, used for warnings
#define TFT_GRAY 0xC618         // Gray, used for neutral UI elements
#define TFT_GREEN tft.color565(34, 139, 34) // British Racing Green for active states
#define TFT_ORANGE tft.color565(255, 165, 0) // Orange, used for alert indicators

// *************************************************************
//  LCD & TOUCHSCREEN PIN ASSIGNMENTS
// *************************************************************
//  The touchscreen operates using 4 pins (XP, XM, YP, YM).
//  These pins are connected to the TFT display for reading input.
// *************************************************************

#define YP A2   // Y+ touch pin (analog input)
#define XM A1   // X- touch pin (analog input)
#define YM 6    // Y- touch pin (digital output)
#define XP 7    // X+ touch pin (digital output)

// *************************************************************
//  SD CARD MODULE CONFIGURATION
// *************************************************************
//  The SD card module is used to store logs and settings. 
//  It connects to the SPI bus and requires a chip select (CS) pin.
// *************************************************************

#define SD_CS_PIN 53  // Chip Select (CS) pin for SD card module

// *************************************************************
//  TEMPERATURE & HUMIDITY SENSOR (DHT21)
// *************************************************************
//  The DHT21 sensor measures temperature and humidity.
//  - DHTPIN: Defines the digital pin connected to the sensor.
//  - DHTTYPE: Specifies the sensor model (DHT21).
// *************************************************************

#define DHTPIN 17       // Data pin for DHT21 sensor
#define DHTTYPE DHT21   // Sensor type definition (DHT21)

// *************************************************************
//  LIGHT CONTROL PINS
// *************************************************************
//  The system uses two LED channels to control lighting.
//  - LED_PANEL_PIN1: Controls the first LED panel
//  - LED_PANEL_PIN2: Controls the second LED panel
// *************************************************************

#define LED_PANEL_PIN1 30  // Output pin for LED panel 1
#define LED_PANEL_PIN2 34  // Output pin for LED panel 2

// *************************************************************
//  OTHER ACTUATORS & SENSORS
// *************************************************************
//  The system includes multiple actuators and sensors for monitoring
//  and controlling the fermentation chamber environment. 
//  These include water level sensors, a humidifier, a heating pad,
//  a fan, and a UV light for sterilization.
// *************************************************************

#define WATER_SENSOR_PIN 51  // Input pin for water level sensor (0 = empty, 1 = full)
#define HUMIDIFIER_PIN 18    // Output pin for humidifier control
#define HEATING_PAD_PIN 25   // Output pin for heating pad (temperature control)
#define FAN_PIN 19           // Output pin for fan (humidity/temperature control)
#define UV_LIGHT_PIN 24      // Output pin for UV sterilization light

#endif  // CONFIG_H
