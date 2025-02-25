---
# FermentationChamber

## 📌 Project Overview
FermentationChamber is an Arduino-based control system designed for precise temperature, humidity, and pH regulation in fermentation processes. It features a touchscreen interface, automatic and manual control modes, data logging to an SD card, and real-time monitoring of environmental conditions.

# Fermentation Chamber Control System  
📡 Automated Fermentation & Growth Chamber - Arduino Mega-based system for monitoring and controlling temperature, humidity, pH, and more.  

---

## 📂 Project Structure  

```plaintext
FermentationChamber/
├── src/                   
│   ├── main.cpp             
│   ├── display.ino          
│   ├── lights_control.ino    
│   ├── wifi.ino             
│   ├── sensors/
│   │   ├── sensors.cpp
│   │   ├── sensors.h
│   │   ├── sensors.ino
│   ├── ui/
│   │   ├── ui.cpp
│   │   ├── touch.cpp
│   ├── alarms/
│   │   ├── alarms.cpp
│   │   ├── alarms.h
│   │   ├── alarms.ino
│   ├── control/
│   │   ├── control.cpp
│   │   ├── control.h
│   │   ├── control.ino
│   ├── sdcard/
│   │   ├── sdcard.cpp
│   │   ├── sdcard.h
│   ├── calibration/
│   │   ├── calibration.cpp
│   │   ├── calibration.h
│   ├── config.h              
├── lib/
│   ├── Adafruit_GFX/
│   ├── MCUFRIEND_kbv/
│   ├── TouchScreen/
│   ├── SD/
│   ├── SPI/
│   ├── DHT/
│   ├── RTClib/
│   ├── math/                    
├── README.md                
├── .gitignore
      
```

# Detailed Project Documentation

This document provides an in-depth explanation of the project architecture, modules, hardware and software requirements, system operation, configuration options, and future improvements. The following sections are available in English, Spanish, and Italian.

---

## English

### 1. Introduction
This project is a control system for a fermentation chamber built on Arduino. It integrates multiple modules—sensors, user interface, device control, alarms, etc.—to monitor and regulate critical parameters such as temperature and humidity, ensuring optimal conditions for fermentation.

### 2. Project Architecture
- **src/**: Contains the main source code divided into files like `main.cpp`, `display.ino`, `lights_control.ino`, `wifi.ino`, etc.
- **lib/**: Contains external libraries (e.g., Adafruit_GFX, MCUFRIEND_kbv, TouchScreen, SD, SPI, DHT, RTClib).

### 3. Module Descriptions
- **Control**: Manages the central logic and controls devices using functions such as `autoModeControl()`, `manualModeControl()`, and `updatePilotsIfChanged()`.
- **Sensors**: Reads and processes sensor data.
- **User Interface (UI)**: Handles the TFT display and user interactions.
- **Alarms**: Generates alerts based on critical conditions.
- **SD Card**: Logs sensor readings and system events.
- **Calibration**: Provides sensor calibration routines.

### 4. Hardware and Software Requirements
- **Hardware**: Arduino Mega 2560, TFT display, sensors (e.g., DHT), actuators (fan, heating pad, humidifier, UV light), SD card module, WiFi module, RTC, touch sensors.
- **Software/Libraries**: Arduino IDE, libraries like Adafruit_GFX, MCUFRIEND_kbv, TouchScreen, SD, SPI, DHT, RTClib.

### 5. Data Flow and System Operation
1. **System Startup**: `initControl()` initializes modules.
2. **Main Loop**: Sensor data is updated and control functions (`autoModeControl()` or `manualModeControl()`) are executed.
3. **Logging and Alarms**: Data is recorded on the SD card and alarms are generated if thresholds are exceeded.

### 6. Configuration and Customization
Configuration is managed in `config.h`, where thresholds, timings, and pin assignments are defined. Code and UI elements can be customized as needed.

### 7. Contribution and Development
- Clone the repository using:
- Test each module individually and report issues via GitHub.

### 8. Future Improvements
Planned enhancements include code optimization, adding WiFi connectivity for remote control and notifications, expanding sensor options, and continuous documentation updates.

---

## Español

### 1. Introducción
Este proyecto es un sistema de control para una cámara de fermentación basado en Arduino. Integra múltiples módulos—sensores, interfaz de usuario, control de dispositivos, alarmas, etc.—para monitorear y regular parámetros críticos como la temperatura y la humedad, asegurando condiciones óptimas para la fermentación.

### 2. Arquitectura del Proyecto
- **src/**: Contiene el código fuente principal dividido en archivos como `main.cpp`, `display.ino`, `lights_control.ino`, `wifi.ino`, etc.
- **lib/**: Contiene las bibliotecas externas requeridas (e.g., Adafruit_GFX, MCUFRIEND_kbv, TouchScreen, SD, SPI, DHT, RTClib).

### 3. Descripción de los Módulos
- **Control**: Gestiona la lógica central y controla los dispositivos mediante funciones como `autoModeControl()`, `manualModeControl()` y `updatePilotsIfChanged()`.
- **Sensores**: Lee y procesa datos de sensores.
- **Interfaz de Usuario (UI)**: Maneja la pantalla TFT y la interacción táctil.
- **Alarmas**: Genera alertas basadas en condiciones críticas.
- **Tarjeta SD**: Registra lecturas de sensores y eventos del sistema.
- **Calibración**: Provee rutinas para la calibración de sensores.

### 4. Requisitos de Hardware y Software
- **Hardware**: Arduino Mega 2560, pantalla TFT, sensores (por ejemplo, DHT), actuadores (ventilador, almohadilla calefactora, humidificador, luz UV), módulo para tarjeta SD, módulo WiFi, RTC, sensores táctiles.
- **Software/Bibliotecas**: Arduino IDE, bibliotecas como Adafruit_GFX, MCUFRIEND_kbv, TouchScreen, SD, SPI, DHT, RTClib.

### 5. Flujo de Datos y Funcionamiento del Sistema
1. **Inicio del Sistema**: `initControl()` inicializa los módulos.
2. **Bucle Principal (Loop)**: Se actualizan los datos de los sensores y se ejecutan las funciones de control (`autoModeControl()` o `manualModeControl()`).
3. **Registro y Alarmas**: Los datos se registran en la tarjeta SD y se generan alarmas cuando se superan los umbrales.

### 6. Configuración y Personalización
La configuración se gestiona en `config.h`, donde se definen umbrales, tiempos y asignaciones de pines. El código y la interfaz pueden personalizarse según las necesidades.

### 7. Contribución y Desarrollo
- Clona el repositorio usando:
- Prueba cada módulo por separado y reporta problemas a través de GitHub.

### 8. Mejoras Futuras
Se planea optimizar el código, añadir conectividad WiFi, expandir opciones de sensores y actualizar la documentación de forma continua.

---

## Italiano

### 1. Introduzione
Questo progetto è un sistema di controllo per una camera di fermentazione basato su Arduino. Integra vari moduli—sensori, interfaccia utente, controllo dei dispositivi, allarmi, ecc.—per monitorare e regolare parametri critici come temperatura e umidità, garantendo condizioni ottimali per la fermentazione.

### 2. Architettura del Progetto
- **src/**: Contiene il codice sorgente principale suddiviso in file come `main.cpp`, `display.ino`, `lights_control.ino`, `wifi.ino`, ecc.
- **lib/**: Contiene le librerie esterne necessarie (e.g., Adafruit_GFX, MCUFRIEND_kbv, TouchScreen, SD, SPI, DHT, RTClib).

### 3. Descrizione dei Moduli
- **Control**: Gestisce la logica centrale e controlla i dispositivi mediante funzioni come `autoModeControl()`, `manualModeControl()` e `updatePilotsIfChanged()`.
- **Sensori**: Legge ed elabora i dati dai sensori.
- **Interfaccia Utente (UI)**: Gestisce il display TFT e l’interazione touch.
- **Allarmi**: Genera allarmi basati su condizioni critiche.
- **Scheda SD**: Registra le letture dei sensori e gli eventi di sistema.
- **Calibrazione**: Fornisce routine per la calibrazione dei sensori.

### 4. Requisiti Hardware e Software
- **Hardware**: Arduino Mega 2560, display TFT, sensori (es. DHT), attuatori (ventilatore, tappetino riscaldante, umidificatore, luce UV), modulo per scheda SD, modulo WiFi, RTC, sensori touch.
- **Software/Librerie**: Arduino IDE, librerie come Adafruit_GFX, MCUFRIEND_kbv, TouchScreen, SD, SPI, DHT, RTClib.

### 5. Flusso dei Dati e Funzionamento del Sistema
1. **Avvio del Sistema**: `initControl()` inizializza i vari moduli.
2. **Loop Principale**: I dati dei sensori vengono aggiornati e vengono eseguite le funzioni di controllo (`autoModeControl()` o `manualModeControl()`).
3. **Registrazione e Allarmi**: I dati vengono registrati sulla scheda SD e vengono generati allarmi quando vengono superate le soglie.

### 6. Configurazione e Personalizzazione
La configurazione viene gestita tramite `config.h`, dove vengono definiti soglie, tempi e assegnazioni dei pin. Il codice e l’interfaccia possono essere personalizzati secondo le esigenze.

### 7. Contributi e Sviluppo
- Clona il repository usando:


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
