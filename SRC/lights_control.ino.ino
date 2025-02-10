#include "config.h"

/**
 * @brief Controls the lights based on user-defined ON/OFF times.
 * 
 * - If lights are manually turned off (`lightsCompletelyOff`), they stay OFF.
 * - If the light timer is disabled, the lights remain OFF.
 * - Otherwise, the function compares the current time with the user-set ON/OFF times
 *   to determine whether the lights should be turned ON or OFF.
 */
void checkLightsTimer() {
    // ⚠️ Only operate lights in automatic mode
    if (!autoModeActive) return;

    // If lights are manually turned off or the timer is disabled, turn them off
    if (lightsCompletelyOff || !lightsTimerEnabled) {
        digitalWrite(LED_PANEL_PIN1, LOW);
        digitalWrite(LED_PANEL_PIN2, LOW);
        return;
    }

    // 🔹 Retrieve the current time
    int currentHour, currentMinute;
    getCurrentTime(currentHour, currentMinute);

    // 🔹 Convert times to minutes for easier comparison
    int nowMins = currentHour * 60 + currentMinute;
    int onMins = lightOnHour * 60 + lightOnMinute;
    int offMins = lightOffHour * 60 + lightOffMinute;

    // 🔹 Determine whether the lights should be ON
    bool turnOn = false;
    if (onMins < offMins) {
        if (nowMins >= onMins && nowMins < offMins) turnOn = true;
    } else if (onMins > offMins) {
        if (nowMins >= onMins || nowMins < offMins) turnOn = true;
    } else {
        turnOn = true;  // If ON and OFF times are the same, lights remain ON
    }

    // 🔹 Set light intensity and turn lights ON/OFF accordingly
    if (turnOn) {
        digitalWrite(LED_PANEL_PIN1, HIGH);
        digitalWrite(LED_PANEL_PIN2, (lightIntensity == 100) ? HIGH : LOW);
    } else {
        digitalWrite(LED_PANEL_PIN1, LOW);
        digitalWrite(LED_PANEL_PIN2, LOW);
    }
}
