#include <Arduino.h>
#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"
#include <liquidcrystal_i2c.h>
#include <HTTPClient.h>
#include "esp_timer.h"

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

const char* ssid[] = {"Econtel_Ecuador", "Backup_SSID"};
const char* password[] = {"Platon31053105", "Backup_Password"};
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

hw_timer_t *timer = NULL;
volatile bool shouldReadTemp = false;

void IRAM_ATTR onTimer() {
  shouldReadTemp = true;
}

void IRAM_ATTR handleSoilSensor1() {
  humidity1_percent = map(analogRead(SOIL_SENSOR_PIN1), 2200, 4095, 100, 0);
  controlarRiego(humidity1_percent, RELAY_PIN_VALVE1, "Válvula 1");
}

void IRAM_ATTR handleSoilSensor2() {
  humidity2_percent = map(analogRead(SOIL_SENSOR_PIN2), 2200, 4095, 100, 0);
  controlarRiego(humidity2_percent, RELAY_PIN_VALVE2, "Válvula 2");
}

void IRAM_ATTR handleSoilSensor3() {
  humidity3_percent = map(analogRead(SOIL_SENSOR_PIN3), 2200, 4095, 100, 0);
  controlarRiego(humidity3_percent, RELAY_PIN_VALVE3, "Válvula 3");
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


bool connectWiFi() {
  for (int i = 0; i < 2; i++) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conectando a:");
    lcd.setCursor(0, 1);
    lcd.print(ssid[i]);
    WiFi.begin(ssid[i], password[i]);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(1000);
      Serial.print("Conectando a ");
      Serial.print(ssid[i]);
      Serial.println("...");
      attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Conectado a:");
      lcd.setCursor(0, 1);
      lcd.print(ssid[i]);
      delay(2000);
      return true;
    }
  }

  return false;
}


void setup() {
  // Inicialización de los pines
  Serial.begin(115200);
  pinMode(RELAY_PIN_PUMP, OUTPUT);
  pinMode(RELAY_PIN_VALVE1, OUTPUT);
  pinMode(RELAY_PIN_VALVE2, OUTPUT);
  pinMode(RELAY_PIN_VALVE3, OUTPUT);

   // Configurar pines de interrupción
  pinMode(SOIL_SENSOR_PIN1, INPUT);
  pinMode(SOIL_SENSOR_PIN2, INPUT);
  pinMode(SOIL_SENSOR_PIN3, INPUT);
  attachInterrupt(digitalPinToInterrupt(SOIL_SENSOR_PIN1), handleSoilSensor1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SOIL_SENSOR_PIN2), handleSoilSensor2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SOIL_SENSOR_PIN3), handleSoilSensor3, CHANGE);
  

  // Inicialización de los sensores de temperatura 
  sensors1.begin();
  sensors2.begin();
  sensors3.begin();

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 5000000, true);
  timerAlarmEnable(timer);
  
  // Conexión a WiFi
  if (!connectWiFi()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No se pudo");
    lcd.setCursor(0, 1);
    lcd.print("conectar a WiFi");
    while (true) {
      delay(1000);
    }
  }

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
  // Verifica si es momento de actualizar las lecturas
  if (shouldReadTemp) {
    shouldReadTemp = false; // Restablece el indicador
    actualizarLecturas(); // Actualiza las lecturas de los sensores

    // Mostrar las lecturas en el LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp1:");
    lcd.print(temp1);
    lcd.setCursor(0, 1);
    lcd.print("Hum1:");
    lcd.print(humidity1_percent);
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp2:");
    lcd.print(temp2);
    lcd.setCursor(0, 1);
    lcd.print("Hum2:");
    lcd.print(humidity2_percent);
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp3:");
    lcd.print(temp3);
    lcd.setCursor(0, 1);
    lcd.print("Hum3:");
    lcd.print(humidity3_percent);
    delay(2000);

    // Controlar el riego basado en las lecturas de humedad
    controlarRiego(humidity1_percent, RELAY_PIN_VALVE1, "Válvula 1");
    controlarRiego(humidity2_percent, RELAY_PIN_VALVE2, "Válvula 2");
    controlarRiego(humidity3_percent, RELAY_PIN_VALVE3, "Válvula 3");
  }

  // Manejar los comandos de Telegram
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    Serial.println("Comando recibido");
    for (int i = 0; i < numNewMessages; i++) {
      handleNewMessages(i);
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  delay(100);
}
