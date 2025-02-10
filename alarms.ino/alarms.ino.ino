// *************************************************************
//  ALARMS.CPP - ALARM MANAGEMENT FUNCTIONS
// *************************************************************
//  This file contains functions for:
//  - Checking alarm conditions
//  - Updating the alarm status indicators
//  - Displaying alarms on both the main screen and data screen
// *************************************************************

#include "config.h"  // Include global configuration file

// *************************************************************
//           CHECK SENSOR VALUES & UPDATE ALARM STATES
// *************************************************************
//  This function checks:
//  - Water level sensor (LOW = empty, HIGH = full)
//  - Temperature exceeding MAX_TEMP
//  - Humidity exceeding MAX_HUMIDITY
//  If any of these conditions are met, the corresponding 
//  alarm is triggered.
// *************************************************************
void checkAlarms() {
  // ********** 1) Read the Water Sensor **********
  int waterReading = digitalRead(WATER_SENSOR_PIN);
  bool newWaterAlarmState = (waterReading == LOW);  // LOW = Empty tank

  // ********** 2) Compare Temperature & Humidity with Thresholds **********
  bool newTempAlarmState = (temperature > MAX_TEMP);
  bool newHumidityAlarmState = (humidity > MAX_HUMIDITY);

  // ********** 3) Check if Any Alarm State Has Changed **********
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

  // ********** 4) If an Alarm Changed, Update the Display **********
  if (alarmStateChanged) {
    if (currentScreen == MAIN_SCREEN && !dataScreenActive) {
      updateAlarmsOnMainScreen();
    }
    if (dataScreenActive) {
      drawActiveAlarms();
    }
  }
}

// *************************************************************
//              UPDATE ALARMS ON MAIN SCREEN
// *************************************************************
//  - Clears the section where alarms are displayed
//  - Redraws only active alarms
// *************************************************************
void updateAlarmsOnMainScreen() {
  // ********** 1) Clear Alarm Area **********
  tft.fillRect(300, 30, 100, 90, TFT_BLACK);

  // ********** 2) Draw Water Alarm if Active **********
  if (waterAlarmActive) {
    drawButton(300, 30, 100, 30, "Water", TFT_RED, TFT_WHITE);
  }

  // ********** 3) Draw Temperature Alarm if Active **********
  if (tempAlarmActive) {
    drawButton(300, 60, 100, 30, "Temp High", TFT_RED, TFT_WHITE);
  }

  // ********** 4) Draw Humidity Alarm if Active **********
  if (humidityAlarmActive) {
    drawButton(300, 90, 100, 30, "Hum High", TFT_RED, TFT_WHITE);
  }
}

// *************************************************************
//        DRAW ACTIVE ALARMS ON THE DATA SCREEN
// *************************************************************
//  - Displays alarms in the Data Screen layout
// *************************************************************
void drawActiveAlarms() {
  tft.fillRect(20, 160, 360, 60, TFT_BLACK);  // Clear old alarms

  tft.setTextColor(TFT_RED);
  tft.setTextSize(2);
  tft.setCursor(20, 170);

  if (waterAlarmActive) {
    tft.print("Water Alarm  ");
  }
  if (tempAlarmActive) {
    tft.print("Temp Alarm  ");
  }
  if (humidityAlarmActive) {
    tft.print("Humidity Alarm");
  }
}

// *************************************************************
//           LOG ALARMS & SENSOR DATA TO SD CARD
// *************************************************************
//  - Logs current temperature, humidity, and water level status
//  - Logs active alarms to the SD card if enabled
// *************************************************************
void logDataToSD() {
  if (!sdInitialized) return;  // Skip if SD is not available

  myFile = SD.open("registro.txt", FILE_WRITE);
  if (myFile) {
    // ********** Write Sensor Data **********
    myFile.print("Temp: ");
    myFile.print(temperature);
    myFile.print(" C, Hum: ");
    myFile.print(humidity);
    myFile.print(" %, Water: ");
    myFile.println(waterLevelEmpty ? "Empty" : "Full");

    // ********** Log Alarms if Active **********
    if (tempAlarmActive) myFile.println("ALARM: Temperature High");
    if (humidityAlarmActive) myFile.println("ALARM: Humidity High");
    if (waterAlarmActive) myFile.println("ALARM: Water Level Low");

    myFile.close();
  } else {
    Serial.println("Failed to open 'registro.txt' on SD for writing.");
  }
}
