#include <Arduino.h>
#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"
#include <liquidcrystal_i2c.h>
#include <HTTPClient.h>
#include <time.h>

// #include <FreeRTOS.h>

// Pines de los sensores y relés
#define SOIL_SENSOR_PIN1 33
#define SOIL_SENSOR_PIN2 35
#define SOIL_SENSOR_PIN3 36
#define ONE_WIRE_BUS1 14
#define ONE_WIRE_BUS2 25
#define ONE_WIRE_BUS3 26
#define RELAY_PIN_PUMP 18
#define RELAY_PIN_VALVE1 23
#define RELAY_PIN_VALVE2 17
#define RELAY_PIN_VALVE3 19

const char* ssids[] = {"AgroSmart","Econtel_Ecuador"};
const char* passwords[] = {"AgroEmp123","Platon31053105"};
const int numNetworks = sizeof(ssids) / sizeof(ssids[0]);

const char* botToken = "7310667787:AAFd3nAKGECa1NMFmxIHbuCB2kCr246lREs";
const char* ntpServer = "1.south-america.pool.ntp.org";
const long  gmtOffset_sec = 0;  // Ajusta según tu zona horaria
const int   daylightOffset_sec = 3600;  // Ajusta si es necesario
OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);
OneWire oneWire3(ONE_WIRE_BUS3);

DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);
DallasTemperature sensors3(&oneWire3);
LiquidCrystal_I2C lcd(PCF8574_ADDR_A20_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

//SemaphoreHandle_t WiFiSemaphore;

// Crear un objeto de tipo WiFiClientSecure para la conexión SSL
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

// Variables para las lecturas de los sensores
float temp1, temp2, temp3;
int soilHum1, soilHum2, soilHum3;
float humidity1_percent, humidity2_percent, humidity3_percent;
float tempThreshold = 34.0;
float humidityThreshold = 0.0;

// Función para conectar a WiFi
bool conectarWiFi2() {
  for (int i = 0; i < numNetworks; i++) {
    WiFi.begin(ssids[i], passwords[i]);
    int retryCount = 0;

    while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
      delay(500);
      Serial.print("Conectando a WiFi: ");
      Serial.println(ssids[i]);
      lcd.clear();
      lcd.print("Conectando WiFi: ");
      lcd.setCursor(0, 1);
      lcd.print(ssids[i]);
      retryCount++;
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Conectado a WiFi");
      lcd.clear();
      lcd.print("Conectado a WiFi");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      return true;  // Conectado exitosamente
    } else {
      Serial.println("No se pudo conectar a la red, intentando con la siguiente...");
      lcd.clear();
      lcd.print("Fallo en red: ");
      lcd.setCursor(0, 1);
      lcd.print(ssids[i]);
      delay(2000);  // Espera 2 segundos antes de intentar la siguiente red
    }
  }

  return false;  // No se pudo conectar a ninguna red
}

bool conecta_wifi() {
  Serial.println("\nConectando a WiFi");
  lcd.clear();
  lcd.print("Conectando a WiFi");
  //WiFi.begin();  // SSID y password ya se configuran con SmartConfig
  Serial.println(WiFi.status());


  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin();  // SSID y password ya se configuran con SmartConfig
    delay(500);
    Serial.print(".");
    lcd.clear();
    lcd.print("Conectando...");

    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
      delay(500);
      Serial.print(".");
      retryCount++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi conectado, Dirección IP:");
      Serial.print(WiFi.SSID());
      Serial.print(" , ");
      Serial.println(WiFi.localIP());
      lcd.clear();
      lcd.print("Conectado a WiFi");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      //xSemaphoreGive(WiFiSemaphore); // Desbloqueamos el semaforo
      return true;
    } else {
      Serial.println("\nNo se pudo conectar a la red WiFi. Reintentando...");
      lcd.clear();
      lcd.print("Fallo conexion");
      lcd.setCursor(0, 1);
      lcd.print("Reintentando...");
      delay(2000);  // Pausa antes de reintentar
    }
  }
  
  return false;  // Esto casi nunca se ejecutará debido al bucle while
}
// Función para conectar a WiFi
bool conectarWiFi() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();
  Serial.println("Esperando conexión SmartConfig móvil/tablet");

  int retryCount = 0;
  while (!WiFi.smartConfigDone() && retryCount < 40) {
    delay(500);
    Serial.print(".");
    retryCount++;
    if (retryCount % 20 == 0) {
      Serial.println();
      Serial.println("Esperando conexión SmartConfig móvil/tablet");
      lcd.clear();
      lcd.print("Esperando");
      lcd.setCursor(0, 1);
      lcd.print("SmartConfig...");
    }
  }

  if (WiFi.smartConfigDone()) {
    Serial.println("\nFinalizado SmartConfig.");
    lcd.clear();
    lcd.print("SmartConfig");
    lcd.setCursor(0, 1);
    lcd.print("Completado");
    delay(2000);  // Pausa para mostrar el mensaje
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi conectado, Dirección IP:");
      Serial.print(WiFi.SSID());
      Serial.print(" , ");
      Serial.println(WiFi.localIP());
      lcd.clear();
      lcd.print("Conectado a WiFi");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP());
      //xSemaphoreGive(WiFiSemaphore); // Desbloqueamos el semaforo
      return true;
    } else {
      Serial.println("\nNo se pudo conectar a la red WiFi. Reintentando...");
      lcd.clear();
      lcd.print("Fallo conexion");
      lcd.setCursor(0, 1);
      lcd.print("Reintentando...");
      delay(2000);  // Pausa antes de reintentar
      return false;
    }

  } else {
    Serial.println("\nSmartConfig falló o fue interrumpido.");
    lcd.clear(); 
    lcd.print("SmartConfig fallo");
    return false;
  }
}
 
// Función para obtener la hora local
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
void enviarComandosDisponibles(const String& chat_id) {
  String message = "Bienvenido al sistema de riego automatizado.\n";
  message += "Aquí están los comandos disponibles:\n";
  message += "/lecturas - Obtener las lecturas actuales de temperatura y humedad del suelo de todas las macetas.\n";
  message += "/estado - Mostrar los umbrales actuales de temperatura y humedad del suelo para el riego.\n";
  message += "/editartemp <valor> - Cambiar el umbral de temperatura para el riego. Ejemplo: /editarTemp 35\n";
  message += "/editarhum <valor> - Cambiar el umbral de humedad del suelo para el riego. Ejemplo: /editarHum 50\n";
  bot.sendMessage(chat_id, message, "");
}
void mostrarEnLCD() {
  lcd.clear();
  
  // Mostrar temperaturas en la primera línea
  lcd.setCursor(0, 0);
  lcd.print("T1:" + String(temp1) + " T2:" + String(temp2) + " T3:" + String(temp3));
  
  // Mostrar humididades en la segunda línea
  lcd.setCursor(0, 1);
  lcd.print("H1:" + String(humidity1_percent) + " H2:" + String(humidity2_percent) + " H3:" + String(humidity3_percent));
  
  delay(2000); // Esperar 2 segundos

  // Limpiar la pantalla
  lcd.clear();
  
  // Mostrar humididades en la primera línea
  lcd.setCursor(0, 0);
  lcd.print(" H3:" + String(humidity3_percent));

  // Mostrar temperaturas en la segunda línea
  lcd.setCursor(0, 1);
  lcd.print(" T3:" + String(temp3));
  
  delay(2000); // Esperar 2 segundos
}

bool controlarRiego(float TempRiego, float HumRiegoPorc, int valvePin, const char* valveName) {
  if (TempRiego >= tempThreshold || HumRiegoPorc < humidityThreshold) { // Ajusta el umbral de humedad según sea necesario
    digitalWrite(valvePin, LOW); // Abre la electroválvula
    Serial.print("Activo Electrovalvula \n");
    //delay(1000);
    digitalWrite(RELAY_PIN_PUMP, LOW); // Enciende la bomba
    Serial.print("Activo Bomba \n");
    return true;
  }
  else{
    digitalWrite(valvePin, HIGH); //Se cierra la electrovalvula
    return false;
  }
}
// Función para enviar los datos al servidor remoto
/* void enviarDatosAlServidor() {
  if (WiFi.status() == WL_CONNECTED) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }

    HTTPClient http;
    http.begin("http://<IP_SERVIDOR>:9191/sensor-data");  // Asegúrate de usar la IP correcta
    http.addHeader("Content-Type", "application/json");

    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);

    String jsonPayload = "{\"timestamp\":\"" + String(timestamp) + "\"," +
                         "\"temp1\":" + String(temp1) + "," +
                         "\"temp2\":" + String(temp2) + "," +
                         "\"temp3\":" + String(temp3) + "," +
                         "\"humidity1_percent\":" + String(humidity1_percent) + "," +
                         "\"humidity2_percent\":" + String(humidity2_percent) + "," +
                         "\"humidity3_percent\":" + String(humidity3_percent) + "}";

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
} */


void actualizarLecturas() {
  // Leer sensores de temperatura
  sensors1.requestTemperatures();
  sensors2.requestTemperatures();
  sensors3.requestTemperatures();

  temp1 = sensors1.getTempCByIndex(0);
  temp2 = sensors2.getTempCByIndex(0);
  temp3 = sensors3.getTempCByIndex(0);
  Serial.print("Temperature 1: ");
  Serial.println(temp1);
  Serial.print("Temperature 2: ");
  Serial.println(temp2);
  Serial.print("Temperature 3: ");
  Serial.println(temp3);

  // Leer sensores de humedad del suelo
  soilHum1 = analogRead(SOIL_SENSOR_PIN1);
  soilHum2 = analogRead(SOIL_SENSOR_PIN2);
  soilHum3 = analogRead(SOIL_SENSOR_PIN3);
  humidity1_percent = map(soilHum1, 2200, 4095, 100, 0);
  humidity2_percent = map(soilHum2, 2200, 4095, 100, 0);
  humidity3_percent = map(soilHum3, 2200, 4095, 100, 0);
  Serial.print("Soil Humidity 1: ");
  Serial.println(humidity1_percent);
  Serial.print("Soil Humidity 2: ");
  Serial.println(humidity2_percent);
  Serial.print("Soil Humidity 3: ");
  Serial.println(humidity3_percent);
}


void actualizarLecturasTask(void *pvParameters) {

  while (true) {
    //xSemaphoreTake(WiFiSemaphore, portMAX_DELAY); // Bloquea la tarea hasta que se conecte a WiFi

    actualizarLecturas();
    // enviarDatosAlServidor(); 
    // Libera el semáforo después de completar la tarea
    //xSemaphoreGive(WiFiSemaphore);// Actualiza los datos en el servidor remoto Agregado hoy 13 agosto
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Espera 2 segundos entre lecturas
  }
}

void controlarRiegoTask(void *pvParameters) {

  while (true) {
    //xSemaphoreTake(WiFiSemaphore, portMAX_DELAY); // Bloquea la tarea hasta que se conecte a WiFi

    bool planta1 = controlarRiego(temp1, humidity1_percent, RELAY_PIN_VALVE1, "Electroválvula 1");
    bool planta2 = controlarRiego(temp2, humidity2_percent, RELAY_PIN_VALVE2, "Electroválvula 2");
    bool planta3 = controlarRiego(temp3, humidity3_percent, RELAY_PIN_VALVE3, "Electroválvula 3");
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Espera 5 segundos entre controles de riego
    if(!(planta1 || planta2 || planta3)){
      digitalWrite(RELAY_PIN_PUMP, HIGH); // Apaga la bomba
    }
    // Libera el semáforo después de completar la tarea
    //xSemaphoreGive(WiFiSemaphore);
  }
}

void mostrarEnLCDTask(void *pvParameters) {

  while (true) {
    //xSemaphoreTake(WiFiSemaphore, portMAX_DELAY); // Bloquea la tarea hasta que se conecte a WiFi

    mostrarEnLCD();
    // Libera el semáforo después de completar la tarea
    //xSemaphoreGive(WiFiSemaphore);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // Espera 1 segundo entre actualizaciones del LCD
  }
}

void setup() {
  // Inicialización de los pines
  Serial.begin(115200);
  pinMode(RELAY_PIN_PUMP, OUTPUT);
  pinMode(RELAY_PIN_VALVE1, OUTPUT);
  pinMode(RELAY_PIN_VALVE2, OUTPUT);
  pinMode(RELAY_PIN_VALVE3, OUTPUT);
  digitalWrite(RELAY_PIN_PUMP,HIGH);
  digitalWrite(RELAY_PIN_VALVE1,HIGH);
  digitalWrite(RELAY_PIN_VALVE2,HIGH);
  digitalWrite(RELAY_PIN_VALVE3,HIGH);
  // Inicialización de los sensores de temperatura
  sensors1.begin();
  sensors2.begin();
  sensors3.begin();
  lcd.begin(16, 2);
  lcd.backlight();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // Conexión a WiFi

  //WiFiSemaphore = xSemaphoreCreateBinary(); // Crear el semáforo

  // Crea las tareas
  /* xTaskCreate([](void *parameter) {
    while (!conectarWiFi()) {
      Serial.println("Reintentando conexión WiFi...");
      lcd.clear();
      lcd.print("Fallo en red WiFi");
      delay(5000);  // Pausa antes de reintentar la conexión
    }
    vTaskDelete(NULL);  // Termina la tarea de conexión WiFi
  }, "Tarea Conectar WiFi", 4096, NULL, 1, NULL); */

  /* if(!conectarWiFi2()) {
    Serial.println("No se pudo conectar a ninguna red WiFi");
    lcd.clear();
    lcd.print("Fallo en red WiFi");
  } */

  //SMARTCONFIG
  while(!conectarWiFi()) {
    Serial.println("No se pudo conectar a ninguna red WiFi");
    lcd.clear();
    lcd.print("Fallo en red WiFi");
    delay(2000);  // Pausa antes de reintentar la conexión
  }

  // Conexión segura al servidor de Telegram
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  client.setInsecure(); 
  
  // Crear tareas de FreeRTOS
  xTaskCreate(actualizarLecturasTask, "Actualizar Lecturas", 2048, NULL, 1, NULL);
  xTaskCreate(controlarRiegoTask, "Controlar Riego", 2048, NULL, 1, NULL);
  xTaskCreate(mostrarEnLCDTask, "Mostrar en LCD", 2048, NULL, 1, NULL);
}


void enviarMensaje(float TempRiego, float HumRiegoPorc, int valvePin, const char* valveName, const String& chat_id) {
  if (TempRiego >= tempThreshold || HumRiegoPorc <= humidityThreshold){
    bot.sendMessage(chat_id, String("Activando ") + valveName, "");
    delay(1000);
    bot.sendMessage(chat_id, "Activando Bomba", "");
    delay(5000); // Riega durante 5 segundos
    bot.sendMessage(chat_id, String("Desactivando ") + valveName, "");
    delay(500);
    bot.sendMessage(chat_id, "Desactivando Bomba", "");
  }
}

void handleNewMessages(int messageIndex) {
  String chat_id = String(bot.messages[messageIndex].chat_id);
  String text = bot.messages[messageIndex].text;

  if (text == "/start") {
    enviarComandosDisponibles(chat_id);
  } else if (text == "/lecturas") {
    actualizarLecturas(); // Actualizar las lecturas antes de enviar el mensaje
    String message = "Lecturas de sensores:\n";
    message += "Cultivo 1 - Temp: " + String(temp1) + "C, Humedad del Suelo: " + String(humidity1_percent) + "%\n";
    message += "Cultivo 2 - Temp: " + String(temp2) + "C, Humedad del Suelo: " + String(humidity2_percent) + "%\n";
    message += "Cultivo 3 - Temp: " + String(temp3) + "C, Humedad del Suelo: " + String(humidity3_percent) + "%";
    bot.sendMessage(chat_id, message, "");
  } else if (text == "/estado") {
    String message = "Umbrales de riego actuales:\n";
    message += "Temperatura: " + String(tempThreshold) + "C\n";
    message += "Humedad del suelo: " + String(humidityThreshold) + "%";
    bot.sendMessage(chat_id, message, "");
  } else if (text.startsWith("/editartemp ")) {
    float newTempThreshold = text.substring(11).toFloat();
    if (newTempThreshold > 0) {
      tempThreshold = newTempThreshold;
      bot.sendMessage(chat_id, "Umbral de temperatura actualizado a: " + String(tempThreshold) + "C", "");
    } else {
      bot.sendMessage(chat_id, "Error: El valor del umbral de temperatura no es válido.", "");
    }
  } else if (text.startsWith("/editarhum ")) {
    float newHumidityThreshold = text.substring(11).toFloat();
    if (newHumidityThreshold >= 0) {
      humidityThreshold = newHumidityThreshold;
      bot.sendMessage(chat_id, "Umbral de humedad del suelo actualizado a: " + String(humidityThreshold) + "%", "");
    } else {
      bot.sendMessage(chat_id, "Error: El valor del umbral de humedad del suelo no es válido.", "");
    }
  }
}


void loop() {
  //Verifica si la ESP32 está conectada a la red WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Conexión WiFi perdida, intentando reconectar...");
    //xSemaphoreTake(WiFiSemaphore, 0);  // Bloquea el semáforo para detener las tareas dependientes
    while (!conectarWiFi()) {
      Serial.println("Reintentando conexión WiFi...");
      delay(2000);  // Pausa antes de reintentar la conexión
    }
    //xSemaphoreGive(WiFiSemaphore);  // Libera el semáforo una vez que se reconecte
  }else{
    // Manejar mensajes entrantes de Telegram
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) { 
      Serial.println("Comando recibido");
      
      for (int i = 0; i < numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        //enviarmensaje(temp1, humidity1_percent, RELAY_PIN_VALVE1, "Valvulá 1", chat_id);
        //enviarmensaje(temp2, humidity2_percent, RELAY_PIN_VALVE2, "Valvulá 2",chat_id);
        //enviarmensaje(temp3, humidity3_percent, RELAY_PIN_VALVE3, "Valvulá 3",chat_id);
        handleNewMessages(i);
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
  }
  
 
}


