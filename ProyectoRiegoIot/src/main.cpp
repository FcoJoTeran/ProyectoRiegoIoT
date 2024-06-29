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
#define ONE_WIRE_BUS2 27
#define ONE_WIRE_BUS3 26

#define RELAY_PIN_PUMP 22
#define RELAY_PIN_VALVE1 23
#define RELAY_PIN_VALVE2 21
#define RELAY_PIN_VALVE3 19
LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección I2C 0x27 y tamaño del LCD 16x2

const char* ssid = "Econtel_Ecuador";
const char* password = "Platon31053105";
const char* botToken = "7310667787:AAFd3nAKGECa1NMFmxIHbuCB2kCr246lREs";
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
float humidity1_percent, humidity2_percent, humidity3_percent;
String chat_id = "6130020249"; // Reemplaza con tu chat_id de Telegram
void setup() {
  // Inicialización de los pines
  Serial.begin(115200);
  pinMode(RELAY_PIN_PUMP, OUTPUT);
  pinMode(RELAY_PIN_VALVE1, OUTPUT);
  pinMode(RELAY_PIN_VALVE2, OUTPUT);
  pinMode(RELAY_PIN_VALVE3, OUTPUT);
  

  // Inicialización de los sensores de temperatura 
  sensors1.begin();
  sensors2.begin();
  sensors3.begin();
  
  // Conexión a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");

  // Conexión segura al servidor de Telegram
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  client.setInsecure(); 

}

void controlarRiego(float soilHum, int valvePin, const char* valveName) {
  if (soilHum > 30 ) { // Ajusta el umbral de humedad según sea necesario
    digitalWrite(valvePin, HIGH); // Abre la electroválvula
    Serial.print("Activo Electrovalvula \n");
    bot.sendMessage(chat_id, String("Activando ") + valveName, "");
    delay(1000);
    digitalWrite(RELAY_PIN_PUMP, HIGH); // Enciende la bomba
    Serial.print("Activo Bomba \n");
    bot.sendMessage(chat_id, "Activando Bomba", "");
    delay(5000); // Riega durante 5 segundos
    digitalWrite(valvePin, LOW); // Cierra la electroválvula
    digitalWrite(RELAY_PIN_PUMP, LOW); // Apaga la bomba
    bot.sendMessage(chat_id, String("Desactivando ") + valveName, "");
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

  if (text == "/lecturas") {
    actualizarLecturas(); // Actualizar las lecturas antes de enviar el mensaje
    String message = "Lecturas de sensores:\n";
    message += "Maceta 1 - Temp: " + String(temp1) + "C, Humedad del Suelo: " + String(humidity1_percent) + "%\n";
    message += "Maceta 2 - Temp: " + String(temp2) + "C, Humedad del Suelo: " + String(humidity2_percent) + "%\n";
    message += "Maceta 3 - Temp: " + String(temp3) + "C, Humedad del Suelo: " + String(humidity3_percent) + "%";
    bot.sendMessage(chat_id, message, "");
  }
}

void loop() {
  digitalWrite(RELAY_PIN_PUMP, LOW);
  digitalWrite(RELAY_PIN_VALVE1, LOW);
  digitalWrite(RELAY_PIN_VALVE2, LOW);
  digitalWrite(RELAY_PIN_VALVE3, LOW);

  actualizarLecturas(); // Actualizar las lecturas en cada ciclo

  // Comprobación de las lecturas de los sensores y control de riego
  controlarRiego(temp1, RELAY_PIN_VALVE1, "Valvulá 1");
  controlarRiego(temp2, RELAY_PIN_VALVE2, "Valvulá 2");
  controlarRiego(temp3, RELAY_PIN_VALVE3, "Valvulá 3");


  // Manejar los comandos de Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    Serial.println("Comando recibido");
    for (int i = 0; i < numNewMessages; i++) {
      handleNewMessages(i);
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  delay(5000);
}
