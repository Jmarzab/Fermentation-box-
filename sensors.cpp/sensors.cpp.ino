#include "sensors.h"
#include "alarms.h"   // For handling alarm states
#include "control.h"  // For auto/manual mode logic

/**
 * @brief Initializes sensors such as DHT and RTC.
 */
void initializeSensors() {
    Serial.println("Initializing sensors...");

    // Initialize DHT sensor
    dht.begin();
    Serial.println("DHT sensor initialized.");

    // Initialize RTC
    Serial.println("Initializing RTC...");
    if (!rtc.begin()) {
        Serial.println("RTC not detected, using simulated time.");
        rtcAvailable = false;
    } else {
        Serial.println("RTC initialized.");
        rtcAvailable = true;

        // Check if RTC lost power
        if (rtc.lostPower()) {
            Serial.println("RTC lost power, setting default date/time.");
            rtc.adjust(DateTime(simYear, simMonth, simDay, simHour, simMinute, 0));
        }
    }
}

/**
 * @brief Reads temperature from the DHT sensor.
 * @return Temperature in Celsius (returns NAN if failed).
 */
float getTemperature() {
    float temp = dht.readTemperature();
    return isnan(temp) ? NAN : temp;
}

/**
 * @brief Reads humidity from the DHT sensor.
 * @return Humidity percentage (returns NAN if failed).
 */
float getHumidity() {
    float hum = dht.readHumidity();
    return isnan(hum) ? NAN : hum;
}

/**
 * @brief Reads pH value and applies calibration.
 * @return Calibrated pH value (returns NAN if out of range).
 */
float getCalibratedPH() {
    float sensorReading = analogRead(PH_SENSOR_PIN) * (5.0 / 1023.0);
    float calculatedPH = pHSlope * sensorReading + pHIntercept;

    // Consider out-of-range values as invalid
    if (calculatedPH < 3.0 || calculatedPH > 9.0) {
        return NAN;
    } else {
        return calculatedPH;
    }
}

/**
 * @brief Checks if the water tank is empty based on sensor reading.
 * @return true if the tank is empty, false otherwise.
 */
bool isWaterTankEmpty() {
    int waterReading = digitalRead(WATER_SENSOR_PIN);
    return (waterReading == LOW);  // Adjust for your sensor (LOW = empty or HIGH = empty)
}

/**
 * @brief Simulates time when RTC is not available.
 */
void updateSimTime() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastSimUpdate >= 60000) {  // Every 60s
        lastSimUpdate = currentMillis;
        simMinute++;

        if (simMinute >= 60) {
            simMinute = 0;
            simHour++;
        }
        if (simHour >= 24) {
            simHour = 0;
            simDay++;
        }
        // No leap year handling (simplified)
    }
}

/**
 * @brief Retrieves current time from RTC (or simulation if RTC is unavailable).
 * @param hour Output parameter for the current hour.
 * @param minute Output parameter for the current minute.
 */
void getCurrentTime(int &hour, int &minute) {
    if (rtcAvailable) {
        DateTime now = rtc.now();
        hour = now.hour();
        minute = now.minute();
    } else {
        hour = simHour;
        minute = simMinute;
    }
}
