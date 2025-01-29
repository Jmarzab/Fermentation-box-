/*************************************************************
 *                      INCLUDES & LIBRARIES
 *************************************************************/
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <SD.h>
#include <SPI.h>
#include <math.h>    // For NAN and isnan()
#include <DHT.h>     // For DHT sensor
#include <RTClib.h>  // For DS1307/DS3231 RTC

/*************************************************************
 *                  RTC & TIME SIMULATION
 *************************************************************/
// RTC detection flag
bool rtcAvailable = false;

// RTC object
RTC_DS3231 rtc;

int currentDigitIndex = 0;   // 0..9 por ejemplo
bool editingDigits = false;  // true si estamos en modo "Set -> OK"
unsigned long lastTimeUpdate = 0;
// 1. Variables globales para control de estado
char lastMainTime[6] = "";  // Buffer para Main Screen
char lastDataTime[6] = "";  // Buffer para Data Screen


/**
 * By default, if no RTC is found, we simulate from 
 * 1/1/2025 08:00. This simulation is minute-based.
 */
int simYear = 2025;
int simMonth = 1;
int simDay = 1;
int simHour = 8;
int simMinute = 0;

// For a basic minute-based simulation (no seconds)
unsigned long lastSimUpdate = 0UL;  // we increment time each 60s

// Temporary storage for Set Clock screen
int tempClockYear = 2025;
int tempClockMonth = 1;
int tempClockDay = 1;
int tempClockHour = 8;
int tempClockMinute = 0;

/*************************************************************
 *                LIGHT CONTROL SETTINGS
 *************************************************************/
/**
 * The user can set an OnTime (HH:MM) and OffTime (HH:MM).
 * If the user sets "lightsTimerEnabled", we compare the current
 * time (RTC or simulated) with these hours to decide if lights 
 * are ON or OFF. We handle intensity (50% or 100%).
 */
int lightOnHour = 8;
int lightOnMinute = 0;
int lightOffHour = 20;
int lightOffMinute = 0;

int lightIntensity = 0;  // can be 50% or 100%
bool lightsTimerEnabled = true;

/*************************************************************
 *                      COLOR DEFINITIONS
 *************************************************************/
#define TFT_BROWN 0xA145
#define TFT_BLUE 0x5C9F
#define TFT_BLACK 0x0000
#define TFT_WHITE 0xFFFF
#define TFT_YELLOW 0xFFE0
#define TFT_GRAY 0xC618
#define TFT_GREEN tft.color565(34, 139, 34)
#define TFT_ORANGE tft.color565(255, 165, 0)

/*************************************************************
 *             LCD/TOUCHSCREEN PIN ASSIGNMENTS
 *************************************************************/
#define YP A2
#define XM A1
#define YM 6
#define XP 7

/*************************************************************
 *                SD CARD PIN ASSIGNMENT
 *************************************************************/
#define SD_CS_PIN 53  // Chip select pin for SD card
File myFile;

/*************************************************************
 *                   DHT SENSOR CONFIGURATION
 *************************************************************/
#define DHTPIN 17
#define DHTTYPE DHT21
DHT dht(DHTPIN, DHTTYPE);

/*************************************************************
 *                LIGHT CONTROL GLOBAL VARIABLES
 *************************************************************/
/**
 * LED panel pins (2 channels).
 * We use two booleans to indicate if each channel is ON.
 * The actual digitalWrite() is done in checkLightsTimer() 
 * (auto mode) or handleManualSettingsTouch() (manual).
 */
#define LED_PANEL_PIN1 30
#define LED_PANEL_PIN2 34
bool ledPanel1On = false;
bool ledPanel2On = false;
bool lightsCompletelyOff = false;  // define it globally


/*************************************************************
 *                OTHER GLOBAL ACTUATORS & SENSORS
 *************************************************************/
#define WATER_SENSOR_PIN 51  // 0 = empty, 1 = full
#define HUMIDIFIER_PIN 18
#define HEATING_PAD_PIN 25
#define FAN_PIN 19
#define UV_LIGHT_PIN 24

bool ledsManualOn = false;
bool uvLightOn = false;
bool mattOn = false;
bool fanOnManual = false;
bool humidifierOn = false;

/*************************************************************
 *        TEMPERATURE, HUMIDITY, ALARM, FAN, ETC.
 *************************************************************/
#define MAX_TEMP 50.0
#define MAX_HUMIDITY 80.0
#define HUMIDITY_TOLERANCE 0.5
#define TEMPERATURE_TOLERANCE 0.5

bool waterLevelEmpty = false;
bool tempAlarmActive = false;
bool humidityAlarmActive = false;
bool waterAlarmActive = false;

bool fanModeFullAuto = false;
bool fanModeSemiAuto = false;

int fanOnTime = 0;
int fanOffTime = 30;

/*************************************************************
 *         pH SENSOR & CALIBRATION PARAMETERS
 *************************************************************/
#define PH_SENSOR_PIN A15
#define PH_TEMP_SENSOR_PIN 15

float pH7Value = 0.0;
float pH4Value = 0.0;
bool pH7Calibrated = false;
bool pH4Calibrated = false;
float pHSlope = 1.0;
float pHIntercept = 0.0;
bool calibratingPH7 = false;
bool calibratingPH4 = false;

/*************************************************************
 *                MISC. GLOBALS
 *************************************************************/
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
MCUFRIEND_kbv tft;

#define MINPRESSURE 200
#define MAXPRESSURE 1000

// Variable para almacenar el tiempo de la última pulsación válida
unsigned long lastButtonPressTime = 0;
const unsigned long debounceDelay = 500;  // 500 ms de debounce


bool sdInitialized = false;
bool autoModeActive = true;

// For inactivity => data screen
unsigned long lastInteractionTime = 0;
const unsigned long inactivityTimeout = 15000;  // 15 seconds
bool dataScreenActive = false;

float temperature = NAN;
float humidity = NAN;

/*************************************************************
 *         ALARM & AUTO CONTROL THRESHOLDS
 *************************************************************/
float autoTemperatureSetpoint = 25.0;
float autoHumiditySetpoint = 60.0;
float alarmTemperatureSetpoint = 30.0;
float alarmHumiditySetpoint = 80.0;

#define UV_LIGHT_MAX_TIME 600
float targetTemperature = 25.0;
float targetHumidity = 60.0;
const float TEMP_MARGIN = 2.0;
const float HUM_MARGIN = 5.0;

/*************************************************************
 *             TRACK OLD STATES FOR DISPLAY
 *************************************************************/
bool oldMattOn = false;
bool oldHumidifierOn = false;
bool oldFanOnManual = false;
bool oldUvLightOn = false;
bool oldLed1On = false;
bool oldLed2On = false;

/*************************************************************
 *       ENUMS, SCREEN STATES, PARAM ADJUST, ETC.
 *************************************************************/
enum Parameter { TEMP,
                 HUMIDITY,
                 FAN_ON_TIME,
                 FAN_OFF_TIME };
Parameter currentParameter;

enum ScreenState {
  MAIN_SCREEN,
  MENU_SCREEN,
  AUTO_SETTINGS,
  MANUAL_SETTINGS,
  PARAMETER_ADJUST,
  PH_CALIBRATION,
  FAN_MODE_SETTINGS,
  FAN_TIME_SETTINGS,
  ALARM_SETTINGS,
  SD_CARD_MENU,
  SET_CLOCK_SCREEN,
  SET_ON_TIME,
  SET_OFF_TIME,
  SCREEN_DATA,
  LIGHT_SETTINGS
}; 

ScreenState currentScreen = MAIN_SCREEN;

// In an advanced approach, you would separate setpoints for auto vs. alarm.
enum ParameterAdjustOrigin {
  FROM_AUTO_SETTINGS,
  FROM_ALARM_SETTINGS,
  FROM_UNKNOWN
};
ParameterAdjustOrigin parameterAdjustOrigin = FROM_UNKNOWN;

/*************************************************************
 *   ALARM CHECK TIMERS
 *************************************************************/
unsigned long lastAlarmCheck = 0;
const unsigned long alarmCheckInterval = 2000;
bool waterAlarmLastState = false;
bool tempAlarmLastState = false;
bool humidityAlarmLastState = false;

/*************************************************************
 *   INACTIVITY => DATA SCREEN
 *************************************************************/
unsigned long lastScreenUpdate = 0;
const unsigned long screenUpdateInterval = 1000;  // 1 second

/*************************************************************
 *       DEFINES FOR DATA SCREEN INVISIBLE BUTTON
 *************************************************************/
#define INVIS_BTN_X 360
#define INVIS_BTN_Y 0
#define INVIS_BTN_WIDTH 40
#define INVIS_BTN_HEIGHT 40

/*************************************************************
 *                   TARGETS FOR CONTROL
 *************************************************************/
#define MAX_TEMP 50.0
#define MAX_HUMIDITY 80.0

/*************************************************************
 *                    FUNCTION PROTOTYPES
 *************************************************************/
/**
 * Setup and Loop
 */
void setup();
void loop();

/**
 * Time simulation and lights
 */
void updateSimTime();
void checkLightsTimer();
void getCurrentTime(int& hour, int& minute);

/**
 * Main UI screens
 */
void drawMainScreenStatic();
void drawMenu();
void drawAutoSettings();
void drawManualSettings();
void drawAlarmSettings();
void drawPhCalibrationScreen();
void drawFanModeSettings();
void drawFanTimeSettings();
void drawLightSettingsScreen();
void drawSDCardMenu();
void drawDataScreen();
void drawCurrentTime(int x, int y, bool isMainScreen);


/**
 * Clock Setting Screen
 */
void drawSetClockScreen();
void handleSetClockTouch(int x, int y);
void refreshSetClockValues();

/**
 * Parameter Adjust, fan time, etc.
 */
void drawParameterAdjustScreen(const char* parameterName, float currentValue);
void handleParameterAdjustTouch(int x, int y);
void drawValueWindow(int x, int y, int w, int h, float value);
void drawFanTimeValue(int x, int y, int value);
void drawParameterValue(float value);


/**
 * Gauges
 */
void drawTemperatureGauge(int x, int y, int radius, float value);
void drawHumidityGauge(int x, int y, int radius, float value);
void drawPHBar(int x, int y, int width, int height, float pHValue);
void drawCircularGaugeWithGradient(int x, int y, int radius, float value, float maxValue, float minValue,
                                   uint16_t startColor, uint16_t midColor, uint16_t endColor);
uint16_t interpolateColor(uint16_t color1, uint16_t color2, float fraction);

/**
 * Button Draw Helpers
 */
void drawButton(int x, int y, int w, int h, const char* label, uint16_t color, uint16_t textColor);
void drawButtonWithFeedback(int x, int y, int w, int h, const char* label,
                            uint16_t color, uint16_t textColor, bool pressed = false);

/**
 * Handling Touch
 */
void handleTouch(int x, int y);
void handleMainScreenTouch(int x, int y);
void handleMenuTouch(int x, int y);
void handleAutoSettingsTouch(int x, int y);
void handleManualSettingsTouch(int x, int y);
void handleAlarmSettingsTouch(int x, int y);
void handlePhCalibrationTouch(int x, int y);
void handleFanModeSettingsTouch(int x, int y);
void handleFanTimeSettingsTouch(int x, int y);
void handleLightSettingsTouch(int x, int y);
void handleDataScreenTouch(int x, int y);

/**
 * Data Screen Refresh
 */
void refreshDataScreen();

/**
 * Device Control
 */
void autoModeControl();
void manualModeControl();
void updateDeviceStatusDisplay();
void updatePilotsIfChanged();

/**
 * Alarms
 */
void checkAlarms();
void checkAlarmsOnMainScreen();

/**
 * pH calibration
 */
void calibratePH7();
void calibratePH4();
void calculatePHCalibration();
float getCalibratedPH();

/**
 * SD Card
 */
bool initializeSDCard();
void displaySDError();
void displaySDData();
void displaySDGraphs();
void displaySDAlarms();
void deleteSDRecords();
void displaySDMessage(const char* message);
void displaySDMoreOptions();
void handleSDCardMenuTouch(int x, int y);
void logDataToSD();
void loadSettingsFromSD();
void saveSettingsToSD();
void plotSDGraph();

/**
 * Helper for parameters
 */
const char* getParameterName(Parameter param);
float getParameterValue(Parameter param);

/*************************************************************
 *                         SETUP
 *************************************************************/
void setup() {
  Serial.begin(9600);
  Serial.println("Starting setup...");

  // RTC init
  Serial.println("Initializing RTC...");
  if (!rtc.begin()) {
    Serial.println("RTC not connected, using simulated time.");
    rtcAvailable = false;
  } else {
    Serial.println("RTC initialized successfully.");
    rtcAvailable = true;
    if (rtc.lostPower()) {
      Serial.println("RTC lost power, setting default date/time.");
      rtc.adjust(DateTime(simYear, simMonth, simDay, simHour, simMinute, 0));
    }
  }

  // TFT init
  Serial.println("Initializing TFT...");
  tft.reset();
  uint16_t identifier = tft.readID();
  if (identifier == 0x0 || identifier == 0xFFFF) {
    Serial.println("Using fallback identifier for TFT.");
    identifier = 0x9327;  // fallback
  }
  tft.begin(identifier);
  Serial.print("TFT initialized with ID: 0x");
  Serial.println(identifier, HEX);
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  Serial.println("TFT screen cleared.");

  // Touch sensor input
  Serial.println("Initializing touch sensor...");
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  Serial.println("Touch sensor initialized.");

  // DHT sensor
  Serial.println("Initializing DHT sensor...");
  dht.begin();
  Serial.println("DHT sensor initialized.");

  // Other pins
  Serial.println("Initializing other pins...");
  pinMode(WATER_SENSOR_PIN, INPUT_PULLUP);
  pinMode(LED_PANEL_PIN1, OUTPUT);
  pinMode(LED_PANEL_PIN2, OUTPUT);
  pinMode(HEATING_PAD_PIN, OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(UV_LIGHT_PIN, OUTPUT);
  Serial.println("Pins initialized.");

  // Turn off all actuators
  Serial.println("Turning off all actuators...");
  digitalWrite(LED_PANEL_PIN1, LOW);
  digitalWrite(LED_PANEL_PIN2, LOW);
  digitalWrite(HEATING_PAD_PIN, LOW);
  digitalWrite(HUMIDIFIER_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(UV_LIGHT_PIN, LOW);
  Serial.println("Actuators turned off.");

  // SD card init
  Serial.println("Initializing SD card...");
  sdInitialized = SD.begin(SD_CS_PIN);
  if (sdInitialized) {
    Serial.println("SD card initialized successfully.");
    loadSettingsFromSD();
  } else {
    Serial.println("SD card initialization failed.");
  }

  // Default fan mode
  Serial.println("Setting default fan mode...");
  fanModeFullAuto = true;
  fanModeSemiAuto = false;

  // Draw main screen
  Serial.println("Drawing main screen...");
  drawMainScreenStatic();
  Serial.println("Setup complete.");
}


/*************************************************************
 *                          LOOP
 *************************************************************/
void loop() {

  

static unsigned long lastTimeUpdate = 0;

  // 1) If RTC not available, update simulation
  if (!rtcAvailable) {
    updateSimTime();
  }
  // 1) If RTC not available, update simulation
  if (!rtcAvailable) {
    updateSimTime();
  }

  // 2) Check the lights ON/OFF automatically if lightsTimerEnabled
  checkLightsTimer();

  // 3) Read DHT sensor
  float newTemp = dht.readTemperature();
  float newHum = dht.readHumidity();
  if (!isnan(newTemp)) temperature = newTemp;
  if (!isnan(newHum)) humidity = newHum;

  // 4) Touch reading
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    int mappedX = map(p.y, 150, 900, 0, 400);
    int mappedY = map(p.x, 900, 150, 0, 240);
    lastInteractionTime = millis();

    if (dataScreenActive) {
      // If data screen is active, we only check invisible button
      handleDataScreenTouch(mappedX, mappedY);
    } else {
      // Normal UI
      handleTouch(mappedX, mappedY);
    }
  }

  // 5) Periodically check alarms
  static unsigned long lastAlarmCheckTime = 0;
  if (millis() - lastAlarmCheckTime >= 1000) {
    lastAlarmCheckTime = millis();
    checkAlarms();
  }

  // 6) Auto or Manual mode logic
  if (autoModeActive) {
    autoModeControl();
  } else {
    manualModeControl();
  }

  // 7) Inactivity => Data Screen
  if (!dataScreenActive && (millis() - lastInteractionTime > inactivityTimeout)) {
    dataScreenActive = true;
    drawDataScreen();
  }

 // Actualización periódica
    if(millis() - lastTimeUpdate > 1000) {
        if(currentScreen == MAIN_SCREEN) {
            drawCurrentTime(190, 10, true);
        } else if(currentScreen == SCREEN_DATA) {
            drawCurrentTime(190, 10, false);
        }
        lastTimeUpdate = millis();
    }
    
    // Resetear buffers al cambiar de pantalla
    static ScreenState lastScreen = currentScreen;
    if(lastScreen != currentScreen) {
        lastMainTime[0] = '\0';
        lastDataTime[0] = '\0';
        lastScreen = currentScreen;
    }

    // 8) If dataScreenActive, refresh T/H gauges
    static unsigned long lastDataRefresh = 0;
    if (dataScreenActive) {
      unsigned long now = millis();
      if (now - lastDataRefresh >= 1000) {
        lastDataRefresh = now;
        refreshDataScreen();
      }
    }
  }

  /*************************************************************
 * @brief Update simulated time every 60 seconds 
 *        if RTC is not available
 *************************************************************/
  void updateSimTime() {
    unsigned long now = millis();
    if (now - lastSimUpdate >= 60000UL) {
      lastSimUpdate += 60000UL;
      simMinute++;
      if (simMinute > 59) {
        simMinute = 0;
        simHour++;
        if (simHour > 23) {
          simHour = 0;
          simDay++;
          // Basic day/month
          if (simDay > 30) {
            simDay = 1;
            simMonth++;
            if (simMonth > 12) {
              simMonth = 1;
              simYear++;
            }
          }
        }
      }
    }
  }

  /*************************************************************
 * @brief Get current time from RTC or from simulation
 *************************************************************/
  void getCurrentTime(int& hour, int& minute) {
    if (rtcAvailable) {
      DateTime now = rtc.now();
      hour = now.hour();
      minute = now.minute();
    } else {
      hour = simHour;
      minute = simMinute;
    }
  }

  /*************************************************************
 * @brief Checks user-chosen OnTime/OffTime vs current 
 *        time (RTC or simulation) to turn on/off LED panels.
 *************************************************************/
  void checkLightsTimer() {
    // Solo ejecutar en modo automático
    if (autoModeActive) {
      // Si las luces están completamente apagadas o el temporizador está deshabilitado, apagar los LEDs
      if (lightsCompletelyOff || !lightsTimerEnabled) {
        digitalWrite(LED_PANEL_PIN1, LOW);
        digitalWrite(LED_PANEL_PIN2, LOW);
        return;
      }

      // Obtener la hora actual
      int currentHour, currentMinute;
      getCurrentTime(currentHour, currentMinute);

      // Calcular si las luces deben estar encendidas
      int nowMins = currentHour * 60 + currentMinute;
      int onMins = lightOnHour * 60 + lightOnMinute;
      int offMins = lightOffHour * 60 + lightOffMinute;

      bool turnOn = false;
      if (onMins < offMins) {
        if (nowMins >= onMins && nowMins < offMins) turnOn = true;
      } else if (onMins > offMins) {
        if (nowMins >= onMins || nowMins < offMins) turnOn = true;
      } else {
        turnOn = true;  // Si on == off, las luces están siempre encendidas
      }

      // Encender o apagar los LEDs según la intensidad
      if (turnOn) {
        digitalWrite(LED_PANEL_PIN1, HIGH);
        digitalWrite(LED_PANEL_PIN2, (lightIntensity == 100) ? HIGH : LOW);
      } else {
        digitalWrite(LED_PANEL_PIN1, LOW);
        digitalWrite(LED_PANEL_PIN2, LOW);
      }
    }
  }

  /*************************************************************
 *                FUNCTION DEFINITIONS (UI)
 *************************************************************/
  /**
 * @brief Draw the main screen (static elements).
 *        This is the default home screen with "Auto", "Menu", "Manual" buttons,
 *        and the device status on the left side.
 */
  void drawMainScreenStatic() {
    currentScreen = MAIN_SCREEN;
    tft.fillScreen(TFT_BLACK);

    drawCurrentTime(190, 10, true); 

    // Draw bottom buttons
    drawButtonWithFeedback(10, 190, 120, 30, "Auto", autoModeActive ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(140, 190, 120, 30, "Menu", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(270, 190, 120, 30, "Manual", !autoModeActive ? TFT_BLUE : TFT_BROWN, TFT_WHITE);

    // Reset alarm states for dynamic re-check
    waterAlarmLastState = !waterAlarmActive;
    tempAlarmLastState = !tempAlarmActive;
    humidityAlarmLastState = !humidityAlarmActive;

    // Update device status and check alarms once
    updateDeviceStatusDisplay();
    checkAlarms();
  }

void drawCurrentTime(int x, int y, bool isMainScreen) {
    DateTime now;
    char currentTime[6];
    
    if(rtcAvailable) {
        now = rtc.now();
    } else {
        now = DateTime(simYear, simMonth, simDay, simHour, simMinute, 0);
    }
    
    sprintf(currentTime, "%02d:%02d", now.hour(), now.minute());
    
    char* lastTime = isMainScreen ? lastMainTime : lastDataTime;
    
    if(strcmp(currentTime, lastTime) != 0) {
        tft.setTextColor(TFT_BLACK);
        tft.setTextSize(2);
        tft.setCursor(x, y);
        tft.print(lastTime);
        
        tft.setTextColor(TFT_CYAN);
        tft.setCursor(x, y);
        tft.print(currentTime);
        
        strcpy(lastTime, currentTime);
    }
}

  /**
 * @brief Draw the real-time data screen with gauges for temperature, humidity,
 *        and a pH bar. Also shows active alarms if any.
 */
  void drawDataScreen() {
    // Draw static elements only once
    tft.fillScreen(TFT_BLACK);
    drawCurrentTime(190, 10, false);

    // Draw static titles and gauges
    drawTemperatureGauge(80, 100, 60, temperature);
    drawHumidityGauge(320, 100, 60, humidity);

    // Draw pH bar
    float pHValue = getCalibratedPH();
    if (isnan(pHValue)) {
      drawPHBar(200, 220, 240, 30, NAN);
    } else {
      drawPHBar(200, 220, 240, 30, pHValue);
    }

    // Show active alarms if any
    drawActiveAlarms();
  }

  /**
 * @brief Draw/refresh the 6 status "pilots" in two columns (like an open book).
 *        - First column (left): Mat, Hum, Fan
 *        - Second column (right): UV, LED1, LED2
 *        - Each row has the same y-coordinates, so they start from the top.
 *        - There's a 10 px horizontal gap between the two columns.
 * 
 *        All comments are in English so you can identify and 
 *        modify them later if needed.
 */
  void updateDeviceStatusDisplay() {
    // Width and height for each pilot "button"
    int pilotWidth = 80;
    int pilotHeight = 25;

    // Vertical spacing between rows
    // (the difference in Y for the next pilot in the same column)
    int verticalSpacing = pilotHeight + 10;  // e.g. 25 + 10 = 35 px

    // X-position for the left column
    int xLeft = 10;

    // X-position for the right column, adding pilotWidth + 10 px gap
    int xRight = xLeft + pilotWidth + 10;  // = 10 + 80 + 10 = 100

    // Starting Y for the first row (top row)
    int yStart = 60;

    // -------------------------------------------------
    //       LEFT COLUMN (Mat, Hum, Fan)
    // -------------------------------------------------

    // Row 1 (top) - MAT
    tft.fillRect(xLeft, yStart, pilotWidth, pilotHeight, TFT_BLACK);
    drawButtonWithFeedback(
      xLeft,
      yStart,
      pilotWidth,
      pilotHeight,
      "Mat",
      (mattOn ? TFT_GREEN : TFT_GRAY),
      TFT_BLACK);

    // Row 2 - HUM
    tft.fillRect(xLeft, yStart + verticalSpacing, pilotWidth, pilotHeight, TFT_BLACK);
    drawButtonWithFeedback(
      xLeft,
      yStart + verticalSpacing,
      pilotWidth,
      pilotHeight,
      "Hum",
      (humidifierOn ? TFT_GREEN : TFT_GRAY),
      TFT_BLACK);

    // Row 3 - FAN
    tft.fillRect(xLeft, yStart + (2 * verticalSpacing), pilotWidth, pilotHeight, TFT_BLACK);
    drawButtonWithFeedback(
      xLeft,
      yStart + (2 * verticalSpacing),
      pilotWidth,
      pilotHeight,
      "Fan",
      (fanOnManual ? TFT_GREEN : TFT_GRAY),
      TFT_BLACK);

    // -------------------------------------------------
    //       RIGHT COLUMN (UV, LED1, LED2)
    // -------------------------------------------------
    // The Y coordinates are the same for each row,
    // but X is offset by xRight instead of xLeft.

    // Row 1 (top) - UV
    tft.fillRect(xRight, yStart, pilotWidth, pilotHeight, TFT_BLACK);
    drawButtonWithFeedback(
      xRight,
      yStart,
      pilotWidth,
      pilotHeight,
      "UV",
      (uvLightOn ? TFT_GREEN : TFT_GRAY),
      TFT_BLACK);

    // Row 2 - LED1
    tft.fillRect(xRight, yStart + verticalSpacing, pilotWidth, pilotHeight, TFT_BLACK);
    drawButtonWithFeedback(
      xRight,
      yStart + verticalSpacing,
      pilotWidth,
      pilotHeight,
      "LED1",
      (oldLed1On ? TFT_GREEN : TFT_GRAY),
      TFT_BLACK);

    // Row 3 - LED2
    tft.fillRect(xRight, yStart + (2 * verticalSpacing), pilotWidth, pilotHeight, TFT_BLACK);
    drawButtonWithFeedback(
      xRight,
      yStart + (2 * verticalSpacing),
      pilotWidth,
      pilotHeight,
      "LED2",
      (oldLed2On ? TFT_GREEN : TFT_GRAY),
      TFT_BLACK);
  }




  /**
 * @brief Draw the main menu screen with several configuration options.
 */
  void drawMenu() {
    currentScreen = MENU_SCREEN;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Main menu");


    // Main menu buttons
    drawButtonWithFeedback(20, 40, 160, 40, "Set Auto", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 40, 160, 40, "Set Manual", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 100, 160, 40, "pH Calib", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 100, 160, 40, "SD Card", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 160, 160, 40, "Set Alarm", TFT_BROWN, TFT_WHITE);
  }


  /**
 * @brief Draw the automatic settings screen (Set Auto).
 */
  void drawAutoSettings() {
    currentScreen = AUTO_SETTINGS;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Auto Settings");

    // Auto settings menu
    drawButtonWithFeedback(20, 40, 160, 40, "Set Temp", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 40, 160, 40, "Set Hum", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 100, 160, 40, "Fan Mode", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 100, 160, 40, "Set Light", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 160, 160, 40, "Set Clock", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
  }

  /**
 * @brief Draw a simplified "Set Clock" screen with big digits for date/time.
 *        We'll have:
 *         - The date/time in large font at the top (DD/MM/YYYY HH:MM)
 *         - A "Set"/"OK" button that cycles through digits
 *         - "+" and "-" buttons to change the current digit
 *         - "Back" button at the bottom-left
 */
  void drawSetClockScreen() {
    currentScreen = SET_CLOCK_SCREEN;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Set Clock (Digits)");

    // If not editing, we reset currentDigitIndex=0
    editingDigits = false;
    currentDigitIndex = 0;
    drawClockDigits();
    // Load from RTC or simulation
    if (rtcAvailable) {
      DateTime now = rtc.now();
      tempClockDay = now.day();
      tempClockMonth = now.month();
      tempClockYear = now.year();
      tempClockHour = now.hour();
      tempClockMinute = now.minute();
    } else {
      tempClockDay = simDay;
      tempClockMonth = simMonth;
      tempClockYear = simYear;
      tempClockHour = simHour;
      tempClockMinute = simMinute;
    }

    // Draw big date/time at top
    drawClockDigits();

    // Botones de 140x40 como en otras pantallas
    drawButtonWithFeedback(20, 130, 140, 40, "Set", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 180, 140, 40, "Back", TFT_BROWN, TFT_WHITE);

    // Botones +/- más separados (100px entre ellos)
    drawButtonWithFeedback(200, 130, 40, 40, "+", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(300, 130, 40, 40, "-", TFT_BROWN, TFT_WHITE);
  }

  /**
 * @brief Draw the big digits for DD/MM/YYYY HH:MM at top, 
 *        highlighting the digit at currentDigitIndex if editingDigits==true.
 */
  void drawClockDigits() {
    char buffer[20];
    sprintf(buffer, "%02d/%02d/%04d %02d:%02d",
            tempClockDay, tempClockMonth, tempClockYear,
            tempClockHour, tempClockMinute);

    tft.fillRect(10, 50, 400, 60, TFT_BLACK);
    tft.setTextSize(3);
    tft.setCursor(10, 50);

    // Definición exacta de grupos y sus tamaños
    const int highlightGroups[5][4] = {
      { 0, 1, -1, -1 },    // DD (2 dígitos)
      { 3, 4, -1, -1 },    // MM (2 dígitos)
      { 6, 7, 8, 9 },      // YYYY (4 dígitos)
      { 11, 12, -1, -1 },  // HH (2 dígitos)
      { 14, 15, -1, -1 }   // MM (2 dígitos)
    };

    const byte groupSizes[5] = { 2, 2, 4, 2, 2 };  // Tamaño de cada grupo

    for (int i = 0; i < strlen(buffer); i++) {
      bool isHighlighted = false;

      if (editingDigits) {
        // Solo verificar las posiciones válidas del grupo
        for (int j = 0; j < groupSizes[currentDigitIndex]; j++) {
          if (i == highlightGroups[currentDigitIndex][j]) {
            isHighlighted = true;
            break;
          }
        }
      }

      tft.setTextColor(isHighlighted ? TFT_BLUE : TFT_WHITE);
      tft.print(buffer[i]);
    }
  }

  void incrementCurrentDigit(bool plus) {
    int step = plus ? 1 : -1;

    switch (currentDigitIndex) {
      case 0:  // Día
        tempClockDay = constrain(tempClockDay + step, 1, 31);
        break;
      case 1:  // Mes
        tempClockMonth = constrain(tempClockMonth + step, 1, 12);
        break;
      case 2:  // Año
        tempClockYear = constrain(tempClockYear + step, 2025, 2100);
        break;
      case 3:  // Hora
        tempClockHour = constrain(tempClockHour + step, 0, 23);
        break;
      case 4:  // Minutos
        tempClockMinute = constrain(tempClockMinute + step, 0, 59);
        break;
    }
  }


  void handleSetClockTouch(int x, int y) {
    // Botón Back
    if (x > 20 && x < 160 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 140, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(50);
      drawButtonWithFeedback(20, 180, 140, 40, "Back", TFT_BROWN, TFT_WHITE, false);

      // Guardar cambios
      if (rtcAvailable) {
        rtc.adjust(DateTime(tempClockYear, tempClockMonth, tempClockDay,
                            tempClockHour, tempClockMinute, 0));
      }
      drawAutoSettings();
    }
    // Botón Set/OK
    else if (x > 20 && x < 160 && y > 130 && y < 170) {
      // Feedback táctil mejorado
      drawButtonWithFeedback(20, 130, 140, 40, "", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButtonWithFeedback(20, 130, 140, 40, editingDigits ? "OK" : "Set",
                             TFT_BROWN, TFT_WHITE, false);

      if (!editingDigits) {
        editingDigits = true;
        currentDigitIndex = 0;
        drawClockDigits();  // Resaltar inmediatamente
      } else {
        // Navegación mejorada entre grupos
        currentDigitIndex++;
        if (currentDigitIndex > 4) {  // 0-4 grupos
          editingDigits = false;
          currentDigitIndex = 0;
        }
        drawClockDigits();
      }

      // Cambio inmediato del label del botón
      tft.fillRect(20, 130, 140, 40, TFT_BLACK);
      drawButtonWithFeedback(20, 130, 140, 40,
                             editingDigits ? "OK" : "Set", TFT_BROWN, TFT_WHITE);
    }

    // Botones +/- con posiciones ajustadas
    else if (x > 200 && x < 240 && y > 130 && y < 170) {  // +
      drawButtonWithFeedback(200, 130, 40, 40, "+", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      drawButtonWithFeedback(200, 130, 40, 40, "+", TFT_BROWN, TFT_WHITE, false);

      if (editingDigits) {
        incrementCurrentDigit(true);
        drawClockDigits();
      }
    } else if (x > 320 && x < 360 && y > 130 && y < 170) {  // -
      drawButtonWithFeedback(300, 130, 40, 40, "-", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      drawButtonWithFeedback(300, 130, 40, 40, "-", TFT_BROWN, TFT_WHITE, false);

      if (editingDigits) {
        incrementCurrentDigit(false);
        drawClockDigits();
      }
    }
  }


  /**
 * @brief Shows the updated clock values (Year,Month,Day,Hour,Min) on screen.
 *        Call this after user increments or decrements something.
 */
  void refreshSetClockValues() {
    // We'll place them next to each label, or in a fixed area

    // Clear the old text area
    // For example, year near x=20, y=70
    tft.fillRect(80, 50, 80, 20, TFT_BLACK);
    tft.setCursor(80, 50);
    tft.setTextColor(TFT_YELLOW);
    tft.setTextSize(2);
    tft.print(tempClockYear);

    // Month
    tft.fillRect(80, 100, 80, 20, TFT_BLACK);
    tft.setCursor(80, 100);
    tft.print(tempClockMonth);

    // Day
    tft.fillRect(80, 150, 80, 20, TFT_BLACK);
    tft.setCursor(80, 150);
    tft.print(tempClockDay);

    // Hour
    tft.fillRect(300, 50, 60, 20, TFT_BLACK);
    tft.setCursor(300, 50);
    tft.print(tempClockHour);

    // Minute
    tft.fillRect(300, 100, 60, 20, TFT_BLACK);
    tft.setCursor(300, 100);
    tft.print(tempClockMinute);
  }



  /**
 * @brief Draw the manual settings screen (Set Manual).
 *        Here, the user can tap to toggle devices ON/OFF only if autoModeActive == false.
 */
  void drawManualSettings() {
    currentScreen = MANUAL_SETTINGS;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Manual Settings");

    drawButtonWithFeedback(20, 40, 160, 40, "Matt", mattOn ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 40, 160, 40, "Humidifier", humidifierOn ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 100, 160, 40, "UV Light", uvLightOn ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 100, 160, 40, "Fan", fanOnManual ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 160, 160, 40, "LEDS", ledsManualOn ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
  }

  /**
 * @brief Draw the screen to set or adjust alarm thresholds (temperature/humidity).
 */
  void drawAlarmSettings() {
    currentScreen = ALARM_SETTINGS;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Set Alarm Values");

    drawButtonWithFeedback(20, 40, 160, 40, "Set Temp", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 40, 160, 40, "Set Hum", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
  }

  /**
 * @brief Display active alarms in the data screen if needed.
 */
  void drawActiveAlarms() {
    // Clear possible alarm areas
    tft.fillRect(162, 50, 80, 30, TFT_BLACK);
    tft.fillRect(162, 90, 80, 30, TFT_BLACK);
    tft.fillRect(162, 130, 80, 30, TFT_BLACK);

    // Show alarm buttons if active
    if (waterAlarmActive) {
      drawButtonWithFeedback(162, 90, 80, 30, "Water", TFT_RED, TFT_YELLOW);
    }
    if (tempAlarmActive) {
      drawButtonWithFeedback(162, 130, 80, 30, "T High", TFT_RED, TFT_YELLOW);
    }
    if (humidityAlarmActive) {
      drawButtonWithFeedback(162, 50, 80, 30, "H High", TFT_RED, TFT_YELLOW);
    }
  }

  /**
 * @brief Draw the pH calibration screen: shows windows for pH7, pH4 calibration
 *        and a button to start/confirm calibration steps.
 */
  void drawPhCalibrationScreen() {
    currentScreen = PH_CALIBRATION;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 10);
    tft.print("pH Calibration");

    // Windows to show pH7 and pH4 values
    drawValueWindow(20, 40, 150, 40, pH7Value);
    drawValueWindow(220, 40, 150, 40, pH4Value);

    // Instructions
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.fillRect(20, 90, 360, 40, TFT_BLACK);
    if (!calibratingPH7 && !calibratingPH4) {
      // Not calibrating yet
      tft.setCursor(20, 100);
      tft.print("Press 'Calibrate' to start.");
    } else if (calibratingPH7) {
      tft.setCursor(20, 100);
      tft.print("Insert probe in pH7");
    } else if (calibratingPH4) {
      tft.setCursor(20, 100);
      tft.print("Insert probe in pH4");
    }

    // Buttons (Back + Calibrate/Confirm)
    drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
    if (!calibratingPH7 && !calibratingPH4) {
      drawButtonWithFeedback(220, 180, 160, 40, "Calibrate", TFT_BROWN, TFT_WHITE);
    } else {
      tft.fillRoundRect(220, 180, 160, 40, 5, TFT_GREEN);
      tft.drawRoundRect(220, 180, 160, 40, 5, TFT_WHITE);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize(2);
      tft.setCursor(260, 191);
      tft.print("Confirm");
    }
  }

  /**
 * @brief Draw the "Fan Mode" settings screen (Full Auto / Semi Auto).
 */
  void drawFanModeSettings() {
    currentScreen = FAN_MODE_SETTINGS;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 10);
    tft.print("Fan Mode");

    // Mode selection buttons
    drawButtonWithFeedback(20, 40, 160, 40, "Full Auto", fanModeFullAuto ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 40, 160, 40, "Semi Auto", fanModeSemiAuto ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 100, 160, 40, "Fan Time", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
  }

  /**
 * @brief Draw the screen to set the fan ON/OFF times (Semi Auto).
 */
  void drawFanTimeSettings() {
    currentScreen = FAN_TIME_SETTINGS;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 10);
    tft.print("Set Fan Time");

    // ON time
    tft.setCursor(20, 60);
    tft.print("Fan ON Time: ");
    drawFanTimeValue(240, 60, fanOnTime);

    // OFF time
    tft.setCursor(20, 120);
    tft.print("Fan OFF Time: ");
    drawFanTimeValue(240, 120, fanOffTime);

    // Buttons for increment/decrement
    drawButton(310, 60, 40, 40, "+", TFT_BROWN, TFT_WHITE);
    drawButton(360, 60, 40, 40, "-", TFT_BROWN, TFT_WHITE);
    drawButton(310, 120, 40, 40, "+", TFT_BROWN, TFT_WHITE);
    drawButton(360, 120, 40, 40, "-", TFT_BROWN, TFT_WHITE);

    // Back button
    drawButton(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
  }

  /*************************************************************
 *                LIGHT SETTINGS SCREEN
 *************************************************************/
  /**
 * @brief Draws the "Light Settings" screen with:
 *        - A button "Time" + the current hours next to it.
 *        - A button "Intensity" + the current % next to it.
 *        - A "Back" button to return to Set Auto.
 */
  /**
 * @brief "Set Lights" screen with 5 buttons:
 *        [OnTime], [OffTime], [50%], [100%], [Off], [Back].
 *        Actually 6, counting "Back".
 */
  void drawLightSettingsScreen() {
    currentScreen = LIGHT_SETTINGS;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(20, 10);
    tft.print("Set Lights");

    // 1) OnTime
    drawButtonWithFeedback(20, 60, 140, 40, "OnTime", TFT_BROWN, TFT_WHITE);
    // 2) OffTime
    drawButtonWithFeedback(180, 60, 140, 40, "OffTime", TFT_BROWN, TFT_WHITE);

    // 3) 50%
    // We check if currently lightIntensity==50 and not "off"
    // We might store a boolean "lightsOffMode" => if it's true => "Off" is selected.
    // For now assume we just compare if lightIntensity==50
    uint16_t color50 = (lightIntensity == 50 && !lightsTimerEnabled) ? TFT_BLUE : TFT_BROWN;
    // Actually depends if you consider "lightsTimerEnabled" or another boolean for "off".
    // We'll define a separate boolean: lightsCompletelyOff.
    // For clarity, let's define: if lightsCompletelyOff==true => "Off" is blue,
    // else if lightIntensity==50 => "50%" is blue, else if 100 => "100%" is blue.

    drawButtonWithFeedback(20, 120, 80, 40, "50%",  // narrower
                           ((!lightsCompletelyOff) && (lightIntensity == 50)) ? TFT_BLUE : TFT_BROWN,
                           TFT_WHITE);

    // 4) 100%
    drawButtonWithFeedback(110, 120, 80, 40, "100%",
                           ((!lightsCompletelyOff) && (lightIntensity == 100)) ? TFT_BLUE : TFT_BROWN,
                           TFT_WHITE);

    // 5) Off
    drawButtonWithFeedback(200, 120, 80, 40, "Off",
                           (lightsCompletelyOff ? TFT_BLUE : TFT_BROWN),
                           TFT_WHITE);

    // 6) Back
    drawButtonWithFeedback(20, 180, 140, 40, "Back", TFT_BROWN, TFT_WHITE);
  }



  /**
 * @brief Redraw only the "Intensity" button on the Light Settings screen.
 *        Uses the global variable lightIntensity.
 */
  void drawLightIntensityButton() {
    // Coordinates should match the original button in drawLightSettingsScreen
    int x = 20, y = 120, w = 200, h = 40;

    String intensityLabel = "Intensity: " + String(lightIntensity) + "%";

    tft.fillRect(x, y, w, h, TFT_BLACK);  // clear old content
    drawButtonWithFeedback(x, y, w, h, intensityLabel.c_str(), TFT_BROWN, TFT_WHITE);
  }


  /**
 * @brief Draw an adjustable parameter screen (for temperature/humidity).
 * @param parameterName The label to display at the top.
 * @param currentValue  The current numeric value.
 */
  void drawParameterAdjustScreen(const char* parameterName, float currentValue) {
    currentScreen = PARAMETER_ADJUST;
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print(parameterName);

    // Show the current value in large font
    tft.fillRect(20, 60, 180, 40, TFT_BLACK);
    tft.setTextSize(4);
    tft.setCursor(20, 60);
    tft.print(currentValue, 1);

    // Add the proper symbol
    if (currentParameter == TEMP) {
      tft.print((char)247);  // degree symbol
      tft.print("C");
    } else if (currentParameter == HUMIDITY) {
      tft.print(" %");
    }

    // Buttons for increment/decrement
    drawButtonWithFeedback(240, 60, 60, 40, "+", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(320, 60, 60, 40, "-", TFT_BROWN, TFT_WHITE);

    // Back button
    drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
  }

  /*************************************************************
 *               ADDITIONAL UI DRAWING FUNCTIONS
 *************************************************************/
  void drawValueWindow(int x, int y, int w, int h, float value) {
    tft.drawRect(x, y, w, h, TFT_WHITE);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(x + 10, y + 10);
    if (value != 0.0) {
      tft.print(value, 2);
    } else {
      tft.print("N/A");
    }
  }

  void drawButtonWithFeedback(int x, int y, int w, int h, const char* label, uint16_t color, uint16_t textColor, bool pressed) {
    // If pressed, we show a different color (blue feedback) briefly
    uint16_t fillColor = pressed ? TFT_BLUE : color;
    tft.fillRoundRect(x, y, w, h, 5, fillColor);
    tft.drawRoundRect(x, y, w, h, 5, TFT_WHITE);
    tft.setTextColor(textColor);
    tft.setTextSize(2);

    int16_t textLength = strlen(label);
    int16_t textWidth = textLength * 12;
    int16_t textHeight = 16;
    tft.setCursor(x + (w - textWidth) / 2, y + (h - textHeight) / 2);
    tft.print(label);
  }

  void drawButton(int x, int y, int w, int h, const char* label, uint16_t color, uint16_t textColor) {
    tft.fillRoundRect(x, y, w, h, 5, color);
    tft.drawRoundRect(x, y, w, h, 5, TFT_WHITE);
    tft.setTextColor(textColor);
    tft.setTextSize(2);

    int16_t textLength = strlen(label);
    int16_t textWidth = textLength * 12;
    int16_t textHeight = 16;
    tft.setCursor(x + (w - textWidth) / 2, y + (h - textHeight) / 2);
    tft.print(label);
  }


  /*************************************************************
 *                     GAUGES & METER DRAW
 *************************************************************/
  /**
 * @brief Draw the temperature gauge with a centered numeric value.
 *        The numeric value includes 1 decimal point and is displayed in the center of the gauge.
 * @param x The x-coordinate of the center of the gauge.
 * @param y The y-coordinate of the center of the gauge.
 * @param radius The radius of the circular gauge.
 * @param value The current temperature value to display.
 */
  void drawTemperatureGauge(int x, int y, int radius, float value) {
    // Title for the gauge
    tft.setTextColor(TFT_ORANGE);
    tft.setTextSize(2);
    const char* title = "TEMPERATURE";
    int textWidthTemp = strlen(title) * 12;                   // Approximate text width
    tft.setCursor(x - (textWidthTemp / 2), y - radius - 32);  // Center title above gauge
    tft.print(title);

    // Draw the outer circular gauge
    drawCircularGaugeWithGradient(x, y, radius + 10, value, 60, 15, TFT_BLUE, TFT_GREEN, TFT_RED);

    // Prepare the numeric value with 1 decimal
    String tempLabel = String(value, 1) + String((char)247) + "C";  // "°C"
    int textWidth = tempLabel.length() * 12;                        // Approximate text width for centering

    // Draw the numeric value in the center of the gauge
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    if (isnan(value)) {
      // If value is invalid, display "N/A"
      const char* invalidText = "N/A";
      textWidth = strlen(invalidText) * 12;
      tft.setCursor(x - textWidth / 2, y - 8);  // Center text
      tft.print(invalidText);
    } else {
      // If value is valid, display the formatted temperature
      tft.setCursor(x - textWidth / 2, y - 8);  // Center text
      tft.print(tempLabel);
    }

    // Draw a red arrow indicating the target temperature
    float tempAlarmValue = targetTemperature;
    float angle = map(tempAlarmValue, 15, 60, -135, 135);
    int arrowX = x + (int)(radius * cos(radians(angle)));
    int arrowY = y + (int)(radius * sin(radians(angle)));
    tft.fillTriangle(arrowX, arrowY, arrowX - 5, arrowY + 10, arrowX + 5, arrowY + 10, TFT_RED);
  }

  /**
 * @brief Draw the humidity gauge with a centered numeric value.
 *        The numeric value includes 1 decimal point and is displayed in the center of the gauge.
 * @param x The x-coordinate of the center of the gauge.
 * @param y The y-coordinate of the center of the gauge.
 * @param radius The radius of the circular gauge.
 * @param value The current humidity value to display.
 */
  void drawHumidityGauge(int x, int y, int radius, float value) {
    // Title for the gauge
    tft.setTextColor(TFT_BLUE);
    tft.setTextSize(2);
    const char* title = "HUMIDITY";
    int textWidthHum = strlen(title) * 12;                   // Approximate text width
    tft.setCursor(x - (textWidthHum / 2), y - radius - 32);  // Center title above gauge
    tft.print(title);

    // Draw the outer circular gauge
    drawCircularGaugeWithGradient(x, y, radius + 10, value, 100, 30, TFT_BLUE, TFT_GREEN, TFT_RED);

    // Prepare the numeric value with 1 decimal
    String humLabel = String(value, 1) + " %";  // "50.0 %"
    int textWidth = humLabel.length() * 12;     // Approximate text width for centering

    // Draw the numeric value in the center of the gauge
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    if (isnan(value)) {
      // If value is invalid, display "N/A"
      const char* invalidText = "N/A";
      textWidth = strlen(invalidText) * 12;
      tft.setCursor(x - textWidth / 2, y - 8);  // Center text
      tft.print(invalidText);
    } else {
      // If value is valid, display the formatted humidity
      tft.setCursor(x - textWidth / 2, y - 8);  // Center text
      tft.print(humLabel);
    }

    // Draw a red arrow indicating the target humidity
    float humAlarmValue = targetHumidity;
    float angle = map(humAlarmValue, 30, 100, -135, 135);
    int arrowX = x + (int)(radius * cos(radians(angle)));
    int arrowY = y + (int)(radius * sin(radians(angle)));
    tft.fillTriangle(arrowX, arrowY, arrowX - 5, arrowY + 10, arrowX + 5, arrowY + 10, TFT_RED);
  }



  /*************************************************************
 * @brief Periodically called while dataScreenActive == true.
 *        Redraws the temperature and humidity gauges if needed
 *        so their color/gradient reflects new T/H values.
 *************************************************************/
  void refreshDataScreen() {
    // We keep track of the last displayed T/H to see if there's a significant change
    static float lastTempDisplayed = NAN;
    static float lastHumDisplayed = NAN;

    const float tempThreshold = 0.2f;  // e.g. only redraw if ±0.2°C difference
    const float humThreshold = 1.0f;   // e.g. only redraw if ±1% RH difference

    bool redrawTemp = false;
    bool redrawHum = false;

    // Check temperature difference
    if (!isnan(temperature)) {
      if (isnan(lastTempDisplayed) || fabs(temperature - lastTempDisplayed) >= tempThreshold) {
        redrawTemp = true;
      }
    }

    // Check humidity difference
    if (!isnan(humidity)) {
      if (isnan(lastHumDisplayed) || fabs(humidity - lastHumDisplayed) >= humThreshold) {
        redrawHum = true;
      }
    }

    // If we need to redraw temperature gauge
    if (redrawTemp) {
      // Erase the old gauge area (a rectangle that covers the circle + margin).
      // Adjust the rectangle to match the gauge location/size.
      tft.fillRect(80 - 70, 100 - 70, 140, 140, TFT_BLACK);

      // Now draw the full gauge with the new temperature
      drawTemperatureGauge(80, 100, 60, temperature);
      lastTempDisplayed = temperature;
    }

    // If we need to redraw humidity gauge
    if (redrawHum) {
      // Erase the old gauge area
      tft.fillRect(320 - 70, 100 - 70, 140, 140, TFT_BLACK);

      // Now draw the full gauge with the new humidity
      drawHumidityGauge(320, 100, 60, humidity);
      lastHumDisplayed = humidity;
    }
  }


  /**
 * @brief Draw a pH bar with a gradient from yellow to blue.
 *        If pHValue is out of range (NAN), shows "N/A".
 */
  void drawPHBar(int x, int y, int width, int height, float pHValue) {
    // Label
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(x - width / 2 - 40, y - height / 2);
    tft.print("pH");

    // Gradient fill
    for (int i = 0; i < width; i++) {
      float ratio = (float)i / width;
      uint16_t color = interpolateColor(TFT_YELLOW, TFT_BLUE, ratio);
      tft.drawFastVLine(x - width / 2 + i, y - height / 2, height, color);
    }

    // Numeric value
    tft.setTextSize(2);
    tft.setCursor(x + width / 2 + 10, y - height / 2);
    if (isnan(pHValue)) {
      tft.print("N/A");
    } else {
      tft.print(pHValue, 1);
    }

    // Red arrow indicating the pH position
    if (!isnan(pHValue)) {
      int arrowX = x - width / 2 + (int)((pHValue / 14.0) * width);
      int arrowY = y - height / 2 - 10;
      tft.fillTriangle(arrowX, arrowY, arrowX - 5, arrowY - 10, arrowX + 5, arrowY - 10, TFT_RED);
    }
  }

  /**
 * @brief Draw a segmented circular gauge with color gradient from startColor -> midColor -> endColor.
 * @param value    Current value to represent.
 * @param maxValue The maximum scale value.
 * @param minValue The minimum scale value.
 */
  void drawCircularGaugeWithGradient(int x, int y, int radius, float value, float maxValue, float minValue,
                                     uint16_t startColor, uint16_t midColor, uint16_t endColor) {
    int segments = 60;
    float angleStep = 360.0 / segments;
    float filledSegments = (value - minValue) / (maxValue - minValue) * segments;

    for (int i = 0; i < segments; i++) {
      float angle = i * angleStep;
      int x0 = x + radius * cos(radians(angle));
      int y0 = y + radius * sin(radians(angle));

      // The gauge line length is longer for filled segments
      int lineLength = (i < filledSegments) ? 8 : 4;
      int x1 = x + (radius - lineLength) * cos(radians(angle));
      int y1 = y + (radius - lineLength) * sin(radians(angle));

      // Determine segment color with interpolation
      uint16_t color;
      if (i < segments / 2) {
        color = interpolateColor(startColor, midColor, (float)i / (segments / 2));
      } else {
        color = interpolateColor(midColor, endColor, (float)(i - segments / 2) / (segments / 2));
      }

      if (i < filledSegments) {
        tft.drawLine(x0, y0, x1, y1, color);
      } else {
        tft.drawLine(x0, y0, x1, y1, TFT_WHITE);
      }
    }
  }

  /**
 * @brief Interpolates between two colors with a fraction in [0..1].
 */
  uint16_t interpolateColor(uint16_t color1, uint16_t color2, float fraction) {
    uint8_t r1 = (color1 >> 11) & 0x1F;
    uint8_t g1 = (color1 >> 5) & 0x3F;
    uint8_t b1 = color1 & 0x1F;
    uint8_t r2 = (color2 >> 11) & 0x1F;
    uint8_t g2 = (color2 >> 5) & 0x3F;
    uint8_t b2 = color2 & 0x1F;

    uint8_t r = r1 + (r2 - r1) * fraction;
    uint8_t g = g1 + (g2 - g1) * fraction;
    uint8_t b = b1 + (b2 - b1) * fraction;

    return (r << 11) | (g << 5) | b;
  }

  /*************************************************************
 *                     HANDLE TOUCH EVENTS
 *************************************************************/
  void handleTouch(int x, int y) {
    switch (currentScreen) {
      case MAIN_SCREEN: handleMainScreenTouch(x, y); break;
      case MENU_SCREEN: handleMenuTouch(x, y); break;
      case AUTO_SETTINGS: handleAutoSettingsTouch(x, y); break;
      case MANUAL_SETTINGS: handleManualSettingsTouch(x, y); break;
      case PH_CALIBRATION: handlePhCalibrationTouch(x, y); break;
      case PARAMETER_ADJUST: handleParameterAdjustTouch(x, y); break;
      case FAN_MODE_SETTINGS: handleFanModeSettingsTouch(x, y); break;
      case FAN_TIME_SETTINGS: handleFanTimeSettingsTouch(x, y); break;
      case ALARM_SETTINGS: handleAlarmSettingsTouch(x, y); break;
      case SD_CARD_MENU: handleSDCardMenuTouch(x, y); break;
      case LIGHT_SETTINGS: handleLightSettingsTouch(x, y); break;
      case SET_CLOCK_SCREEN: handleSetClockTouch(x, y); break;
      case SET_ON_TIME:
      case SET_OFF_TIME: handleOnOffTimeTouch(x, y); break;
    }
  }

  /**
 * @brief Handle touches on the Main Screen, where we have:
 *        - "Auto" button  (x:10..130,  y:190..220)
 *        - "Menu" button  (x:140..260, y:190..220)
 *        - "Manual" button(x:270..390, y:190..220)
 *
 * @param x The x-coordinate of the touch
 * @param y The y-coordinate of the touch
 */
  void handleMainScreenTouch(int x, int y) {
    // 1) Check if the user pressed "Auto" button
    if (x > 10 && x < 130 && y > 190 && y < 220) {
      if (!autoModeActive) {
        // We are switching from Manual to Auto
        autoModeActive = true;

        // Force all manual devices off:
        mattOn = false;
        humidifierOn = false;
        fanOnManual = false;
        uvLightOn = false;
        digitalWrite(HEATING_PAD_PIN, LOW);
        digitalWrite(HUMIDIFIER_PIN, LOW);
        digitalWrite(FAN_PIN, LOW);
        digitalWrite(UV_LIGHT_PIN, LOW);

        // Update the button visuals:
        // "Auto" => Blue
        drawButtonWithFeedback(10, 190, 120, 30, "Auto", TFT_BLUE, TFT_WHITE, true);
        // "Manual" => Brown
        drawButtonWithFeedback(270, 190, 120, 30, "Manual", TFT_BROWN, TFT_WHITE);

        // You could also call updatePilotsIfChanged() here
        // if you want to reflect the pilot states immediately.
        // Or you can rely on the loop calling autoModeControl().
        delay(100);
      }
    }
    // 2) Check if the user pressed "Menu" button
    else if (x > 140 && x < 260 && y > 190 && y < 220) {
      // Give feedback color for "Menu"
      drawButtonWithFeedback(140, 190, 120, 30, "Menu", TFT_BLUE, TFT_WHITE, true);
      delay(100);
      // Then draw the Menu screen
      drawMenu();
    }
    // 3) Check if the user pressed "Manual" button
    else if (x > 270 && x < 390 && y > 190 && y < 220) {
      if (autoModeActive) {
        // We are switching from Auto to Manual
        autoModeActive = false;

        // Turn everything off (auto devices):
        mattOn = false;
        humidifierOn = false;
        fanOnManual = false;  // We'll rely on user toggles in manual
        uvLightOn = false;
        digitalWrite(HEATING_PAD_PIN, LOW);
        digitalWrite(HUMIDIFIER_PIN, LOW);
        digitalWrite(FAN_PIN, LOW);
        digitalWrite(UV_LIGHT_PIN, LOW);

        // Update the button visuals:
        // "Auto" => Brown
        drawButtonWithFeedback(10, 190, 120, 30, "Auto", TFT_BROWN, TFT_WHITE);
        // "Manual" => Blue
        drawButtonWithFeedback(270, 190, 120, 30, "Manual", TFT_BLUE, TFT_WHITE, true);

        // Optional: call updatePilotsIfChanged() if you want immediate pilot refresh.
        delay(100);
      }
    }
  }


  /**
 * @brief Handle touches on the main menu screen.
 */
  void handleMenuTouch(int x, int y) {
    if (x > 20 && x < 180 && y > 40 && y < 80) {
      // "Set Auto"
      drawButtonWithFeedback(20, 40, 160, 40, "Set Auto", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawAutoSettings();
    } else if (x > 220 && x < 380 && y > 40 && y < 80) {
      // "Set Manual"
      drawButtonWithFeedback(220, 40, 160, 40, "Set Manual", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawManualSettings();
    } else if (x > 20 && x < 180 && y > 100 && y < 140) {
      // "pH Calib"
      drawButtonWithFeedback(20, 100, 160, 40, "pH Calib", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawPhCalibrationScreen();
    } else if (x > 220 && x < 380 && y > 100 && y < 140) {
      // "SD Card"
      drawButtonWithFeedback(220, 100, 160, 40, "SD Card", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawSDCardMenu();
    } else if (x > 220 && x < 380 && y > 160 && y < 200) {
      // "Set Alarm"
      drawButtonWithFeedback(220, 160, 160, 40, "Set Alarm", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawAlarmSettings();
    } else if (x > 20 && x < 180 && y > 180 && y < 220) {
      // "Back"
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawMainScreenStatic();
    }
  }

  /**
 * @brief Handle touches in the Auto Settings screen.
 */
  void handleAutoSettingsTouch(int x, int y) {
    // "Set Temp"
    if (x > 20 && x < 180 && y > 40 && y < 80) {
      drawButtonWithFeedback(20, 40, 160, 40, "Set Temp", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      currentParameter = TEMP;
      // Indicate we came from the Auto Settings
      parameterAdjustOrigin = FROM_AUTO_SETTINGS;
      drawParameterAdjustScreen("Set Temp", autoTemperatureSetpoint);
    }
    // "Set Hum"
    else if (x > 220 && x < 380 && y > 40 && y < 80) {
      drawButtonWithFeedback(220, 40, 160, 40, "Set Hum", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      currentParameter = HUMIDITY;
      // Indicate we came from the Auto Settings
      parameterAdjustOrigin = FROM_AUTO_SETTINGS;
      drawParameterAdjustScreen("Set Hum", autoHumiditySetpoint);
    }
    // "Fan Mode"
    else if (x > 20 && x < 180 && y > 100 && y < 140) {
      drawButtonWithFeedback(20, 100, 160, 40, "Fan Mode", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      drawFanModeSettings();
    }
    // "Set Light"
    else if (x > 220 && x < 380 && y > 100 && y < 140) {
      drawButtonWithFeedback(220, 100, 160, 40, "Set Light", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      drawLightSettingsScreen();  // <-- Nueva función
    }
    // "Set Clock"
    else if (x > 220 && x < 380 && y > 160 && y < 200) {
      drawButtonWithFeedback(220, 160, 160, 40, "Set Clock", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      drawSetClockScreen();
    }
    // "Back"
    else if (x > 20 && x < 180 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawMenu();
    }
  }

  /**
 * @brief Handle touches in the Manual Settings screen.
 *        IMPORTANT: The physical ON/OFF changes only occur if autoModeActive == false.
 */
  void handleManualSettingsTouch(int x, int y) {
    // Verificar si ha pasado el tiempo de debounce
    if (millis() - lastButtonPressTime < debounceDelay) {
      return;  // Ignorar la pulsación si no ha pasado el tiempo de debounce
    }

    // Actualizar el tiempo de la última pulsación válida
    lastButtonPressTime = millis();

    // "Matt"
    if (x > 20 && x < 180 && y > 40 && y < 80) {
      drawButtonWithFeedback(20, 40, 160, 40, "Matt", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      bool newVal = !mattOn;  // Toggle
      if (!autoModeActive) {
        mattOn = newVal;
        digitalWrite(HEATING_PAD_PIN, (newVal ? HIGH : LOW));
      }
      drawButtonWithFeedback(20, 40, 160, 40, "Matt", newVal ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    }
    // "Humidifier"
    else if (x > 220 && x < 380 && y > 40 && y < 80) {
      drawButtonWithFeedback(220, 40, 160, 40, "Humidifier", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      bool newVal = !humidifierOn;
      if (!autoModeActive) {
        humidifierOn = newVal;
        digitalWrite(HUMIDIFIER_PIN, (newVal ? HIGH : LOW));
      }
      drawButtonWithFeedback(220, 40, 160, 40, "Humidifier", newVal ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    }
    // "UV Light"
    else if (x > 20 && x < 180 && y > 100 && y < 140) {
      drawButtonWithFeedback(20, 100, 160, 40, "UV Light", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      bool newVal = !uvLightOn;
      if (!autoModeActive) {
        uvLightOn = newVal;
        digitalWrite(UV_LIGHT_PIN, (newVal ? HIGH : LOW));
      }
      drawButtonWithFeedback(20, 100, 160, 40, "UV Light", newVal ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    }
    // "Fan"
    else if (x > 220 && x < 380 && y > 100 && y < 140) {
      drawButtonWithFeedback(220, 100, 160, 40, "Fan", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      bool newVal = !fanOnManual;
      if (!autoModeActive) {
        fanOnManual = newVal;
        digitalWrite(FAN_PIN, (newVal ? HIGH : LOW));
      }
      drawButtonWithFeedback(220, 100, 160, 40, "Fan", newVal ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    }
    // "LEDS"
    else if (x > 220 && x < 380 && y > 160 && y < 200) {
      drawButtonWithFeedback(220, 160, 160, 40, "LEDS", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      bool newVal = !ledsManualOn;
      if (!autoModeActive) {
        ledsManualOn = newVal;
        // Encender o apagar los LEDs según la intensidad
        if (ledsManualOn) {
          digitalWrite(LED_PANEL_PIN1, HIGH);
          digitalWrite(LED_PANEL_PIN2, (lightIntensity == 100) ? HIGH : LOW);
        } else {
          digitalWrite(LED_PANEL_PIN1, LOW);
          digitalWrite(LED_PANEL_PIN2, LOW);
        }
      }
      drawButtonWithFeedback(220, 160, 160, 40, "LEDS", newVal ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    }
    // "Back"
    else if (x > 20 && x < 180 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawMenu();
    }
  }

  /**
 * @brief Handle touches in the Alarm Settings screen (Set Temp Alarm or Set Hum Alarm).
 */
  void handleAlarmSettingsTouch(int x, int y) {
    // "Set Temp"
    if (x > 20 && x < 180 && y > 40 && y < 80) {
      drawButtonWithFeedback(20, 40, 160, 40, "Set Temp", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      currentParameter = TEMP;
      // Indicate we came from the Alarm Settings
      parameterAdjustOrigin = FROM_ALARM_SETTINGS;
      drawParameterAdjustScreen("Set Temp Alarm", alarmTemperatureSetpoint);
    }
    // "Set Hum"
    else if (x > 220 && x < 380 && y > 40 && y < 80) {
      drawButtonWithFeedback(220, 40, 160, 40, "Set Hum", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      currentParameter = HUMIDITY;
      // Indicate we came from the Alarm Settings
      parameterAdjustOrigin = FROM_ALARM_SETTINGS;
      drawParameterAdjustScreen("Set Hum Alarm", alarmHumiditySetpoint);
    }
    // "Back"
    else if (x > 20 && x < 180 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(200);
      drawMenu();
    }
  }

  /**
 * @brief Handle touches in the pH Calibration screen.
 */
  void handlePhCalibrationTouch(int x, int y) {
    // "Confirm" or "Calibrate" button
    if (x > 220 && x < 380 && y > 180 && y < 220) {
      if (!calibratingPH7 && !calibratingPH4) {
        // Start calibration for pH7
        calibratingPH7 = true;
        // Redraw instructions
        tft.fillRect(20, 90, 360, 40, TFT_BLACK);
        tft.setCursor(20, 100);
        tft.print("Fit probe to pH7");
        // Change button to "Confirm"
        tft.fillRoundRect(220, 180, 160, 40, 5, TFT_GREEN);
        tft.drawRoundRect(220, 180, 160, 40, 5, TFT_WHITE);
        tft.setCursor(260, 191);
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE);
        tft.print("Confirm");
      } else if (calibratingPH7) {
        calibratePH7();
        calibratingPH7 = false;
        calibratingPH4 = true;
        // Update instructions
        tft.fillRect(20, 90, 360, 40, TFT_BLACK);
        tft.setCursor(20, 100);
        tft.setTextSize(2);
        tft.print("Fit probe to pH4");
      } else if (calibratingPH4) {
        calibratePH4();
        calibratingPH4 = false;
        calculatePHCalibration();
        // Redraw full screen to reset
        drawPhCalibrationScreen();
      }
    }
    // "Back" button
    else if (x > 20 && x < 180 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButton(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
      drawMenu();
    }
  }

  /**
 * @brief Handles touch events on the Parameter Adjust screen.
 *        Now it updates either the Auto setpoints or the Alarm setpoints 
 *        depending on parameterAdjustOrigin.
 */
  void handleParameterAdjustTouch(int x, int y) {
    bool valueChanged = false;

    // "+" button (x:240..300, y:60..100)
    if (x > 240 && x < 300 && y > 60 && y < 100) {
      drawButtonWithFeedback(240, 60, 60, 40, "+", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButtonWithFeedback(240, 60, 60, 40, "+", TFT_BROWN, TFT_WHITE);

      // Increase the correct variable
      if (parameterAdjustOrigin == FROM_AUTO_SETTINGS) {
        if (currentParameter == TEMP) {
          autoTemperatureSetpoint += 1.0;  // Update for display
          targetTemperature += 1.0;        // Update for auto mode
        } else if (currentParameter == HUMIDITY) {
          autoHumiditySetpoint += 1.0;  // Update for display
          targetHumidity += 1.0;        // Update for auto mode
        }
      } else if (parameterAdjustOrigin == FROM_ALARM_SETTINGS) {
        if (currentParameter == TEMP) {
          alarmTemperatureSetpoint += 1.0;  // Update alarm temperature
        } else if (currentParameter == HUMIDITY) {
          alarmHumiditySetpoint += 1.0;  // Update alarm humidity
        }
      }
      valueChanged = true;
    }
    // "-" button (x:320..380, y:60..100)
    else if (x > 320 && x < 380 && y > 60 && y < 100) {
      drawButtonWithFeedback(320, 60, 60, 40, "-", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButtonWithFeedback(320, 60, 60, 40, "-", TFT_BROWN, TFT_WHITE);

      // Decrease the correct variable
      if (parameterAdjustOrigin == FROM_AUTO_SETTINGS) {
        if (currentParameter == TEMP) {
          autoTemperatureSetpoint -= 1.0;  // Update for display
          targetTemperature -= 1.0;        // Update for auto mode
        } else if (currentParameter == HUMIDITY) {
          autoHumiditySetpoint -= 1.0;  // Update for display
          targetHumidity -= 1.0;        // Update for auto mode
        }
      } else if (parameterAdjustOrigin == FROM_ALARM_SETTINGS) {
        if (currentParameter == TEMP) {
          alarmTemperatureSetpoint -= 1.0;  // Update alarm temperature
        } else if (currentParameter == HUMIDITY) {
          alarmHumiditySetpoint -= 1.0;  // Update alarm humidity
        }
      }
      valueChanged = true;
    }
    // "Back" button (x:20..180, y:180..220)
    else if (x > 20 && x < 180 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);

      // Save settings to SD card (if implemented)
      saveSettingsToSD();

      // Return to the appropriate screen
      if (parameterAdjustOrigin == FROM_AUTO_SETTINGS) {
        drawAutoSettings();
      } else if (parameterAdjustOrigin == FROM_ALARM_SETTINGS) {
        drawAlarmSettings();
      } else {
        // Fallback to main menu
        drawMenu();
      }
    }

    // If a value was changed, update the displayed value
    if (valueChanged) {
      drawParameterValue(getParameterValue(currentParameter));
    }
  }



  /**
 * @brief Handle touches on the Fan Mode Settings screen.
 */
  void handleFanModeSettingsTouch(int x, int y) {
    // "Full Auto"
    if (x > 20 && x < 180 && y > 40 && y < 80) {
      drawButtonWithFeedback(20, 40, 160, 40, "Full Auto", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      fanModeFullAuto = true;
      fanModeSemiAuto = false;
      drawButtonWithFeedback(20, 40, 160, 40, "Full Auto", fanModeFullAuto ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
      drawButtonWithFeedback(220, 40, 160, 40, "Semi Auto", fanModeSemiAuto ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    }
    // "Semi Auto"
    else if (x > 220 && x < 380 && y > 40 && y < 80) {
      drawButtonWithFeedback(220, 40, 160, 40, "Semi Auto", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      fanModeFullAuto = false;
      fanModeSemiAuto = true;
      drawButtonWithFeedback(220, 40, 160, 40, "Semi Auto", fanModeSemiAuto ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
      drawButtonWithFeedback(20, 40, 160, 40, "Full Auto", fanModeFullAuto ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    }
    // "Fan Time"
    else if (x > 220 && x < 380 && y > 100 && y < 140) {
      drawButtonWithFeedback(220, 100, 160, 40, "Fan Time", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawFanTimeSettings();
    }
    // "Back"
    else if (x > 20 && x < 180 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawAutoSettings();
    }
  }

  /**
 * @brief Handle touches on the Fan Time Settings screen (increment/decrement).
 */
  void handleFanTimeSettingsTouch(int x, int y) {
    bool valueChanged = false;

    // Fan ON time +
    if (x > 310 && x < 350 && y > 60 && y < 100) {
      drawButtonWithFeedback(310, 60, 40, 40, "+", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButton(310, 60, 40, 40, "+", TFT_BROWN, TFT_WHITE);
      fanOnTime += 10;
      drawFanTimeValue(240, 60, fanOnTime);
      valueChanged = true;
    }
    // Fan ON time -
    else if (x > 360 && x < 400 && y > 60 && y < 100) {
      drawButtonWithFeedback(360, 60, 40, 40, "-", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButton(360, 60, 40, 40, "-", TFT_BROWN, TFT_WHITE);
      if (fanOnTime > 0) fanOnTime -= 10;
      drawFanTimeValue(240, 60, fanOnTime);
      valueChanged = true;
    }
    // Fan OFF time +
    else if (x > 310 && x < 350 && y > 120 && y < 160) {
      drawButtonWithFeedback(310, 120, 40, 40, "+", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButton(310, 120, 40, 40, "+", TFT_BROWN, TFT_WHITE);
      fanOffTime += 10;
      drawFanTimeValue(240, 120, fanOffTime);
      valueChanged = true;
    }
    // Fan OFF time -
    else if (x > 360 && x < 400 && y > 120 && y < 160) {
      drawButtonWithFeedback(360, 120, 40, 40, "-", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButton(360, 120, 40, 40, "-", TFT_BROWN, TFT_WHITE);
      if (fanOffTime > 0) fanOffTime -= 10;
      drawFanTimeValue(240, 120, fanOffTime);
      valueChanged = true;
    }
    // "Back"
    else if (x > 20 && x < 180 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButton(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
      drawFanModeSettings();
    }

    // If the user changed something, we can refresh the "Back" button
    if (valueChanged) {
      drawButton(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
    }
  }

  /*************************************************************
 *         HANDLE TOUCH EVENTS FOR LIGHT SETTINGS
 *************************************************************/

  void handleLightSettingsTouch(int x, int y) {
    // OnTime
    if (x > 20 && x < 160 && y > 60 && y < 100) {
      drawButtonWithFeedback(20, 60, 140, 40, "OnTime", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      adjustOnOffTime(true);
      // Abre una pantalla "Set OnTime" similar a 'drawParameterAdjustScreen'
      // o a la que uses en "FanTime"
    }
    // OffTime
    else if (x > 180 && x < 320 && y > 60 && y < 100) {
      drawButtonWithFeedback(180, 60, 140, 40, "OffTime", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      adjustOnOffTime(false);
    }
    // 50%
    else if (x > 20 && x < 100 && y > 120 && y < 160) {
      // set lightIntensity=50
      // set lightsCompletelyOff=false
      lightsCompletelyOff = false;
      lightIntensity = 50;
      // feedback
      drawLightSettingsScreen();  // re-draw to show "50%" in blue, others in brown
      delay(100);
    }
    // 100%
    else if (x > 110 && x < 190 && y > 120 && y < 160) {
      lightsCompletelyOff = false;
      lightIntensity = 100;
      drawLightSettingsScreen();
      delay(100);
    }
    // Off
    else if (x > 200 && x < 280 && y > 120 && y < 160) {
      // user wants to turn everything off
      lightsCompletelyOff = true;
      // in checkLightsTimer() or manual logic, you set both pins LOW if off
      drawLightSettingsScreen();
      delay(100);
    }
    // Back
    else if (x > 20 && x < 160 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 140, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawAutoSettings();
    }
  }

  // Función 1: Reemplaza la función adjustOnOffTime existente
  void adjustOnOffTime(bool settingOnTime) {
    currentScreen = settingOnTime ? SET_ON_TIME : SET_OFF_TIME;
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print(settingOnTime ? "Set On Time" : "Set Off Time");

    // Mostrar valores actuales
    int h = settingOnTime ? lightOnHour : lightOffHour;
    int m = settingOnTime ? lightOnMinute : lightOffMinute;
    drawOnOffTimeValue(h, m);

    // Botones
    drawButtonWithFeedback(20, 60, 60, 40, "+H", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(100, 60, 60, 40, "-H", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 120, 60, 40, "+M", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(100, 120, 60, 40, "-M", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 180, 140, 40, "Back", TFT_BROWN, TFT_WHITE);
  }

  void handleOnOffTimeTouch(int x, int y) {
    bool isOnTime = (currentScreen == SET_ON_TIME);
    int& hour = isOnTime ? lightOnHour : lightOffHour;
    int& minute = isOnTime ? lightOnMinute : lightOffMinute;
    bool redraw = false;

    // Botones de hora
    if (x > 20 && x < 80 && y > 60 && y < 100) {  // +H
      drawButtonWithFeedback(20, 60, 60, 40, "+H", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButtonWithFeedback(20, 60, 60, 40, "+H", TFT_BROWN, TFT_WHITE, false);
      hour = (hour + 1) % 24;
      redraw = true;
    } else if (x > 100 && x < 160 && y > 60 && y < 100) {  // -H
      drawButtonWithFeedback(100, 60, 60, 40, "-H", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButtonWithFeedback(100, 60, 60, 40, "-H", TFT_BROWN, TFT_WHITE, false);
      hour = (hour - 1 + 24) % 24;
      redraw = true;
    }

    // Botones de minuto
    else if (x > 20 && x < 80 && y > 120 && y < 160) {  // +M
      drawButtonWithFeedback(20, 120, 60, 40, "+M", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButtonWithFeedback(20, 120, 60, 40, "+M", TFT_BROWN, TFT_WHITE, false);
      minute = (minute + 1) % 60;
      redraw = true;
    } else if (x > 100 && x < 160 && y > 120 && y < 160) {  // -M
      drawButtonWithFeedback(100, 120, 60, 40, "-M", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButtonWithFeedback(100, 120, 60, 40, "-M", TFT_BROWN, TFT_WHITE, false);
      minute = (minute - 1 + 60) % 60;
      redraw = true;
    }

    // Botón Back
    else if (x > 20 && x < 160 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 140, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawButtonWithFeedback(20, 180, 140, 40, "Back", TFT_BROWN, TFT_WHITE, false);
      drawLightSettingsScreen();
      return;
    }

    if (redraw) {
      drawOnOffTimeValue(hour, minute);
    }
  }


  /**
 * @brief drawOnOffTimeValue: prints the HH:MM at x=200,y=80 for example
 */
  void drawOnOffTimeValue(int hour, int minute) {
    tft.fillRect(200, 60, 120, 60, TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    tft.setTextSize(3);

    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hour, minute);
    tft.setCursor(200, 80);
    tft.print(timeStr);
  }


  /*************************************************************
 * @brief Handle touches specifically when dataScreenActive == true.
 *        Checks if the user tapped the invisible button at top-right
 *        and, if so, returns to main screen.
 *        Otherwise, ignore touches (so the user can watch data quietly).
 *************************************************************/
  void handleDataScreenTouch(int x, int y) {
    // Check if tapped in the invisible button rectangle
    if (x >= INVIS_BTN_X && x < (INVIS_BTN_X + INVIS_BTN_WIDTH) && y >= INVIS_BTN_Y && y < (INVIS_BTN_Y + INVIS_BTN_HEIGHT)) {
      // User tapped the hidden area => exit Data Screen
      dataScreenActive = false;
      drawMainScreenStatic();
    } else {
      // If you want to do something else on other areas of data screen, do it here.
      // Currently, we do nothing if the user taps outside the invisible button.
    }
  }



  /*************************************************************
 * @brief Checks if any pilot-state changed. 
 *        If yes and we're on the MAIN_SCREEN (and not dataScreen),
 *        calls updateDeviceStatusDisplay().
 *
 *        We track LED1 and LED2 separately:
 *        - If autoModeActive && lightsOn:
 *             LED1 -> always ON
 *             LED2 -> ON only if lightIntensity == 100
 *        - If !autoModeActive && ledsManualOn:
 *             same logic: LED1 on, LED2 on only if intensity==100
 *************************************************************/
  void updatePilotsIfChanged() {
    bool changed = false;

    // Compare each pilot with its old state
    if (mattOn != oldMattOn) {
      changed = true;
      oldMattOn = mattOn;
    }
    if (humidifierOn != oldHumidifierOn) {
      changed = true;
      oldHumidifierOn = humidifierOn;
    }
    if (fanOnManual != oldFanOnManual) {
      changed = true;
      oldFanOnManual = fanOnManual;
    }
    if (uvLightOn != oldUvLightOn) {
      changed = true;
      oldUvLightOn = uvLightOn;
    }

    // Now handle LED1 / LED2 using the new booleans
    bool currentLed1On = ledPanel1On;
    bool currentLed2On = ledPanel2On;

    if (currentLed1On != oldLed1On) {
      changed = true;
      oldLed1On = currentLed1On;
    }
    if (currentLed2On != oldLed2On) {
      changed = true;
      oldLed2On = currentLed2On;
    }

    // If something changed, and we are in MAIN_SCREEN (not data screen),
    // then update the pilot visuals
    if (changed && (currentScreen == MAIN_SCREEN) && !dataScreenActive) {
      updateDeviceStatusDisplay();
    }
  }



  /*************************************************************
 * @brief Automatic mode logic:
 *        - Manages matOn/humidifierOn/fanOnManual
 *          based on temperature/humidity thresholds.
 *        - Also calls updatePilotsIfChanged() to update pilots
 *          only if something changed and only if we are
 *          in MAIN_SCREEN.
 *************************************************************/
  void autoModeControl() {
    // 1) If you wish, load from SD in each iteration:
    //    (You can omit or keep, depending on usage)
    if (sdInitialized) {
      loadSettingsFromSD();
    }

    // 2) Turn off anything that was on in manual mode
    //    We do this to avoid conflicts
    mattOn = false;
    humidifierOn = false;
    fanOnManual = false;
    uvLightOn = false;
    digitalWrite(HEATING_PAD_PIN, LOW);
    digitalWrite(HUMIDIFIER_PIN, LOW);
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(UV_LIGHT_PIN, LOW);

    // 3) Automatic Temperature logic
    if (temperature < (targetTemperature - TEMP_MARGIN)) {
      mattOn = true;
      digitalWrite(HEATING_PAD_PIN, HIGH);
    } else if (temperature >= targetTemperature) {
      mattOn = false;
      digitalWrite(HEATING_PAD_PIN, LOW);
    }

    // 4) Automatic Humidity logic
    if (humidity < (targetHumidity - HUM_MARGIN)) {
      humidifierOn = true;
      digitalWrite(HUMIDIFIER_PIN, HIGH);
    } else if (humidity >= targetHumidity) {
      humidifierOn = false;
      digitalWrite(HUMIDIFIER_PIN, LOW);
    }

    // 5) Automatic Fan logic
    if (fanModeFullAuto) {
      // Full auto => turn fan on if T or H exceed max
      if (temperature > MAX_TEMP || humidity > MAX_HUMIDITY) {
        fanOnManual = true;
        digitalWrite(FAN_PIN, HIGH);
      } else {
        fanOnManual = false;
        digitalWrite(FAN_PIN, LOW);
      }
    } else if (fanModeSemiAuto) {
      // Timed approach
      static unsigned long lastFanToggleTime = 0;
      unsigned long currentTime = millis();

      // If fan is ON, check if we exceed fanOnTime
      if (fanOnManual && (currentTime - lastFanToggleTime >= (unsigned long)fanOnTime * 1000UL)) {
        fanOnManual = false;
        digitalWrite(FAN_PIN, LOW);
        lastFanToggleTime = currentTime;
      }
      // If fan is OFF, check if we exceed fanOffTime
      else if (!fanOnManual && (currentTime - lastFanToggleTime >= (unsigned long)fanOffTime * 1000UL)) {
        fanOnManual = true;
        digitalWrite(FAN_PIN, HIGH);
        lastFanToggleTime = currentTime;
      }
    }

    // 6) Now check if any pilot changed. If yes and we're
    //    in MAIN_SCREEN, refresh the pilot display.
    updatePilotsIfChanged();
  }


  /*************************************************************
 * @brief Manual mode logic:
 *        - Devices turn on/off only if the user toggled them
 *          in handleManualSettingsTouch().
 *        - Also handles UV Light auto-off timer.
 *        - Calls updatePilotsIfChanged() to update pilot icons
 *          only if a device changed and we're in MAIN_SCREEN.
 *************************************************************/
  void manualModeControl() {

    // Only run manual logic if in manual mode
    if (!autoModeActive) {
      // Turn on both LEDs if manual mode is active
      digitalWrite(LED_PANEL_PIN1, ledsManualOn ? HIGH : LOW);
      digitalWrite(LED_PANEL_PIN2, ledsManualOn ? HIGH : LOW);
    }
    if (!autoModeActive) {
      // 1) Write device states to physical pins
      //    based on booleans the user toggled in the UI
      digitalWrite(FAN_PIN, fanOnManual ? HIGH : LOW);
      digitalWrite(HEATING_PAD_PIN, mattOn ? HIGH : LOW);
      digitalWrite(HUMIDIFIER_PIN, humidifierOn ? HIGH : LOW);

      // 2) UV Light timer logic
      static unsigned long uvLightStartTime = 0;
      static bool uvStarted = false;

      // If uvLight turned on just now
      if (uvLightOn && !uvStarted) {
        uvStarted = true;
        uvLightStartTime = millis();
      }

      // If UV is on, check the auto-off
      if (uvLightOn) {
        if (millis() - uvLightStartTime >= UV_LIGHT_MAX_TIME * 1000UL) {
          // Turn off UV after max time
          uvLightOn = false;
          digitalWrite(UV_LIGHT_PIN, LOW);
          uvStarted = false;
        } else {
          // Keep it ON
          digitalWrite(UV_LIGHT_PIN, HIGH);
        }
      } else {
        // If uvLightOn is false, ensure pin is LOW
        digitalWrite(UV_LIGHT_PIN, LOW);
        uvStarted = false;
      }



      // 3) Check pilot changes
      updatePilotsIfChanged();
    }
  }



  /*************************************************************
 * @brief Checks all sensors for alarm conditions and updates
 *        the display icons if the alarm status has changed.
 *************************************************************/
  /*************************************************************
 * @brief Checks all sensors for alarm conditions and updates
 *        the display icons depending on the current screen.
 *        - If in MAIN_SCREEN (and not dataScreenActive),
 *          we draw the main screen alarm icons.
 *        - If dataScreenActive == true, we call drawActiveAlarms()
 *          for the Data Screen layout.
 *************************************************************/
  void checkAlarms() {
    // --- 1) Read the Water Sensor (boya) ---
    //     Example: using pull-up, "LOW" = tank empty => alarm
    //     Adjust to your hardware (HIGH or LOW).
    int waterReading = digitalRead(WATER_SENSOR_PIN);
    bool newWaterAlarmState = (waterReading == LOW);
    // ^^^ If en tu hardware es al revés, usa HIGH

    // --- 2) Compare temperature/humidity with max thresholds
    bool newTempAlarmState = (temperature > MAX_TEMP);
    bool newHumidityAlarmState = (humidity > MAX_HUMIDITY);

    // --- 3) Check if any alarm state changed
    bool alarmStateChanged = false;

    if (newWaterAlarmState != waterAlarmActive) {
      waterAlarmActive = newWaterAlarmState;
      alarmStateChanged = true;
    }
    if (newTempAlarmState != tempAlarmActive) {
      tempAlarmActive = newTempAlarmState;
      alarmStateChanged = true;
    }
    if (newHumidityAlarmState != humidityAlarmActive) {
      humidityAlarmActive = newHumidityAlarmState;
      alarmStateChanged = true;
    }

    // --- 4) If something changed, update the display
    if (alarmStateChanged) {
      // If we are in the MAIN_SCREEN (and not dataScreenActive),
      // redraw alarm icons on the main screen
      if (currentScreen == MAIN_SCREEN && !dataScreenActive) {
        // This function just erases the portion of the main screen
        // where alarms icons are, and draws them again if active.
        checkAlarmsOnMainScreen();
      }

      // If the Data Screen is active, use the function for data screen alarms
      // (the center icons, presumably).
      if (dataScreenActive) {
        drawActiveAlarms();
      }
    }
  }


  /*************************************************************
 * @brief Updates/Redraws the alarm icons on the Main Screen.
 *        Called whenever an alarm state changes while
 *        currentScreen == MAIN_SCREEN.
 *************************************************************/
  void checkAlarmsOnMainScreen() {
    // 1) Clear the area where alarm icons appear
    //    (for example, a 100x90 rectangle at x=300, y=30)
    tft.fillRect(300, 30, 100, 90, TFT_BLACK);

    // 2) If waterAlarmActive == true, draw the water alarm icon
    if (waterAlarmActive) {
      drawButtonWithFeedback(300, 30, 100, 30, "Water", TFT_RED, TFT_WHITE);
    }

    // 3) If tempAlarmActive == true, draw the temp alarm icon
    if (tempAlarmActive) {
      drawButtonWithFeedback(300, 60, 100, 30, "Temp High", TFT_RED, TFT_WHITE);
    }

    // 4) If humidityAlarmActive == true, draw the humidity alarm icon
    if (humidityAlarmActive) {
      drawButtonWithFeedback(300, 90, 100, 30, "Hum High", TFT_RED, TFT_WHITE);
    }

    // If none are active, the rectangle stays black (no alarm).
  }



  void logDataToSD() {
    if (!sdInitialized) return;
    myFile = SD.open("registro.txt", FILE_WRITE);
    if (myFile) {
      myFile.print("Temp: ");
      myFile.print(temperature);
      myFile.print(" C, Hum: ");
      myFile.print(humidity);
      myFile.print(" %, Agua: ");
      myFile.println(waterLevelEmpty ? "Vacío" : "Lleno");

      if (tempAlarmActive) myFile.println("Alarma: Temperatura Alta");
      if (humidityAlarmActive) myFile.println("Alarma: Humedad Alta");
      if (waterAlarmActive) myFile.println("Alarma: Agua Vacía");
      myFile.close();
    } else {
      Serial.println("Failed to open 'registro.txt' on SD for writing.");
    }
  }

  /*************************************************************
 *                     SD CARD FUNCTIONS
 *************************************************************/
  bool initializeSDCard() {
    if (!sdInitialized) {
      sdInitialized = SD.begin(SD_CS_PIN);
    }
    return sdInitialized;
  }

  void displaySDError() {
    tft.fillRect(10, 200, 460, 40, TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(20, 210);
    tft.print("Error: SD card not found.");
  }

  void displaySDData() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Data on SD:");
    // Additional code for display
  }

  void displaySDGraphs() {
    // Clear screen
    tft.fillScreen(TFT_BLACK);
    // We can show a small title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("SD Graph Demo");

    // Then call the plot
    plotSDGraph();

    // If you want to pause or wait for user input, you might do so here
  }


  void displaySDAlarms() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Alarms on SD:");
    // Additional code for display
  }

  void deleteSDRecords() {
    if (SD.exists("registro.txt")) {
      SD.remove("registro.txt");
    }
  }

  void displaySDMessage(const char* message) {
    tft.fillRect(10, 200, 460, 40, TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.setCursor(20, 210);
    tft.print(message);
  }

  void displaySDMoreOptions() {
    // Future implementation
  }

  /**
 * @brief Show the SD Card menu screen.
 */
  void drawSDCardMenu() {
    currentScreen = SD_CARD_MENU;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("SD Card Menu");

    // SD menu buttons
    drawButtonWithFeedback(20, 40, 160, 40, "Data", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 40, 160, 40, "Graphs", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 100, 160, 40, "Alarms", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 100, 160, 40, "Delete Reg", TFT_RED, TFT_WHITE);
    drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 160, 160, 40, "More", TFT_BROWN, TFT_WHITE);
  }

  /**
 * @brief Handle touch events in the SD Card menu.
 */
  void handleSDCardMenuTouch(int x, int y) {
    // "Data"
    if (x > 20 && x < 180 && y > 40 && y < 80) {
      drawButtonWithFeedback(20, 40, 160, 40, "Data", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      if (initializeSDCard()) {
        displaySDData();
      } else {
        displaySDError();
      }
    }
    // "Graphs"
    else if (x > 220 && x < 380 && y > 40 && y < 80) {
      drawButtonWithFeedback(220, 40, 160, 40, "Graphs", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      if (initializeSDCard()) {
        displaySDGraphs();
      } else {
        displaySDError();
      }
    }
    // "Alarms"
    else if (x > 20 && x < 180 && y > 100 && y < 140) {
      drawButtonWithFeedback(20, 100, 160, 40, "Alarms", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      if (initializeSDCard()) {
        displaySDAlarms();
      } else {
        displaySDError();
      }
    }
    // "Delete Reg"
    else if (x > 220 && x < 380 && y > 100 && y < 140) {
      drawButtonWithFeedback(220, 100, 160, 40, "Delete Reg", TFT_RED, TFT_WHITE, true);
      delay(100);
      if (initializeSDCard()) {
        deleteSDRecords();
        displaySDMessage("Records deleted.");
      } else {
        displaySDError();
      }
    }
    // "Back"
    else if (x > 20 && x < 180 && y > 180 && y < 220) {
      drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      drawMenu();
    }
    // "More"
    else if (x > 220 && x < 380 && y > 160 && y < 200) {
      drawButtonWithFeedback(220, 160, 160, 40, "More", TFT_BROWN, TFT_WHITE, true);
      delay(100);
      displaySDMoreOptions();
    }
  }

  /*************************************************************
 *                 LOAD/SAVE SETTINGS FROM SD
 *************************************************************/
  void saveSettingsToSD() {
    if (SD.begin(SD_CS_PIN)) {
      myFile = SD.open("config.txt", FILE_WRITE);
      if (myFile) {
        myFile.print("Temp:");
        myFile.println(targetTemperature);
        myFile.print("Hum:");
        myFile.println(targetHumidity);
        myFile.close();
        Serial.println("Settings saved to SD (config.txt).");
      } else {
        Serial.println("Error opening config.txt for writing.");
      }
    } else {
      Serial.println("Could not initialize SD card for saving settings.");
    }
  }

  void loadSettingsFromSD() {
    if (SD.begin(SD_CS_PIN)) {
      myFile = SD.open("config.txt");
      if (myFile) {
        String line;
        while (myFile.available()) {
          line = myFile.readStringUntil('\n');
          if (line.startsWith("Temp:")) {
            targetTemperature = line.substring(5).toFloat();
          } else if (line.startsWith("Hum:")) {
            targetHumidity = line.substring(4).toFloat();
          }
        }
        myFile.close();
        Serial.println("Settings loaded from SD (config.txt).");
      } else {
        Serial.println("config.txt not found, using default values.");
      }
    } else {
      Serial.println("Could not initialize SD card, using default values.");
    }
  }

  /*************************************************************
 *   EXAMPLE: PLOT A SIMPLE TEMPERATURE GRAPH FROM registro.txt
 *   This is a naive approach that reads the file line by line,
 *   extracts the temperature after "Temp: ", stores values in
 *   an array, and then draws a line chart.
 *************************************************************/
  void plotSDGraph() {
    // Clear screen
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Temp Graph from SD");

    // Attempt to open registro.txt
    if (!sdInitialized) {
      tft.setCursor(10, 50);
      tft.print("SD not init.");
      return;
    }

    File dataFile = SD.open("registro.txt");
    if (!dataFile) {
      tft.setCursor(10, 50);
      tft.print("No registro.txt found!");
      return;
    }

// We store up to 200 temperature readings in an array
#define MAX_POINTS 200
    float temps[MAX_POINTS];
    int count = 0;

    // Read line by line
    while (dataFile.available() && count < MAX_POINTS) {
      String line = dataFile.readStringUntil('\n');
      // We look for a substring: "Temp: "
      int idx = line.indexOf("Temp:");
      if (idx >= 0) {
        // Extract the part after "Temp: "
        // e.g. line = "Temp: 23.5 C, Hum: 65.0 %, Agua: Lleno"
        String sub = line.substring(idx + 6);  // skip "Temp: "
        // parse up to the space or next comma
        float val = sub.toFloat();
        temps[count] = val;
        count++;
      }
    }
    dataFile.close();

    // Now we have 'count' temperature values in temps[]
    if (count < 2) {
      tft.setCursor(10, 70);
      tft.print("Not enough data!");
      return;
    }

    // We'll draw a basic line graph from these points
    // e.g. x from 10..310, y from 220..40
    int x0 = 10;
    int y0 = 220;
    int graphWidth = 300;
    int graphHeight = 180;

    // Draw axis
    tft.drawRect(x0, y0 - graphHeight, graphWidth, graphHeight, TFT_WHITE);

    // We need to find min and max to scale the data
    float minTemp = temps[0];
    float maxTemp = temps[0];
    for (int i = 1; i < count; i++) {
      if (temps[i] < minTemp) minTemp = temps[i];
      if (temps[i] > maxTemp) maxTemp = temps[i];
    }
    if (minTemp == maxTemp) {
      // avoid divide by zero
      maxTemp = minTemp + 1.0;
    }

    // We'll plot each point
    // dx = graphWidth / (count-1)
    // scale for y
    float dx = (float)graphWidth / (count - 1);
    for (int i = 0; i < count - 1; i++) {
      float t1 = temps[i];
      float t2 = temps[i + 1];
      int xA = x0 + (int)(dx * i);
      int xB = x0 + (int)(dx * (i + 1));

      // y = map from t1 to screen
      // e.g. y0 - graphHeight * (value - minTemp)/(maxTemp-minTemp)
      int yA = y0 - (int)(graphHeight * (t1 - minTemp) / (maxTemp - minTemp));
      int yB = y0 - (int)(graphHeight * (t2 - minTemp) / (maxTemp - minTemp));

      // draw line
      tft.drawLine(xA, yA, xB, yB, TFT_GREEN);
    }
  }


  /*************************************************************
 *                 pH CALIBRATION FUNCTIONS
 *************************************************************/
  void calibratePH7() {
    // Example read from an analog pin
    pH7Value = analogRead(A1) * (5.0 / 1023.0);
    pH7Calibrated = true;
  }

  void calibratePH4() {
    // Example read from an analog pin
    pH4Value = analogRead(A1) * (5.0 / 1023.0);
    pH4Calibrated = true;
  }

  void calculatePHCalibration() {
    if (pH7Calibrated && pH4Calibrated) {
      pHSlope = (4.0 - 7.0) / (pH4Value - pH7Value);
      pHIntercept = 7.0 - pHSlope * pH7Value;
    }
  }

  float getCalibratedPH() {
    float sensorReading = analogRead(A1) * (5.0 / 1023.0);
    float calculatedPH = pHSlope * sensorReading + pHIntercept;

    // We consider out-of-range (<3 or >9) as invalid for display
    if (calculatedPH < 3.0 || calculatedPH > 9.0) {
      return NAN;
    } else {
      return calculatedPH;
    }
  }

  /*************************************************************
 *                    HELPER FUNCTIONS
 *************************************************************/
  void drawFanTimeValue(int x, int y, int value) {
    tft.fillRect(x, y, 60, 40, TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(x + 10, y + 10);
    tft.print(value);
    tft.print(" s");
  }

  void drawParameterValue(float value) {
    // Clear the area where the value is displayed
    tft.fillRect(20, 60, 180, 40, TFT_BLACK);

    // Set text properties
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(4);

    // Draw the value
    tft.setCursor(20, 60);
    tft.print(value, 1);  // Display with 1 decimal place

    // Add the unit (if applicable)
    if (currentParameter == TEMP) {
      tft.print((char)247);  // Degree symbol
      tft.print("C");
    } else if (currentParameter == HUMIDITY) {
      tft.print(" %");
    }
  }

  const char* getParameterName(Parameter param) {
    switch (param) {
      case TEMP: return "Temperature";
      case HUMIDITY: return "Humidity";
      case FAN_ON_TIME: return "Fan On Time";
      case FAN_OFF_TIME: return "Fan Off Time";
      default: return "";
    }
  }

  /**
 * @brief Returns the correct parameter value (auto vs. alarm) 
 *        based on currentParameter + parameterAdjustOrigin.
 */
  float getParameterValue(Parameter param) {
    if (parameterAdjustOrigin == FROM_AUTO_SETTINGS) {
      switch (param) {
        case TEMP: return autoTemperatureSetpoint;
        case HUMIDITY: return autoHumiditySetpoint;
        default: return 0.0;
      }
    } else if (parameterAdjustOrigin == FROM_ALARM_SETTINGS) {
      switch (param) {
        case TEMP: return alarmTemperatureSetpoint;
        case HUMIDITY: return alarmHumiditySetpoint;
        default: return 0.0;
      }
    }
    // Fallback if unknown
    return 0.0;
  }
