#include "control.h"
#include "ui.h"       // For UI updates
#include "alarms.h"   // For alarm handling
#include "sensors.h"  // For temperature & humidity readings

// Ensure the UV light pin (UV_LIGHT_PIN) is configured as OUTPUT and initially LOW in setup (see main).
// Define maximum UV light on time (safety cutoff duration in seconds) before automatic shutdown
#ifndef UV_LIGHT_MAX_TIME
#define UV_LIGHT_MAX_TIME 1800  // 30 minutes in seconds
#endif

/**
 * @brief Handles automatic control of heating, humidifier, and fan
 *        based on temperature and humidity thresholds.
 */
void autoModeControl() {
    if (!autoModeActive) return;  // If auto mode is not active, exit (skip auto control)

    // Read the current sensor values (temperature and humidity)
    float currentTemp = getTemperature();
    float currentHumidity = getHumidity();

    // --- Temperature control (with hysteresis) ---
    // Turn the heating pad ON if temperature is below (targetTemperature - TEMP_MARGIN)
    if (currentTemp < (targetTemperature - TEMP_MARGIN)) {
        if (!mattOn) {
            mattOn = true;
            digitalWrite(HEATING_PAD_PIN, HIGH);
        }
    }
    // Turn the heating pad OFF if temperature is above (targetTemperature + TEMP_MARGIN)
    else if (currentTemp > (targetTemperature + TEMP_MARGIN)) {
        if (mattOn) {
            mattOn = false;
            digitalWrite(HEATING_PAD_PIN, LOW);
        }
    }
    // If within the threshold range, keep the current heating state unchanged

    // --- Humidity control (with hysteresis) ---
    // Turn the humidifier ON if humidity is below (targetHumidity - HUM_MARGIN)
    if (currentHumidity < (targetHumidity - HUM_MARGIN)) {
        if (!humidifierOn) {
            humidifierOn = true;
            digitalWrite(HUMIDIFIER_PIN, HIGH);
        }
    }
    // Turn the humidifier OFF if humidity is above (targetHumidity + HUM_MARGIN)
    else if (currentHumidity > (targetHumidity + HUM_MARGIN)) {
        if (humidifierOn) {
            humidifierOn = false;
            digitalWrite(HUMIDIFIER_PIN, LOW);
        }
    }
    // If within the threshold range, keep the current humidifier state unchanged

    // --- Fan control ---
    if (fanModeFullAuto) {
        // Full Auto Mode: if temperature or humidity exceeds defined max limits, turn the fan ON
        // Reset any semi-auto fan flags when entering full auto mode (since semi-auto timing is not used here)
        if (currentTemp > MAX_TEMP || currentHumidity > MAX_HUMIDITY) {
            if (!fanOnManual) {
                fanOnManual = true;
                digitalWrite(FAN_PIN, HIGH);
            }
        } else {
            if (fanOnManual) {
                fanOnManual = false;
                digitalWrite(FAN_PIN, LOW);
            }
        }
    }
    else if (fanModeSemiAuto) {
        // Semi-Auto Mode: fan alternates between ON and OFF based on predefined timing intervals
        static unsigned long lastFanToggleTime = 0;
        static bool semiAutoTimerInitialized = false;
        unsigned long currentTime = millis();

        // On first entry into semi-auto mode, initialize the fan toggle timer
        if (!semiAutoTimerInitialized) {
            lastFanToggleTime = currentTime;
            semiAutoTimerInitialized = true;
        }
        
        // Toggle fan state based on timing intervals
        if (fanOnManual) {
            // If the fan is ON and has run for fanOnTime seconds, turn it OFF and reset timer
            if (currentTime - lastFanToggleTime >= fanOnTime * 1000UL) {
                fanOnManual = false;
                digitalWrite(FAN_PIN, LOW);
                lastFanToggleTime = currentTime;
            }
        } else {
            // If the fan is OFF and has been off for fanOffTime seconds, turn it ON and reset timer
            if (currentTime - lastFanToggleTime >= fanOffTime * 1000UL) {
                fanOnManual = true;
                digitalWrite(FAN_PIN, HIGH);
                lastFanToggleTime = currentTime;
            }
        }
    }
    else {
        // Otherwise, if not in full auto or semi-auto mode, no special fan timing is applied.
        // (The semi-auto timer initialization flag can be reset here if needed.)
    }

    // Update UI indicators if any device state changed
    updatePilotsIfChanged();
}

/**
 * @brief Handles manual control logic (Manual Mode).
 *        Devices are toggled on/off only when manually selected by the user.
 *        Also handles manual UV light control with a 30-minute auto-off safety timer.
 */
void manualModeControl() {
    if (autoModeActive) return;  // If auto mode is active, skip manual control logic

    // --- Device Activation (Manual Overrides) ---
    // Set each device output according to its manual state variable
    digitalWrite(FAN_PIN,         fanOnManual   ? HIGH : LOW);
    digitalWrite(HEATING_PAD_PIN, mattOn        ? HIGH : LOW);
    digitalWrite(HUMIDIFIER_PIN,  humidifierOn  ? HIGH : LOW);
    // (UV light is handled in its own section below with timing logic)

    // --- UV Light Auto-Off Timer (Manual Control) ---
    // Manage the UV sterilization light manually, with an automatic shutoff after 30 minutes
    static unsigned long uvLightStartTime = 0;  // Timestamp when UV light was turned on (in this cycle)
    static bool uvStarted = false;              // Flag to indicate UV timer has started for current cycle

    // If the UV light was just turned on, start the timer
    if (uvLightOn && !uvStarted) {
        uvStarted = true;
        uvLightStartTime = millis();
    }

    if (uvLightOn) {
        // UV light is requested ON
        if (millis() - uvLightStartTime >= UV_LIGHT_MAX_TIME * 1000UL) {
            // If UV has been ON for the maximum allowed time, turn it OFF and reset the timer flag
            uvLightOn = false;
            digitalWrite(UV_LIGHT_PIN, LOW);
            uvStarted = false;
        } else {
            // UV light is within the allowed time window, keep the UV output ON
            digitalWrite(UV_LIGHT_PIN, HIGH);
        }
    } else {
        // UV light is OFF (either not turned on or turned off by user/timer)
        // Ensure the UV output is OFF and reset the timer flag so a new cycle can start fresh
        digitalWrite(UV_LIGHT_PIN, LOW);
        uvStarted = false;
    }

    // Update UI indicators if any device state changed due to manual control
    updatePilotsIfChanged();
}

/**
 * @brief Checks if any device's status has changed and updates the UI indicators accordingly.
 *        Only triggers a display update when an actual change is detected to avoid unnecessary refreshes.
 */
void updatePilotsIfChanged() {
    bool changed = false;

    // Check heating pad status
    if (mattOn != oldMattOn) {
        changed = true;
        oldMattOn = mattOn;
    }
    // Check humidifier status
    if (humidifierOn != oldHumidifierOn) {
        changed = true;
        oldHumidifierOn = humidifierOn;
    }
    // Check fan status (manual control state)
    if (fanOnManual != oldFanOnManual) {
        changed = true;
        oldFanOnManual = fanOnManual;
    }
    // Check UV light status
    if (uvLightOn != oldUvLightOn) {
        changed = true;
        oldUvLightOn = uvLightOn;
    }

    // Check LED panel statuses
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

    // If any state changed and we are on the main screen (and not showing a data screen), update the display
    if (changed && currentScreen == MAIN_SCREEN && !dataScreenActive) {
        updateDeviceStatusDisplay();
    }
}

/**
 * @brief Updates the graphical display to reflect the status of active devices.
 *        Draws an indicator (green icon) for each device that is currently ON.
 */
void updateDeviceStatusDisplay() {
    // Clear the status display area (erase previous device indicators)
    tft.fillRect(20, 200, 200, 40, TFT_BLACK);

    // Start drawing indicators from the leftmost position
    int x = 20;
    // Draw a 40x40 green square with label for each active device, and increment x for the next indicator
    if (mattOn) {
        // "M" indicates the heating pad (Mat) is ON
        drawButtonWithFeedback(x, 200, 40, 40, "M", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
    if (humidifierOn) {
        // "H" indicates the humidifier is ON
        drawButtonWithFeedback(x, 200, 40, 40, "H", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
    if (fanOnManual) {
        // "F" indicates the fan is ON
        drawButtonWithFeedback(x, 200, 40, 40, "F", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
    if (uvLightOn) {
        // "UV" indicates the UV light is ON
        drawButtonWithFeedback(x, 200, 40, 40, "UV", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
    if (ledPanel1On) {
        // "L1" indicates LED Panel 1 is ON
        drawButtonWithFeedback(x, 200, 40, 40, "L1", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
    if (ledPanel2On) {
        // "L2" indicates LED Panel 2 is ON
        drawButtonWithFeedback(x, 200, 40, 40, "L2", TFT_GREEN, TFT_BLACK);
        x += 45;
    }
}

