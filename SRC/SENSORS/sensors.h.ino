#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"

// Function declarations for sensor readings
void initializeSensors();         // Initializes all sensors
float getTemperature();           // Reads temperature from DHT sensor
float getHumidity();              // Reads humidity from DHT sensor
float getCalibratedPH();          // Returns pH reading using calibration
bool isWaterTankEmpty();          // Checks if water sensor detects an empty tank
void updateSimTime();             // Simulates time if RTC is unavailable
void getCurrentTime(int &hour, int &minute); // Gets current time from RTC or simulation

#endif // SENSORS_H
