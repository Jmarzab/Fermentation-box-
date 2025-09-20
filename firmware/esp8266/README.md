# Firmware ESP8266

Este directorio contiene el firmware para el módulo ESP8266 que proporcionará conectividad Wi-Fi y un API REST para el Mega.

## Funcionalidades previstas

- Servidor web ligero para mostrar estado y historial.
- Endpoints REST (`/api/status`, `/api/cmd`, `/api/history`) para obtener datos y enviar comandos.
- Integración de mDNS para acceder desde `fermbox.local`.
- Portal cautivo para configuración inicial de Wi-Fi.
