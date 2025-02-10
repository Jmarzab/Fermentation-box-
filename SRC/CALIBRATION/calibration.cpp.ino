//************************************************************
// File: calibration.cpp
// Description: Handles pH sensor calibration and calculation
// of pH values based on the calibration curve.
//************************************************************

#include "calibration.h"
#include "config.h"
#include <math.h>  // For NAN and isnan()

// Global variables for storing calibration points
float pH7Value = 0.0;
float pH4Value = 0.0;
bool pH7Calibrated = false;
bool pH4Calibrated = false;
float pHSlope = 1.0;
float pHIntercept = 0.0;
bool calibratingPH7 = false;
bool calibratingPH4 = false;

//************************************************************
// @brief Captures the sensor reading for pH7 calibration.
//************************************************************
void calibratePH7() {
    pH7Value = analogRead(PH_SENSOR_PIN) * (5.0 / 1023.0);
    pH7Calibrated = true;
}

//************************************************************
// @brief Captures the sensor reading for pH4 calibration.
//************************************************************
void calibratePH4() {
    pH4Value = analogRead(PH_SENSOR_PIN) * (5.0 / 1023.0);
    pH4Calibrated = true;
}

//************************************************************
// @brief Calculates the calibration slope and intercept
//        using the stored pH 7 and pH 4 values.
//************************************************************
void calculatePHCalibration() {
    if (pH7Calibrated && pH4Calibrated) {
        pHSlope = (4.0 - 7.0) / (pH4Value - pH7Value);
        pHIntercept = 7.0 - pHSlope * pH7Value;
    }
}

//************************************************************
// @brief Reads the current sensor value and applies the
//        calibration formula to determine the actual pH value.
// @return The calibrated pH value, or NAN if out of range.
//************************************************************
float getCalibratedPH() {
    float sensorReading = analogRead(PH_SENSOR_PIN) * (5.0 / 1023.0);
    float calculatedPH = pHSlope * sensorReading + pHIntercept;

    // If pH is out of displayable range, return NAN
    if (calculatedPH < 3.0 || calculatedPH > 9.0) {
        return NAN;
    } else {
        return calculatedPH;
    }
}
