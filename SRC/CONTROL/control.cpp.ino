#include "control.h"
#include "ui.h"       // For UI updates
#include "alarms.h"   // For alarm handling
#include "sensors.h"  // For temperature & humidity readings

/**
 * @brief Handles automatic control of heating, humidifier, and fan
 *        based on temperature and humidity thresholds.
 */
void autoModeControl() {
    if (!autoModeActive) return;  // If not in auto mode, exit function

    // Read current sensor values
    float currentTemp = getTemperature();
    float currentHumidity = getHumidity();

    // --- Temperature Control ---
    if (currentTemp < (targetTemperature - TEMP_MARGIN)) {
        mattOn = true;  // Turn on heating pad
        digitalWrite(HEATING_PAD_PIN, HIGH);
    } else if (currentTemp >= targetTemperature) {
        mattOn = false;  // Turn off heating pad
        digitalWrite(HEATING_PAD_PIN, LOW);
    }

    // --- Humidity Control ---
    if (currentHumidity < (targetHumidity - HUM_MARGIN)) {
        humidifierOn = true;  // Turn on humidifier
        digitalWrite(HUMIDIFIER_PIN, HIGH);
    } else if (currentHumidity >= targetHumidity) {
        humidifierOn = false;  // Turn off humidifier
        digitalWrite(HUMIDIFIER_PIN, LOW);
    }

    // --- Fan Control (Full Auto Mode) ---
    if (fanModeFullAuto) {
        if (currentTemp > MAX_TEMP || currentHumidity > MAX_HUMIDITY) {
            fanOnManual = true;
            digitalWrite(FAN_PIN, HIGH);
        } else {
            fanOnManual = false;
            digitalWrite(FAN_PIN, LOW);
        }
    }

    // --- Fan Control (Semi-Auto Mode) ---
    else if (fanModeSemiAuto) {
        static unsigned long lastFanToggleTime = 0;
        unsigned long currentTime = millis();

        if (fanOnManual && (currentTime - lastFanToggleTime >= fanOnTime * 1000UL)) {
            fanOnManual = false;
            digitalWrite(FAN_PIN, LOW);
            lastFanToggleTime = currentTime;
        } else if (!fanOnManual && (currentTime - lastFanToggleTime >= fanOffTime * 1000UL)) {
            fanOnManual = true;
            digitalWrite(FAN_PIN, HIGH);
            lastFanToggleTime = currentTime;
        }
    }

    // Update status indicators if anything changed
    updatePilotsIfChanged();
}

/**
 * @brief Handles manual control logic.
 *        Devices are toggled on/off only when manually selected.
 */
void manualModeControl() {
    if (autoModeActive) return;  // If in auto mode, exit function

    // --- Device Activation ---
    digitalWrite(FAN_PIN, fanOnManual ? HIGH : LOW);
    digitalWrite(HEATING_PAD_PIN, mattOn ? HIGH : LOW);
    digitalWrite(HUMIDIFIER_PIN, humidifierOn ? HIGH : LOW);

    // --- UV Light Auto-Off Timer ---
    static unsigned long uvLightStartTime = 0;
    static bool uvStarted = false;

    if (uvLightOn && !uvStarted) {
        uvStarted = true;
        uvLightStartTime = millis();
    }

    if (uvLightOn) {
        if (millis() - uvLightStartTime >= UV_LIGHT_MAX_TIME * 1000UL) {
            uvLightOn = false;
            digitalWrite(UV_LIGHT_PIN, LOW);
            uvStarted = false;
        } else {
            digitalWrite(UV_LIGHT_PIN, HIGH);
        }
    } else {
        digitalWrite(UV_LIGHT_PIN, LOW);
        uvStarted = false;
    }

    // Update status indicators if anything changed
    updatePilotsIfChanged();
}

/**
 * @brief Checks if any device status changed and updates UI indicators.
 *        Only updates when there's an actual change.
 */
void updatePilotsIfChanged() {
    bool changed = false;

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

    if (changed && currentScreen == MAIN_SCREEN && !dataScreenActive) {
        updateDeviceStatusDisplay();
    }
}

/**
 * @brief Updates the display to reflect the status of active devices.
 */
void updateDeviceStatusDisplay() {
    // Clear the status area
    tft.fillRect(20, 200, 200, 40, TFT_BLACK);

    // Draw active device indicators
    int x = 20;
    if (mattOn) {
        drawButtonWithFeedback(x, 200, 40, 40, "M", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
    if (humidifierOn) {
        drawButtonWithFeedback(x, 200, 40, 40, "H", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
    if (fanOnManual) {
        drawButtonWithFeedback(x, 200, 40, 40, "F", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
    if (uvLightOn) {
        drawButtonWithFeedback(x, 200, 40, 40, "UV", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
}
