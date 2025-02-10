// *************************************************************
//  UI.CPP - USER INTERFACE FUNCTIONS
// *************************************************************
//  This file contains all functions related to the graphical 
//  interface, including:
//  - Drawing main UI screens
//  - Rendering gauges and buttons
//  - Handling touch input for UI elements
// *************************************************************

#include "config.h"  // Include global configuration file

// *************************************************************
//                 DRAW MAIN SCREEN
// *************************************************************
//  Draws the main screen layout, including:
//  - System mode buttons (Auto, Manual, Menu, Grow)
//  - Device status indicators (left side)
//  - Alarm indicators (right side)
// *************************************************************
void drawMainScreenStatic() {
  currentScreen = MAIN_SCREEN;
  tft.fillScreen(TFT_BLACK);  // Clear screen

  // ********** Draw Buttons **********
  drawButton(20, 200, 80, 40, "Auto", TFT_BROWN, TFT_WHITE);
  drawButton(120, 200, 80, 40, "Manual", TFT_BROWN, TFT_WHITE);
  drawButton(220, 200, 80, 40, "Menu", TFT_BROWN, TFT_WHITE);
  drawButton(320, 200, 80, 40, "Grow", TFT_BROWN, TFT_WHITE);

  // ********** Draw Device Status Indicators **********
  drawButton(10, 50, 50, 30, "M", TFT_GRAY, TFT_BLACK);
  drawButton(10, 85, 50, 30, "H", TFT_GRAY, TFT_BLACK);
  drawButton(10, 120, 50, 30, "F", TFT_GRAY, TFT_BLACK);
  drawButton(10, 155, 50, 30, "UV", TFT_GRAY, TFT_BLACK);

  // ********** Draw Alarm Indicators **********
  drawButton(330, 50, 80, 30, "Water", TFT_BLACK, TFT_YELLOW);
  drawButton(330, 85, 80, 30, "Temp", TFT_BLACK, TFT_YELLOW);
  drawButton(330, 120, 80, 30, "Hum", TFT_BLACK, TFT_YELLOW);

  // ********** Draw Gauges **********
  drawTemperatureGauge(100, 50, 30, temperature);
  drawHumidityGauge(220, 50, 30, humidity);
  drawPHBar(140, 150, 120, 20, getCalibratedPH());

  // ********** Draw Current Time **********
  drawCurrentTime(190, 10, true);
}

// *************************************************************
//                 DRAW MENU SCREEN
// *************************************************************
//  Displays the menu with options:
//  - Auto Settings
//  - Manual Settings
//  - Alarm Settings
//  - pH Calibration
//  - SD Card Menu
//  - Back button
// *************************************************************
void drawMenu() {
  currentScreen = MENU_SCREEN;
  tft.fillScreen(TFT_BLACK);

  drawButton(20, 40, 160, 40, "Auto Settings", TFT_BROWN, TFT_WHITE);
  drawButton(220, 40, 160, 40, "Manual Settings", TFT_BROWN, TFT_WHITE);
  drawButton(20, 100, 160, 40, "Alarm Settings", TFT_BROWN, TFT_WHITE);
  drawButton(220, 100, 160, 40, "pH Calibration", TFT_BROWN, TFT_WHITE);
  drawButton(20, 160, 160, 40, "SD Menu", TFT_BROWN, TFT_WHITE);
  drawButton(220, 160, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
}

// *************************************************************
//              DRAW AUTO SETTINGS SCREEN
// *************************************************************
void drawAutoSettings() {
  currentScreen = AUTO_SETTINGS;
  tft.fillScreen(TFT_BLACK);

  drawButton(20, 40, 160, 40, "Set Temp", TFT_BROWN, TFT_WHITE);
  drawButton(220, 40, 160, 40, "Set Hum", TFT_BROWN, TFT_WHITE);
  drawButton(20, 100, 160, 40, "Fan Mode", TFT_BROWN, TFT_WHITE);
  drawButton(220, 100, 160, 40, "Set Light", TFT_BROWN, TFT_WHITE);
  drawButton(220, 160, 160, 40, "Set Clock", TFT_BROWN, TFT_WHITE);
  drawButton(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
}

// *************************************************************
//              DRAW MANUAL SETTINGS SCREEN
// *************************************************************
void drawManualSettings() {
  currentScreen = MANUAL_SETTINGS;
  tft.fillScreen(TFT_BLACK);

  drawButton(20, 40, 160, 40, "Matt", TFT_BROWN, TFT_WHITE);
  drawButton(220, 40, 160, 40, "Humidifier", TFT_BROWN, TFT_WHITE);
  drawButton(20, 100, 160, 40, "UV Light", TFT_BROWN, TFT_WHITE);
  drawButton(220, 100, 160, 40, "Fan", TFT_BROWN, TFT_WHITE);
  drawButton(220, 160, 160, 40, "LEDS", TFT_BROWN, TFT_WHITE);
  drawButton(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
}

// *************************************************************
//              DRAW PARAMETER ADJUSTMENT SCREEN
// *************************************************************
void drawParameterAdjustScreen(const char* parameterName, float currentValue) {
  currentScreen = PARAMETER_ADJUST;
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.print(parameterName);

  drawValueWindow(20, 60, 180, 40, currentValue);

  drawButton(240, 60, 60, 40, "+", TFT_BROWN, TFT_WHITE);
  drawButton(320, 60, 60, 40, "-", TFT_BROWN, TFT_WHITE);
  drawButton(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
}

// *************************************************************
//            DRAW BUTTONS WITH & WITHOUT FEEDBACK
// *************************************************************
void drawButton(int x, int y, int w, int h, const char* label, uint16_t color, uint16_t textColor) {
  tft.fillRoundRect(x, y, w, h, 5, color);
  tft.drawRoundRect(x, y, w, h, 5, TFT_WHITE);
  tft.setCursor(x + 10, y + 10);
  tft.setTextSize(2);
  tft.setTextColor(textColor);
  tft.print(label);
}

void drawButtonWithFeedback(int x, int y, int w, int h, const char* label, uint16_t color, uint16_t textColor, bool pressed) {
  if (pressed) {
    tft.fillRoundRect(x, y, w, h, 5, TFT_BLUE);
  } else {
    tft.fillRoundRect(x, y, w, h, 5, color);
  }
  tft.drawRoundRect(x, y, w, h, 5, TFT_WHITE);
  tft.setCursor(x + 10, y + 10);
  tft.setTextSize(2);
  tft.setTextColor(textColor);
  tft.print(label);
}

// *************************************************************
//           DRAW VALUE WINDOW FOR PARAMETER ADJUSTMENT
// *************************************************************
void drawValueWindow(int x, int y, int w, int h, float value) {
  tft.fillRect(x, y, w, h, TFT_BLACK);
  tft.drawRect(x, y, w, h, TFT_WHITE);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.setCursor(x + 10, y + 10);
  tft.print(value, 1);  // Show value with 1 decimal place
}

// *************************************************************
//          DRAW CURRENT TIME ON SCREEN
// *************************************************************
void drawCurrentTime(int x, int y, bool isMainScreen) {
  char timeStr[6]; // HH:MM format
  if (rtcAvailable) {
    DateTime now = rtc.now();
    sprintf(timeStr, "%02d:%02d", now.hour(), now.minute());
  } else {
    sprintf(timeStr, "%02d:%02d", simHour, simMinute);
  }
  
  tft.setCursor(x, y);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.print(timeStr);
}
