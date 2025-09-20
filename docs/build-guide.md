# Guía de construcción

A continuación se indican los pasos para montar la Fermentation Box.

1. **Montaje del hardware**:
   - Construye la caja e instala la pantalla TFT de 3.95" y el sensor táctil.
   - Conecta los sensores DHT21, módulo RTC DS3231, bomba de agua y demás actuadores según el esquema en `hardware/`.
2. **Cargar el firmware**:
   - Abre `firmware/mega/FermentationBox.ino` con el Arduino IDE.
   - Selecciona la placa *Arduino Mega 2560* y el puerto correcto.
   - Carga el sketch en la placa.
3. **Conectar el ESP8266 (opcional)**:
   - Monta el ESP8266 como se describe en `firmware/esp8266/README.md`.
   - Carga el firmware del ESP8266 y sigue las instrucciones para configurar la red Wi-Fi.
