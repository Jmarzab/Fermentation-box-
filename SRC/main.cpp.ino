// *************************************************************
//  MAIN.CPP - MAIN CONTROL LOGIC
// *************************************************************
//  This file contains the `setup()` and `loop()` functions, 
//  which initialize and control the fermentation chamber system.
//  - `setup()`: Initializes all hardware components (display, SD, sensors).
//  - `loop()`: Handles touchscreen interactions, sensor readings, 
//              auto/manual mode logic, and UI updates.
// *************************************************************

#include "config.h"  // Include global configuration file

// *************************************************************
//  GLOBAL OBJECTS & VARIABLES
// *************************************************************
MCUFRIEND_kbv tft;  // TFT Display object
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);  // Touchscreen object
RTC_DS3231 rtc;  // RTC (Real-Time Clock) object
DHT dht(DHTPIN, DHTTYPE);  // DHT sensor object
File myFile;  // File object for SD operations

// Flag to check if the RTC is available
bool rtcAvailable = false;

// Last interaction timestamp (for inactivity timeout)
unsigned long lastInteractionTime = 0;
const unsigned long inactivityTimeout = 15000; // 15 seconds

// Stores current temperature and humidity readings
float temperature = NAN;
float humidity = NAN;

// *************************************************************
//                          SETUP
// *************************************************************
void setup() {
  Serial.begin(9600);  // Start serial communication for debugging
  Serial.println("Starting setup...");

  // ********** Initialize RTC **********
  Serial.println("Initializing RTC...");
  if (!rtc.begin()) {
    Serial.println("RTC not connected, using simulated time.");
    rtcAvailable = false;
  } else {
    Serial.println("RTC initialized successfully.");
    rtcAvailable = true;
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, setting default date/time.");
      rtc.adjust(DateTime(2025, 1, 1, 8, 0, 0)); // Default simulation time
    }
  }

  // ********** Initialize TFT Display **********
  Serial.println("Initializing TFT...");
  tft.reset();
  uint16_t identifier = tft.readID();
  if (identifier == 0x0 || identifier == 0xFFFF) {
    Serial.println("Using fallback identifier for TFT.");
    identifier = 0x9327;  // Default identifier
  }
  tft.begin(identifier);
  Serial.print("TFT initialized with ID: 0x");
  Serial.println(identifier, HEX);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);  // Clear the screen
  Serial.println("TFT screen cleared.");

  // ********** Initialize Touchscreen **********
  Serial.println("Initializing touchscreen...");
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  Serial.println("Touchscreen initialized.");

  // ********** Initialize DHT Sensor **********
  Serial.println("Initializing DHT sensor...");
  dht.begin();
  Serial.println("DHT sensor initialized.");

  // ********** Initialize Actuators **********
  Serial.println("Initializing actuators...");
  pinMode(WATER_SENSOR_PIN, INPUT_PULLUP);
  pinMode(LED_PANEL_PIN1, OUTPUT);
  pinMode(LED_PANEL_PIN2, OUTPUT);
  pinMode(HEATING_PAD_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(UV_LIGHT_PIN, OUTPUT);
  Serial.println("Actuators initialized.");

  // Ensure all actuators are OFF at startup
  Serial.println("Turning off all actuators...");
  digitalWrite(LED_PANEL_PIN1, LOW);
  digitalWrite(LED_PANEL_PIN2, LOW);
  digitalWrite(HEATING_PAD_PIN, LOW);
  digitalWrite(HUMIDIFIER_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(UV_LIGHT_PIN, LOW);
  Serial.println("All actuators turned off.");

  // ********** Initialize SD Card **********
  Serial.println("Initializing SD card...");
  sdInitialized = SD.begin(SD_CS_PIN);
  if (sdInitialized) {
    Serial.println("SD card initialized successfully.");
    loadSettingsFromSD();
  } else {
    Serial.println("SD card initialization failed.");
  }

  // Set default fan mode
  Serial.println("Setting default fan mode...");
  fanModeFullAuto = true;
  fanModeSemiAuto = false;

  // Draw main screen
  Serial.println("Drawing main screen...");
  drawMainScreenStatic();
  Serial.println("Setup complete.");
}

// *************************************************************
//                          LOOP
// *************************************************************
void loop() {
  static unsigned long lastTimeUpdate = 0;
  
  // ********** 1) Update Simulation Time (if no RTC) **********
  if (!rtcAvailable) {
    updateSimTime();
  }

  // ********** 2) Manage Light Timer **********
  checkLightsTimer();

  // ********** 3) Read Temperature & Humidity **********
  float newTemp = dht.readTemperature();
  float newHum = dht.readHumidity();
  if (!isnan(newTemp)) temperature = newTemp;
  if (!isnan(newHum)) humidity = newHum;

  // ********** 4) Handle Touch Input **********
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    int mappedX = map(p.y, 150, 900, 0, 400);
    int mappedY = map(p.x, 900, 150, 0, 240);
    lastInteractionTime = millis();

    if (dataScreenActive) {
      handleDataScreenTouch(mappedX, mappedY);
    } else {
      handleTouch(mappedX, mappedY);
    }
  }

  // ********** 5) Periodic Alarm Check **********
  static unsigned long lastAlarmCheckTime = 0;
  if (millis() - lastAlarmCheckTime >= 1000) {
    lastAlarmCheckTime = millis();
    checkAlarms();
  }

  // ********** 6) Auto/Manual Mode Control **********
  if (autoModeActive) {
    autoModeControl();
  } else {
    manualModeControl();
  }

  // ********** 7) Inactivity Timeout (Switch to Data Screen) **********
  if (!dataScreenActive && (millis() - lastInteractionTime > inactivityTimeout)) {
    dataScreenActive = true;
    drawDataScreen();
  }

  // ********** 8) Periodic Time Display Update **********
  if (millis() - lastTimeUpdate > 1000) {
    if (currentScreen == MAIN_SCREEN) {
      drawCurrentTime(190, 10, true);
    } else if (currentScreen == SCREEN_DATA) {
      drawCurrentTime(190, 10, false);
    }
    lastTimeUpdate = millis();
  }

  // Reset buffers when switching screens
  static ScreenState lastScreen = currentScreen;
  if (lastScreen != currentScreen) {
    lastMainTime[0] = '\0';
    lastDataTime[0] = '\0';
    lastScreen = currentScreen;
  }

  // ********** 9) Refresh Data Screen Periodically **********
  static unsigned long lastDataRefresh = 0;
  if (dataScreenActive) {
    unsigned long now = millis();
    if (now - lastDataRefresh >= 1000) {
      lastDataRefresh = now;
      refreshDataScreen();
    }
  }
}
