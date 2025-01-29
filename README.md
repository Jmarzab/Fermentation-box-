Fermentation Chamber Control System
📡 Automated Fermentation & Growth Chamber - Arduino Mega-based system for monitoring and controlling temperature, humidity, pH, and more.

📂 Project Structure
bash
Copy
Edit
📂 FermentationChamber
│── 📄 main.ino                 # Main Arduino file with setup() and loop()
│── 📂 src                      # Code organized into modular files
│   │── 📄 ui.cpp               # Screen drawing functions, UI elements
│   │── 📄 ui.h                 # UI function declarations
│   │── 📄 touch.cpp            # Touchscreen event handling
│   │── 📄 touch.h              # Touch function declarations
│   │── 📄 control.cpp          # Device control logic (Auto/Manual)
│   │── 📄 control.h            # Control function declarations
│   │── 📄 sensors.cpp          # Sensor readings (DHT, pH, RTC, etc.)
│   │── 📄 sensors.h            # Sensor function declarations
│   │── 📄 alarms.cpp           # Alarm logic and states
│   │── 📄 alarms.h             # Alarm function declarations
│   │── 📄 sdcard.cpp           # SD card handling functions
│   │── 📄 sdcard.h             # SD card function declarations
│   │── 📄 calibration.cpp      # pH calibration functions
│   │── 📄 calibration.h        # Calibration function declarations
│   │── 📄 time.cpp             # Time & date management (RTC & simulation)
│   │── 📄 time.h               # Time function declarations
│   │── 📄 config.h             # Global definitions and pin assignments
│── 📂 lib                      # External libraries used in the project
│── 📄 README.md                # Project documentation
│── 📄 .gitignore               # Ignore unnecessary files in Git
🛠 Features
📡 Real-time monitoring: Temperature, humidity, and pH sensors.
🔄 Automatic & Manual Mode: Controls electric blanket, humidifier, fan, and UV light.
📊 Graphical interface: Touchscreen UI with real-time data and adjustable parameters.
🔔 Alarm system: Alerts for temperature, humidity, and water level issues.
💾 SD card logging: Saves temperature, humidity, and alarm logs for analysis.
🕒 RTC support: Keeps track of time even if power is lost.
🔌 Hardware Used
Component	Model/Type	Purpose
Microcontroller	Arduino Mega	Main controller
Display	3.6" TFT LCD	User interface
Temp/Humidity Sensor	DHT21	Environment monitoring
pH Sensor	DFRobot pH Meter V1.1	pH measurement
RTC Module	DS3231	Keeps track of time
SD Card Module	SPI-based SD	Data logging
Water Level Sensor	Float switch	Detects low water level
Actuators	Relay-controlled devices	Heating, humidification, ventilation, UV
🔧 Installation
1️⃣ Upload the firmware
Open main.ino in the Arduino IDE.
Ensure you have the required libraries installed.
Select Arduino Mega as the board.
Upload the code.
2️⃣ Wiring & Connections
Connect the sensors and actuators according to the config.h pin definitions.
Ensure the SD card module is connected correctly.
3️⃣ Operating Modes
Auto Mode: Devices operate based on temperature, humidity, and time settings.
Manual Mode: Manually control individual components.
pH Calibration: Interactive UI for calibrating the pH probe.
📸 Screenshots

(Replace this with actual images of your project)

🗂 Code Organization
ui.cpp → Handles UI drawing (buttons, screens).
touch.cpp → Handles touchscreen interactions.
control.cpp → Controls actuators based on Auto/Manual mode.
sensors.cpp → Reads temperature, humidity, and pH values.
alarms.cpp → Checks for alarm conditions.
sdcard.cpp → Handles data logging and settings storage.
calibration.cpp → Manages pH calibration.
time.cpp → Handles RTC and simulated time.
config.h → Stores global settings and pin definitions.
📦 Required Libraries
Install these libraries via Arduino Library Manager (Sketch → Include Library → Manage Libraries):

Adafruit_GFX
MCUFRIEND_kbv
TouchScreen
SD
SPI
DHT
RTClib
math.h
📄 To-Do List
✅ Main UI & Navigation
✅ Auto & Manual Control Modes
✅ Sensor Integration
✅ Alarm System
✅ pH Calibration
✅ SD Card Logging
🔲 Graphical Data Visualization
🔲 WiFi Integration (ESP32 Future Upgrade)

📜 License
This project is open-source under the MIT License. Feel free to modify and share!

