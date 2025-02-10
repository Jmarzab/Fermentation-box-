#include "alarms.h"
#include "control.h"  // To trigger fan if needed
#include "ui.h"       // To update alarm indicators

/**
 * @brief Checks all sensors for alarm conditions and updates
 *        the display indicators if the alarm status has changed.
 */
void checkAlarms() {
    // --- 1) Read the Water Sensor ---
    //     Example: using pull-up, "LOW" = tank empty => alarm
    int waterReading = digitalRead(WATER_SENSOR_PIN);
    bool newWaterAlarmState = (waterReading == LOW);  // Adjust for your sensor

    // --- 2) Compare temperature/humidity with maximum thresholds ---
    bool newTempAlarmState = (temperature > MAX_TEMP);
    bool newHumidityAlarmState = (humidity > MAX_HUMIDITY);

    // --- 3) Check if any alarm state changed ---
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

    // --- 4) If something changed, update the display ---
    if (alarmStateChanged) {
        if (currentScreen == MAIN_SCREEN && !dataScreenActive) {
            checkAlarmsOnMainScreen();  // Update Main Screen
        }
        if (dataScreenActive) {
            drawActiveAlarms();  // Update Data Screen
        }
    }
}

/**
 * @brief Updates the alarm indicators on the Main Screen.
 *        Called whenever an alarm state changes while in MAIN_SCREEN.
 */
void checkAlarmsOnMainScreen() {
    // 1) Clear the alarm display area
    tft.fillRect(300, 30, 100, 90, TFT_BLACK);

    // 2) If water alarm is active, draw the water alarm indicator
    if (waterAlarmActive) {
        drawButtonWithFeedback(300, 30, 100, 30, "Water", TFT_RED, TFT_WHITE);
    }

    // 3) If temperature alarm is active, draw the temperature alarm indicator
    if (tempAlarmActive) {
        drawButtonWithFeedback(300, 60, 100, 30, "Temp High", TFT_RED, TFT_WHITE);
    }

    // 4) If humidity alarm is active, draw the humidity alarm indicator
    if (humidityAlarmActive) {
        drawButtonWithFeedback(300, 90, 100, 30, "Hum High", TFT_RED, TFT_WHITE);
    }

    // If no alarms are active, the area remains black (no alarm).
}

/**
 * @brief Displays the active alarms on the Data Screen.
 *        Called when switching to Data Screen or when an alarm state changes.
 */
void drawActiveAlarms() {
    // Clear the area where alarms will be displayed
    tft.fillRect(150, 100, 200, 90, TFT_BLACK);

    int yOffset = 100;

    // Display "WATER LOW" alarm if active
    if (waterAlarmActive) {
        drawButtonWithFeedback(150, yOffset, 200, 30, "WATER LOW", TFT_RED, TFT_WHITE);
        yOffset += 40;
    }

    // Display "TEMP HIGH" alarm if active
    if (tempAlarmActive) {
        drawButtonWithFeedback(150, yOffset, 200, 30, "TEMP HIGH", TFT_RED, TFT_WHITE);
        yOffset += 40;
    }

    // Display "HUM HIGH" alarm if active
    if (humidityAlarmActive) {
        drawButtonWithFeedback(150, yOffset, 200, 30, "HUM HIGH", TFT_RED, TFT_WHITE);
    }
}
