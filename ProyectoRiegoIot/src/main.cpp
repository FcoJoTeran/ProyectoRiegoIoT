#include <Arduino.h>
#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

// Pines de los sensores y relés
#define SOIL_SENSOR_PIN1 32
#define SOIL_SENSOR_PIN2 33
#define SOIL_SENSOR_PIN3 34

#define ONE_WIRE_BUS1 14
#define ONE_WIRE_BUS2 27
#define ONE_WIRE_BUS3 26

#define RELAY_PIN_PUMP 25
#define RELAY_PIN_VALVE1 13
#define RELAY_PIN_VALVE2 12
#define RELAY_PIN_VALVE3 14

OneWire oneWire1(ONE_WIRE_BUS1);
OneWire oneWire2(ONE_WIRE_BUS2);
OneWire oneWire3(ONE_WIRE_BUS3);

DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);
DallasTemperature sensors3(&oneWire3);

// Crear un objeto de tipo WiFiClientSecure para la conexión SSL
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

// Variables para las lecturas de los sensores
float temp1, temp2, temp3;
int soilHum1, soilHum2, soilHum3;

void setup() {
  // Inicialización de los pines
  pinMode(RELAY_PIN_PUMP, OUTPUT);
  pinMode(RELAY_PIN_VALVE1, OUTPUT);
  pinMode(RELAY_PIN_VALVE2, OUTPUT);
  pinMode(RELAY_PIN_VALVE3, OUTPUT);

  // Inicialización de los sensores de temperatura
  sensors1.begin();
  sensors2.begin();
  sensors3.begin();

  // Conexión a WiFi
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");

  // Conexión segura al servidor de Telegram
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
}

void loop() {
  // Leer sensores de temperatura
  sensors1.requestTemperatures();
  sensors2.requestTemperatures();
  sensors3.requestTemperatures();
  
  temp1 = sensors1.getTempCByIndex(0);
  temp2 = sensors2.getTempCByIndex(0);
  temp3 = sensors3.getTempCByIndex(0);

  // Leer sensores de humedad del suelo
  soilHum1 = analogRead(SOIL_SENSOR_PIN1);
  soilHum2 = analogRead(SOIL_SENSOR_PIN2);
  soilHum3 = analogRead(SOIL_SENSOR_PIN3);

  // Comprobación de las lecturas de los sensores y control de riego
  controlarRiego(soilHum1, RELAY_PIN_VALVE1);
  controlarRiego(soilHum2, RELAY_PIN_VALVE2);
  controlarRiego(soilHum3, RELAY_PIN_VALVE3);

  // Manejar los comandos de Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    Serial.println("Comando recibido");
    for (int i = 0; i < numNewMessages; i++) {
      handleNewMessages(i);
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  delay(1000);
}

void controlarRiego(int soilHum, int valvePin) {
  if (soilHum < 500) { // Ajusta el umbral de humedad según sea necesario
    digitalWrite(valvePin, LOW); // Abre la electroválvula
    digitalWrite(RELAY_PIN_PUMP, LOW); // Enciende la bomba
    delay(5000); // Riega durante 5 segundos
    digitalWrite(valvePin, HIGH); // Cierra la electroválvula
    digitalWrite(RELAY_PIN_PUMP, HIGH); // Apaga la bomba
  }
}

void handleNewMessages(int messageIndex) {
  String chat_id = String(bot.messages[messageIndex].chat_id);
  String text = bot.messages[messageIndex].text;

  if (text == "/lecturas") {
    String message = "Lecturas de sensores:\n";
    message += "Maceta 1 - Temp: " + String(temp1) + "C, Humedad del Suelo: " + String(soilHum1) + "\n";
    message += "Maceta 2 - Temp: " + String(temp2) + "C, Humedad del Suelo: " + String(soilHum2) + "\n";
    message += "Maceta 3 - Temp: " + String(temp3) + "C, Humedad del Suelo: " + String(soilHum3);
    bot.sendMessage(chat_id, message, "");
  }
}
