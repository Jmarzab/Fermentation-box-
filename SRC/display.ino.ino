#include "config.h"
#include "display.h"

/**
 * @brief Draws the main screen with:
 *        - "Auto", "Menu", and "Manual" buttons at the bottom.
 *        - Device status indicators on the left.
 *        - Alarms (if active) on the right.
 */
void drawMainScreenStatic() {
    currentScreen = MAIN_SCREEN;
    tft.fillScreen(TFT_BLACK);

    // 🕒 Display the current time at the top-right
    drawCurrentTime(190, 10, true);

    // 🔘 Draw bottom buttons
    drawButtonWithFeedback(10, 190, 120, 30, "Auto", autoModeActive ? TFT_BLUE : TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(140, 190, 120, 30, "Menu", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(270, 190, 120, 30, "Manual", !autoModeActive ? TFT_BLUE : TFT_BROWN, TFT_WHITE);

    // 🔄 Reset alarm tracking to ensure proper updates
    waterAlarmLastState = !waterAlarmActive;
    tempAlarmLastState = !tempAlarmActive;
    humidityAlarmLastState = !humidityAlarmActive;

    // 🔍 Update device status and check alarms once
    updateDeviceStatusDisplay();
    checkAlarms();
}

/**
 * @brief Displays the current time (HH:MM) on the main or data screen.
 * @param x X-coordinate for placement.
 * @param y Y-coordinate for placement.
 * @param isMainScreen If true, updates main screen time; otherwise, updates data screen time.
 */
void drawCurrentTime(int x, int y, bool isMainScreen) {
    DateTime now;
    char currentTime[6];

    // ⏳ Get time from RTC or use simulation
    if (rtcAvailable) {
        now = rtc.now();
    } else {
        now = DateTime(simYear, simMonth, simDay, simHour, simMinute, 0);
    }

    sprintf(currentTime, "%02d:%02d", now.hour(), now.minute());

    // ⚠️ Determine the correct buffer to compare against
    char* lastTime = isMainScreen ? lastMainTime : lastDataTime;

    // 📝 Only update if the time has changed to prevent unnecessary redraws
    if (strcmp(currentTime, lastTime) != 0) {
        // Clear previous time display
        tft.setTextColor(TFT_BLACK);
        tft.setTextSize(2);
        tft.setCursor(x, y);
        tft.print(lastTime);

        // Display new time in cyan color
        tft.setTextColor(TFT_CYAN);
        tft.setCursor(x, y);
        tft.print(currentTime);

        // Store new time for future comparisons
        strcpy(lastTime, currentTime);
    }
}
