#include <Arduino.h>
#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"
#include <liquidcrystal_i2c.h>
#include <HTTPClient.h>

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

OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);
OneWire oneWire3(ONE_WIRE_BUS3);

DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);
DallasTemperature sensors3(&oneWire3);
LiquidCrystal_I2C lcd(PCF8574_ADDR_A20_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

// Crear un objeto de tipo WiFiClientSecure para la conexión SSL
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

// Variables para las lecturas de los sensores
float temp1, temp2, temp3;
int soilHum1, soilHum2, soilHum3;
float humidity1_percent, humidity2_percent, humidity3_percent;
float tempThreshold = 30.0;
float humidityThreshold = 30.0;

// Función para conectar a WiFi
bool conectarWiFi() {
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


void enviarComandosDisponibles(const String& chat_id) {
  String message = "Bienvenido al sistema de riego automatizado.\n";
  message += "Aquí están los comandos disponibles:\n";
  message += "/lecturas - Obtener las lecturas actuales de temperatura y humedad del suelo de todas las macetas.\n";
  message += "/estado - Mostrar los umbrales actuales de temperatura y humedad del suelo para el riego.\n";
  message += "/editartemp <valor> - Cambiar el umbral de temperatura para el riego. Ejemplo: /editarTemp 35\n";
  message += "/editarhum <valor> - Cambiar el umbral de humedad del suelo para el riego. Ejemplo: /editarHum 50\n";
  bot.sendMessage(chat_id, message, "");
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
  // Conexión a WiFi
  /* WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
    lcd.print("Conectando WiFi...");
  }
  Serial.println("Conectado a WiFi"); */

  if(!conectarWiFi()) {
    Serial.println("No se pudo conectar a ninguna red WiFi");
    lcd.clear();
    lcd.print("Fallo en red WiFi");
  }

  // Conexión segura al servidor de Telegram
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  client.setInsecure(); 

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

void controlarRiego(float TempRiego, float HumRiegoPorc, int valvePin, const char* valveName) {
  if (TempRiego >= tempThreshold || HumRiegoPorc >= humidityThreshold) { // Ajusta el umbral de humedad según sea necesario
    digitalWrite(valvePin, LOW); // Abre la electroválvula
    Serial.print("Activo Electrovalvula \n");
    // bot.sendMessage(chat_id, String("Activando ") + valveName, "");
    delay(1000);
    digitalWrite(RELAY_PIN_PUMP, LOW); // Enciende la bomba
    Serial.print("Activo Bomba \n");
    // bot.sendMessage(chat_id, "Activando Bomba", "");
    delay(5000); // Riega durante 5 segundos
    digitalWrite(valvePin, HIGH); // Cierra la electroválvula
    delay(500);
    digitalWrite(RELAY_PIN_PUMP, HIGH); // Apaga la bomba
    // bot.sendMessage(chat_id, String("Desactivando ") + valveName, "");
    // bot.sendMessage(chat_id, "Desactivando Bomba", "");
  }
}
void enviarmensaje(float TempRiego, float HumRiegoPorc, int valvePin, const char* valveName, const String& chat_id) {
  if (TempRiego >= tempThreshold || HumRiegoPorc >= humidityThreshold){
    bot.sendMessage(chat_id, String("Activando ") + valveName, "");
    delay(1000);
    bot.sendMessage(chat_id, "Activando Bomba", "");
    delay(5000); // Riega durante 5 segundos
    bot.sendMessage(chat_id, String("Desactivando ") + valveName, "");
    delay(500);
    bot.sendMessage(chat_id, "Desactivando Bomba", "");
  }
}
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
  Serial.print("Raw Humidity 1: ");
  Serial.println(soilHum1);
  Serial.print("Raw Humidity 2: ");
  Serial.println(soilHum2);
  Serial.print("Raw Humidity 3: ");
  Serial.println(soilHum3);

  humidity1_percent = map(soilHum1, 2200, 4095, 100, 0);
  humidity2_percent = map(soilHum2, 2200, 4095, 100, 0);
  humidity3_percent = map(soilHum3, 2200, 4095, 100, 0);
  Serial.print("Humidity 1: ");
  Serial.print(humidity1_percent);
  Serial.println("%");
  Serial.print("Humidity 2: ");
  Serial.print(humidity2_percent);
  Serial.println("%");
  Serial.print("Humidity 3: ");
  Serial.print(humidity3_percent);
  Serial.println("%");

  // Asegurarse de que los porcentajes estén dentro del rango de 0 a 100
  humidity1_percent = constrain(humidity1_percent, 0, 100);
  humidity2_percent = constrain(humidity2_percent, 0, 100);
  humidity3_percent = constrain(humidity3_percent, 0, 100);
}



void handleNewMessages(int messageIndex) {
  String chat_id = String(bot.messages[messageIndex].chat_id);
  String text = bot.messages[messageIndex].text;

  if (text == "/start") {
    enviarComandosDisponibles(chat_id);
  } else if (text == "/lecturas") {
    actualizarLecturas(); // Actualizar las lecturas antes de enviar el mensaje
    String message = "Lecturas de sensores:\n";
    message += "Maceta 1 - Temp: " + String(temp1) + "C, Humedad del Suelo: " + String(humidity1_percent) + "%\n";
    message += "Maceta 2 - Temp: " + String(temp2) + "C, Humedad del Suelo: " + String(humidity2_percent) + "%\n";
    message += "Maceta 3 - Temp: " + String(temp3) + "C, Humedad del Suelo: " + String(humidity3_percent) + "%";
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
    if (newHumidityThreshold > 0) {
      humidityThreshold = newHumidityThreshold;
      bot.sendMessage(chat_id, "Umbral de humedad del suelo actualizado a: " + String(humidityThreshold) + "%", "");
    } else {
      bot.sendMessage(chat_id, "Error: El valor del umbral de humedad del suelo no es válido.", "");
    }
  }
}

void loop() {
  //digitalWrite(RELAY_PIN_PUMP, LOW);
  //digitalWrite(RELAY_PIN_VALVE1, LOW);
  // digitalWrite(RELAY_PIN_VALVE2, LOW);
  // digitalWrite(RELAY_PIN_VALVE3, LOW);


  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexión WiFi perdida. Reintentando...");
    lcd.clear();
    lcd.print("Reintentando WiFi...");
    conectarWiFi();
  }

  actualizarLecturas();
  mostrarEnLCD();
  controlarRiego(temp1, humidity1_percent, RELAY_PIN_VALVE1, "Valvulá 1");
  controlarRiego(temp2, humidity2_percent, RELAY_PIN_VALVE2, "Valvulá 2");
  controlarRiego(temp3, humidity3_percent, RELAY_PIN_VALVE3, "Valvulá 3");
  String chat_id = "6130020249";
  enviarmensaje(temp1, humidity1_percent, RELAY_PIN_VALVE1, "Valvulá 1", chat_id);
  enviarmensaje(temp2, humidity2_percent, RELAY_PIN_VALVE2, "Valvulá 2",chat_id);
  enviarmensaje(temp3, humidity3_percent, RELAY_PIN_VALVE3, "Valvulá 3",chat_id);
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
 
  delay(2000);
}



