/* #include <Arduino.h>
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
 */
/* #include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <UniversalTelegramBot.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// Initialize Telegram BOT
String BOTtoken = "7310667787:AAFd3nAKGECa1NMFmxIHbuCB2kCr246lREs";  // your Bot Token (Get from Botfather)

const char* ssid = "Econtel_Ecuador";
const char* password = "Platon31053105";

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
String CHAT_ID = "6130020249";

bool sendPhoto = false;

WiFiClientSecure clientTCP;
UniversalTelegramBot bot(BOTtoken, clientTCP);

#define FLASH_LED_PIN 4
bool flashState = LOW;

//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;


void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Welcome , " + from_name + "\n";
      welcome += "Use the following commands to interact with the ESP32-CAM \n";
      welcome += "/photo : takes a new photo\n";
      welcome += "/flash : toggles flash LED \n";
      bot.sendMessage(CHAT_ID, welcome, "");
    }
    if (text == "/flash") {
      flashState = !flashState;
      digitalWrite(FLASH_LED_PIN, flashState);
      Serial.println("Change flash LED state");
    }
    if (text == "/photo") {
      sendPhoto = true;
      Serial.println("New photo request");
    }
  }
}
// Pines de los sensores de humedad del suelo
const int HUMIDITY_SENSOR_1_PIN = 36;
const int HUMIDITY_SENSOR_2_PIN = 35;
const int HUMIDITY_SENSOR_3_PIN = 33;

// Pines de los sensores de temperatura DS18B20
const int TEMP_SENSOR_1_PIN = 26;
const int TEMP_SENSOR_2_PIN = 14;
const int TEMP_SENSOR_3_PIN = 27;

// Pines de los relés
const int RELAY_1_PIN = 22;
const int RELAY_2_PIN = 23;
const int RELAY_3_PIN = 21;
const int BOMBA12V = 19;

// Configuración de OneWire y DallasTemperature
OneWire oneWire1(TEMP_SENSOR_1_PIN);
OneWire oneWire2(TEMP_SENSOR_2_PIN);
OneWire oneWire3(TEMP_SENSOR_3_PIN);

DallasTemperature sensors1(&oneWire1);
DallasTemperature sensors2(&oneWire2);
DallasTemperature sensors3(&oneWire3);

void setup() {
  Serial.begin(115200);

  // Configurar pines de relés como salida
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  pinMode(BOMBA12V, OUTPUT);

  // Inicializar los relés en estado apagado
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(BOMBA12V, LOW);

  // Iniciar los sensores de temperatura
  sensors1.begin();
  sensors2.begin();
  sensors3.begin();
}
void controlarRiego(float humedad, int pinElectrovalvula) {
  if (humedad < 30) { // Activar riego si la humedad es menor al 30%
    digitalWrite(pinElectrovalvula, HIGH);  // Activar electroválvula
    delay(1000);  // Esperar 1 segundo
    digitalWrite(BOMBA12V, HIGH);  // Activar bomba
  } else {
    digitalWrite(pinElectrovalvula, LOW);  // Desactivar electroválvula
    digitalWrite(BOMBA12V, LOW);  // Desactivar bomba
  }
}

void loop() {
  // Leer sensores de humedad del suelo
  int humidity1 = analogRead(HUMIDITY_SENSOR_1_PIN);
  int humidity2 = analogRead(HUMIDITY_SENSOR_2_PIN);
  int humidity3 = analogRead(HUMIDITY_SENSOR_3_PIN);

  // Imprimir valores de humedad originales
  Serial.print("Raw Humidity 1: ");
  Serial.println(humidity1);
  Serial.print("Raw Humidity 2: ");
  Serial.println(humidity2);
  Serial.print("Raw Humidity 3: ");
  Serial.println(humidity3);

  // Convertir las lecturas del ADC en porcentajes de humedad
  float humidity1_percent = map(humidity1, 2300, 4095, 100, 0);
  float humidity2_percent = map(humidity2, 2200, 4095, 100, 0);
  float humidity3_percent = map(humidity3, 2800, 4095, 100, 0);

  // Asegurarse de que los porcentajes estén dentro del rango de 0 a 100
  humidity1_percent = constrain(humidity1_percent, 0, 100);
  humidity2_percent = constrain(humidity2_percent, 0, 100);
  humidity3_percent = constrain(humidity3_percent, 0, 100);

  // Imprimir valores de humedad en porcentaje
  Serial.print("Humidity 1: ");
   Serial.print(humidity1_percent);
  Serial.println("%");
  Serial.print("Humidity 2: ");
   Serial.print(humidity2_percent);
  Serial.println("%");
  Serial.print("Humidity 3: ");
   Serial.print(humidity3_percent);
  Serial.println("%");

  // Solicitar temperaturas de los sensores DS18B20
  sensors1.requestTemperatures();
  float temp1 = sensors1.getTempCByIndex(0);

  sensors2.requestTemperatures();
  float temp2 = sensors2.getTempCByIndex(0);

  sensors3.requestTemperatures();
  float temp3 = sensors3.getTempCByIndex(0);

  // Imprimir valores de temperatura
  Serial.print("Temperature 1: ");
  Serial.println(temp1);
  Serial.print("Temperature 2: ");
  Serial.println(temp2);
  Serial.print("Temperature 3: ");
  Serial.println(temp3); 

  // Lógica para activar electroválvulas y bomba
  controlarRiego(humidity1_percent, RELAY_1_PIN);
  controlarRiego(humidity2_percent, RELAY_2_PIN);
  controlarRiego(humidity3_percent, RELAY_3_PIN);

  // Esperar un tiempo antes de la siguiente lectura
  delay(5000);
}
*/

#include <Arduino.h>
#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"

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
  // client.setInsecure(); // Solo para pruebas

}

void controlarRiego(int soilHum, int valvePin) {
  if (soilHum < 500) { // Ajusta el umbral de humedad según sea necesario
    digitalWrite(valvePin, LOW); // Abre la electroválvula
    Serial.print("Activo Electrovalvula ");
    Serial.print(valvePin);
    delay(1000);
    digitalWrite(RELAY_PIN_PUMP, LOW); // Enciende la bomba
    delay(5000); // Riega durante 5 segundos
    digitalWrite(valvePin, HIGH); // Cierra la electroválvula
    digitalWrite(RELAY_PIN_PUMP, HIGH); // Apaga la bomba
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
  actualizarLecturas(); // Actualizar las lecturas en cada ciclo

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
