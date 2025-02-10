//************************************************************
// File: calibration.h
// Description: Function declarations for pH sensor calibration.
// Provides functions to calibrate the pH probe using pH 7 and
// pH 4 solutions and compute the calibration curve parameters.
//************************************************************

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Arduino.h>

// Function prototypes
void calibratePH7();
void calibratePH4();
void calculatePHCalibration();
float getCalibratedPH();

#endif // CALIBRATION_H
