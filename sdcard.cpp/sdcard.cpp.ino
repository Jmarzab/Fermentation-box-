//************************************************************
// File: sdcard.cpp
// Description: Handles all SD card operations, including
// initialization, reading, writing, and displaying data.
//************************************************************

#include "sdcard.h"
#include "config.h"
#include "ui.h"
#include <SPI.h>
#include <SD.h>

// Global variable for SD initialization status
bool sdInitialized = false;

//************************************************************
// @brief Initializes the SD card and checks if it's available.
// @return True if the SD card is successfully initialized.
//************************************************************
bool initializeSDCard() {
    if (!sdInitialized) {
        sdInitialized = SD.begin(SD_CS_PIN);
    }
    return sdInitialized;
}

//************************************************************
// @brief Displays an error message if the SD card is not found.
//************************************************************
void displaySDError() {
    tft.fillRect(10, 200, 460, 40, TFT_BLACK);
    tft.setTextColor(TFT_RED);
    tft.setTextSize(2);
    tft.setCursor(20, 210);
    tft.print("Error: SD card not found.");
}

//************************************************************
// @brief Displays stored data from the SD card on the screen.
//************************************************************
void displaySDData() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Data on SD:");
    // Additional logic to read and display data can be added here.
}

//************************************************************
// @brief Displays graph data stored on the SD card.
//************************************************************
void displaySDGraphs() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("SD Graph Demo");

    // Call the function to plot the graph
    plotSDGraph();
}

//************************************************************
// @brief Displays alarms stored on the SD card.
//************************************************************
void displaySDAlarms() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("Alarms on SD:");
    // Additional logic to read and display alarms can be added here.
}

//************************************************************
// @brief Deletes the SD card log file (registro.txt).
//************************************************************
void deleteSDRecords() {
    if (SD.exists("registro.txt")) {
        SD.remove("registro.txt");
    }
}

//************************************************************
// @brief Displays a message related to SD card operations.
// @param message The message to be displayed.
//************************************************************
void displaySDMessage(const char* message) {
    tft.fillRect(10, 200, 460, 40, TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(2);
    tft.setCursor(20, 210);
    tft.print(message);
}

//************************************************************
// @brief Placeholder for additional SD card options (future use).
//************************************************************
void displaySDMoreOptions() {
    // Future implementation
}

//************************************************************
// @brief Draws the SD card menu screen.
//************************************************************
void drawSDCardMenu() {
    currentScreen = SD_CARD_MENU;
    tft.fillScreen(TFT_BLACK);

    // Title
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.print("SD Card Menu");

    // Draw buttons for SD card options
    drawButtonWithFeedback(20, 40, 160, 40, "Data", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 40, 160, 40, "Graphs", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(20, 100, 160, 40, "Alarms", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 100, 160, 40, "Delete Reg", TFT_RED, TFT_WHITE);
    drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE);
    drawButtonWithFeedback(220, 160, 160, 40, "More", TFT_BROWN, TFT_WHITE);
}

//************************************************************
// @brief Handles touch events in the SD card menu.
//************************************************************
void handleSDCardMenuTouch(int x, int y) {
    // "Data"
    if (x > 20 && x < 180 && y > 40 && y < 80) {
        drawButtonWithFeedback(20, 40, 160, 40, "Data", TFT_BROWN, TFT_WHITE, true);
        delay(100);
        if (initializeSDCard()) {
            displaySDData();
        } else {
            displaySDError();
        }
    }
    // "Graphs"
    else if (x > 220 && x < 380 && y > 40 && y < 80) {
        drawButtonWithFeedback(220, 40, 160, 40, "Graphs", TFT_BROWN, TFT_WHITE, true);
        delay(100);
        if (initializeSDCard()) {
            displaySDGraphs();
        } else {
            displaySDError();
        }
    }
    // "Alarms"
    else if (x > 20 && x < 180 && y > 100 && y < 140) {
        drawButtonWithFeedback(20, 100, 160, 40, "Alarms", TFT_BROWN, TFT_WHITE, true);
        delay(100);
        if (initializeSDCard()) {
            displaySDAlarms();
        } else {
            displaySDError();
        }
    }
    // "Delete Reg"
    else if (x > 220 && x < 380 && y > 100 && y < 140) {
        drawButtonWithFeedback(220, 100, 160, 40, "Delete Reg", TFT_RED, TFT_WHITE, true);
        delay(100);
        if (initializeSDCard()) {
            deleteSDRecords();
            displaySDMessage("Records deleted.");
        } else {
            displaySDError();
        }
    }
    // "Back"
    else if (x > 20 && x < 180 && y > 180 && y < 220) {
        drawButtonWithFeedback(20, 180, 160, 40, "Back", TFT_BROWN, TFT_WHITE, true);
        delay(100);
        drawMenu();
    }
    // "More"
    else if (x > 220 && x < 380 && y > 160 && y < 200) {
        drawButtonWithFeedback(220, 160, 160, 40, "More", TFT_BROWN, TFT_WHITE, true);
        delay(100);
        displaySDMoreOptions();
    }
}
