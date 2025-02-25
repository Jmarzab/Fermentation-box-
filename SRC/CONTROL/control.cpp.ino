#include "control.h"
#include "ui.h"       // For UI updates
#include "alarms.h"   // For alarm handling
#include "sensors.h"  // For temperature & humidity readings

/**
 * @brief Handles automatic control of heating, humidifier, and fan
 *        based on temperature and humidity thresholds.
 */
void autoModeControl() {
    if (!autoModeActive) return;  // Si no está en modo automático, salir

    // Lectura de los valores actuales de los sensores
    float currentTemp = getTemperature();
    float currentHumidity = getHumidity();

    // --- Control de Temperatura (con histéresis) ---
    // Encender calefacción si la temperatura es demasiado baja
    if (currentTemp < (targetTemperature - TEMP_MARGIN)) {
        if (!mattOn) {
            mattOn = true;
            digitalWrite(HEATING_PAD_PIN, HIGH);
        }
    }
    // Apagar calefacción si la temperatura es demasiado alta
    else if (currentTemp > (targetTemperature + TEMP_MARGIN)) {
        if (mattOn) {
            mattOn = false;
            digitalWrite(HEATING_PAD_PIN, LOW);
        }
    }
    // Si está entre los umbrales, se mantiene el estado actual

    // --- Control de Humedad (con histéresis) ---
    // Encender humidificador si la humedad es demasiado baja
    if (currentHumidity < (targetHumidity - HUM_MARGIN)) {
        if (!humidifierOn) {
            humidifierOn = true;
            digitalWrite(HUMIDIFIER_PIN, HIGH);
        }
    }
    // Apagar humidificador si la humedad es demasiado alta
    else if (currentHumidity > (targetHumidity + HUM_MARGIN)) {
        if (humidifierOn) {
            humidifierOn = false;
            digitalWrite(HUMIDIFIER_PIN, LOW);
        }
    }
    // Si está entre los umbrales, se mantiene el estado actual

    // --- Control del Ventilador ---
    // Modo Full Auto: Enciende el ventilador si se supera un máximo de temperatura o humedad
    if (fanModeFullAuto) {
        // Reiniciamos cualquier flag de semi-auto al entrar en full auto
        // (asumimos que el flag de semi-auto se gestiona solo en ese bloque)
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
    // Modo Semi-Auto: El ventilador se alterna entre encendido y apagado según tiempos definidos
    else if (fanModeSemiAuto) {
        static unsigned long lastFanToggleTime = 0;
        static bool semiAutoTimerInitialized = false;
        unsigned long currentTime = millis();

        // Al ingresar por primera vez en modo semi-auto, inicializar el temporizador
        if (!semiAutoTimerInitialized) {
            lastFanToggleTime = currentTime;
            semiAutoTimerInitialized = true;
        }
        
        // Si el ventilador está encendido, se espera fanOnTime; si está apagado, se espera fanOffTime
        if (fanOnManual) {
            if (currentTime - lastFanToggleTime >= fanOnTime * 1000UL) {
                fanOnManual = false;
                digitalWrite(FAN_PIN, LOW);
                lastFanToggleTime = currentTime;
            }
        } else {
            if (currentTime - lastFanToggleTime >= fanOffTime * 1000UL) {
                fanOnManual = true;
                digitalWrite(FAN_PIN, HIGH);
                lastFanToggleTime = currentTime;
            }
        }
    }
    // En otros casos, se puede reiniciar el flag de semi-auto para asegurar inicialización al reactivar el modo
    else {
        // Si no se está en modo semi-auto, reiniciamos la inicialización del temporizador
        // (Si se requiere, se puede declarar una variable global o similar)
    }

    // Actualizar indicadores en la interfaz si ha habido cambios
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
