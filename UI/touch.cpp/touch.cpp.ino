// *************************************************************
//  TOUCH.CPP - TOUCHSCREEN HANDLING
// *************************************************************
//  This file contains functions for:
//  - Detecting and processing touch inputs
//  - Managing button interactions across different screens
//  - Handling user interactions with settings and toggles
// *************************************************************

#include "config.h"  // Include global configuration file

// *************************************************************
//              GENERAL TOUCH HANDLING FUNCTION
// *************************************************************
//  - Reads the touch sensor and maps coordinates
//  - Routes touch events to the appropriate screen handler
// *************************************************************
void handleTouch(int x, int y) {
  if (currentScreen == MAIN_SCREEN) {
    handleMainScreenTouch(x, y);
  } else if (currentScreen == MENU_SCREEN) {
    handleMenuTouch(x, y);
  } else if (currentScreen == AUTO_SETTINGS) {
    handleAutoSettingsTouch(x, y);
  } else if (currentScreen == MANUAL_SETTINGS) {
    handleManualSettingsTouch(x, y);
  } else if (currentScreen == PARAMETER_ADJUST) {
    handleParameterAdjustTouch(x, y);
  } else if (currentScreen == FAN_MODE_SETTINGS) {
    handleFanModeSettingsTouch(x, y);
  } else if (currentScreen == FAN_TIME_SETTINGS) {
    handleFanTimeSettingsTouch(x, y);
  } else if (currentScreen == PH_CALIBRATION) {
    handlePhCalibrationTouch(x, y);
  } else if (currentScreen == SD_CARD_MENU) {
    handleSDCardMenuTouch(x, y);
  } else if (currentScreen == SCREEN_DATA) {
    handleDataScreenTouch(x, y);
  } else if (currentScreen == LIGHT_SETTINGS) {
    handleLightSettingsTouch(x, y);
  } else if (currentScreen == SET_CLOCK_SCREEN) {
    handleSetClockTouch(x, y);
  }
}

// *************************************************************
//                MAIN SCREEN TOUCH HANDLING
// *************************************************************
//  - Handles button presses on the main screen
//  - Detects user selection for Auto, Manual, Menu, and Grow modes
// *************************************************************
void handleMainScreenTouch(int x, int y) {
  // "Auto" Mode Button
  if (x > 20 && x < 180 && y > 200 && y < 240) {
    drawButtonWithFeedback(20, 200, 160, 40, "Auto", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    autoModeActive = true;
    drawMainScreenStatic();
  }
  // "Manual" Mode Button
  else if (x > 220 && x < 380 && y > 200 && y < 240) {
    drawButtonWithFeedback(220, 200, 160, 40, "Manual", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    autoModeActive = false;
    drawMainScreenStatic();
  }
  // "Menu" Button
  else if (x > 20 && x < 180 && y > 260 && y < 300) {
    drawButtonWithFeedback(20, 260, 160, 40, "Menu", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    drawMenu();
  }
  // "Grow" Mode Button (future implementation)
  else if (x > 220 && x < 380 && y > 260 && y < 300) {
    drawButtonWithFeedback(220, 260, 160, 40, "Grow", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    // Placeholder for future Grow Mode logic
  }
}

// *************************************************************
//                MENU SCREEN TOUCH HANDLING
// *************************************************************
//  - Handles button presses in the main menu
//  - Navigates to various settings screens
// *************************************************************
void handleMenuTouch(int x, int y) {
  // "Auto Settings"
  if (x > 20 && x < 180 && y > 40 && y < 80) {
    drawButtonWithFeedback(20, 40, 160, 40, "Auto Settings", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    drawAutoSettings();
  }
  // "Manual Settings"
  else if (x > 220 && x < 380 && y > 40 && y < 80) {
    drawButtonWithFeedback(220, 40, 160, 40, "Manual Settings", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    drawManualSettings();
  }
  // "Alarms"
  else if (x > 20 && x < 180 && y > 100 && y < 140) {
    drawButtonWithFeedback(20, 100, 160, 40, "Alarms", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    drawAlarmSettings();
  }
  // "pH Calibration"
  else if (x > 220 && x < 380 && y > 100 && y < 140) {
    drawButtonWithFeedback(220, 100, 160, 40, "pH Calibration", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    drawPhCalibrationScreen();
  }
  // "Fan Mode"
  else if (x > 20 && x < 180 && y > 160 && y < 200) {
    drawButtonWithFeedback(20, 160, 160, 40, "Fan Mode", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    drawFanModeSettings();
  }
  // "SD Card"
  else if (x > 220 && x < 380 && y > 160 && y < 200) {
    drawButtonWithFeedback(220, 160, 160, 40, "SD Card", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    drawSDCardMenu();
  }
  // "Back"
  else if (x > 20 && x < 180 && y > 220 && y < 260) {
    drawButtonWithFeedback(20, 220, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
    delay(100);
    drawMainScreenStatic();
  }
}

// *************************************************************
//      HANDLING TOUCH EVENTS FOR OTHER SCREENS
// *************************************************************
//  - Auto Settings: handleAutoSettingsTouch(x, y);
//  - Manual Settings: handleManualSettingsTouch(x, y);
//  - Parameter Adjustments: handleParameterAdjustTouch(x, y);
//  - Fan Mode: handleFanModeSettingsTouch(x, y);
//  - Fan Time: handleFanTimeSettingsTouch(x, y);
//  - Light Settings: handleLightSettingsTouch(x, y);
//  - pH Calibration: handlePhCalibrationTouch(x, y);
//  - SD Card Menu: handleSDCardMenuTouch(x, y);
//  - Clock Setup: handleSetClockTouch(x, y);
// *************************************************************

// *************************************************************
//  HANDLE TOUCH WHEN DATA SCREEN IS ACTIVE
// *************************************************************
//  - If the Data Screen is active, only check for exit button
// *************************************************************
void handleDataScreenTouch(int x, int y) {
  // Check if tapped in the invisible button rectangle (top-right)
  if (x >= INVIS_BTN_X && x < (INVIS_BTN_X + INVIS_BTN_WIDTH) &&
      y >= INVIS_BTN_Y && y < (INVIS_BTN_Y + INVIS_BTN_HEIGHT)) {
    // User tapped the hidden area => exit Data Screen
    dataScreenActive = false;
    drawMainScreenStatic();
  }
}
