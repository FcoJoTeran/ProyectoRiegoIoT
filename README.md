# ProyectoRiegoIoT
Repositorio para proyecto Riego


## Configuración

Creamos un archivo que tenga las credenciales de la red y el bot de telegram. 


``` c
// include/config.h
//Aquí se definen las credenciales de la red WiFi y del bot de Telegram
#ifndef CONFIG_H
#define CONFIG_H

const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";
const char* botToken = "TU_BOT_TOKEN";

#endif // CONFIG_H

```