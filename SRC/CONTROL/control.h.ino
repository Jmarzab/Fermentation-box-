#ifndef CONTROL_H
#define CONTROL_H

#include "config.h"

// Function declarations
void autoModeControl();        // Handles automatic device control based on temperature/humidity
void manualModeControl();      // Handles manual device control (based on user toggles)
void updateDeviceStatusDisplay(); // Refreshes device status indicators on UI
void updatePilotsIfChanged();  // Updates status indicators only if there's a change

#endif // CONTROL_H
