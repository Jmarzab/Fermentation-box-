#ifndef ALARMS_H
#define ALARMS_H

#include "config.h"

// Function declarations for alarm handling
void checkAlarms();             // Checks if any alarm condition is met
void checkAlarmsOnMainScreen(); // Updates the alarm indicators on the main screen
void drawActiveAlarms();        // Displays alarms on the Data Screen

#endif // ALARMS_H
