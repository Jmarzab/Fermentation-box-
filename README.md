---
# FermentationChamber

## 📌 Project Overview
FermentationChamber is an Arduino-based control system designed for precise temperature, humidity, and pH regulation in fermentation processes. It features a touchscreen interface, automatic and manual control modes, data logging to an SD card, and real-time monitoring of environmental conditions.

## 📂 Project Structure
# Fermentation Chamber Control System  
📡 Automated Fermentation & Growth Chamber - Arduino Mega-based system for monitoring and controlling temperature, humidity, pH, and more.  

---

## 📂 Project Structure  



## 🔧 Hardware Requirements
- **Microcontroller:** Arduino Mega 2560
- **Display:** 3.6" TFT Touchscreen (MCUFriend)
- **Sensors:**
  - DHT21 (Temperature & Humidity)
  - pH sensor (DFRobot pHmeter V1.1)
  - Water level sensor
  - RTC module (DS3231)
- **Actuators:**
  - Heating pad
  - Ultrasonic humidifier
  - UV light
  - Cooling fan
  - LED grow lights
- **Storage:** SD Card module for data logging

## 🛠️ Software Requirements
- **Arduino IDE** (Latest version recommended)
- **Libraries Required:**
  - `Adafruit_GFX`
  - `MCUFRIEND_kbv`
  - `TouchScreen`
  - `SD`
  - `SPI`
  - `DHT`
  - `RTClib`

To install the required libraries, open the Arduino IDE, go to **Sketch > Include Library > Manage Libraries**, and search for the above libraries.

## 🚀 Installation & Setup
1. **Clone the Repository:**
   ```bash
   git clone https://github.com/yourusername/FermentationChamber.git
   cd FermentationChamber

Open the Arduino Project:
Open main.ino inside the FermentationChamber folder using Arduino IDE.
Verify and Upload:
Connect the Arduino Mega 2560 to your computer via USB.
Select Board: Arduino Mega 2560 in Arduino IDE.
Select the correct COM Port.
Click Verify (✔️) and then Upload (⬆️).

📊 Features & Functionality
🔹 Main UI Components
Touchscreen Interface with buttons for Auto, Manual, Menu, and Grow Mode.
Live Data Display showing:
- Current Temperature & Humidity
- Setpoints for Auto Mode
- pH reading (with a calibration system)
- Alarm indicators for Water, Temp, and Humidity

🔹 Control Modes
Auto Mode:
- Controls heating, humidification, and ventilation based on set thresholds.
- Uses temperature and humidity margins to prevent frequent toggling.
- Turns on cooling fan when exceeding max limits.

Manual Mode:
- Allows user to manually toggle devices (Matt, Fan, UV Light, Humidifier, LEDS).
- Device states persist until Auto Mode is reactivated.

Grow Mode:
- Adjusts lighting duration and intensity for optimal plant growth.

🔹 Alarms & Safety
Water Level Alarm: Activates if the reservoir is empty.
Temperature Alarm: Triggers if temperature exceeds the max threshold.
Humidity Alarm: Activates if humidity surpasses the defined limit.
Failsafe: If no signal is received from the water sensor, the water alarm activates.

🔹 pH Calibration
Two-Point Calibration (pH7 and pH4) to improve accuracy.
pH values outside [3-9] are considered invalid and displayed as "N/A".

🔹 SD Card Logging & Graphs
Data Logging: Temperature, humidity, and alarms are recorded in CSV format.
Graphical Representation: Displays historical trends from registro.txt.
Menu for SD Access:
- View Data
- View Graphs
- View Alarms
- Delete Records

❗ Known Issues
pH Sensor Noise: pH readings may fluctuate slightly due to interference. Shielding cables can help.
SD Card Delay: If logging frequency is too high, the system may experience a lag.

🛠️ Future Improvements
Wi-Fi Integration: Using ESP32 for remote monitoring and control.
Data Export via Web Interface: Ability to access logs and graphs from a browser.
Battery Backup for RTC: Ensuring correct timekeeping after power loss.

📜 License
This project is open-source under the MIT License.

📞 Contact & Support
If you encounter any issues or have suggestions, feel free to create an Issue or reach out via email.

Happy fermenting! 🍶🍞
