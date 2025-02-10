//************************************************************
// File: sdcard.h
// Description: Function declarations for SD card handling.
// Provides functions to initialize, read, write, and manage
// data stored on the SD card.
//************************************************************

#ifndef SDCARD_H
#define SDCARD_H

#include <Arduino.h>
#include <SD.h>

// SD card chip select pin (defined in config.h)
extern const int SD_CS_PIN;

// Function prototypes
bool initializeSDCard();
void displaySDError();
void displaySDData();
void displaySDGraphs();
void displaySDAlarms();
void deleteSDRecords();
void displaySDMessage(const char* message);
void displaySDMoreOptions();
void handleSDCardMenuTouch(int x, int y);
void logDataToSD();
void loadSettingsFromSD();
void saveSettingsToSD();
void plotSDGraph();

#endif // SDCARD_H
