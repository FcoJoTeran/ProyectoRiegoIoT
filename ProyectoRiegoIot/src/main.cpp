#include <Arduino.h>
#include <WiFi.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include "config.h" // Incluimos el archivo de configuración

// Creamos un objeto de tipo WiFiClientSecure para la conexión SSL
WiFiClientSecure client;
UniversalTelegramBot bot(botToken, client);

// Se definen los pines de los sensores y relés
#define DHTTYPE DHT22 // Cambia según el tipo de tu sensor DHT
#define DHTPIN1 26
#define DHTPIN2 27
#define DHTPIN3 14
#define RELAY_PIN_PUMP 32
#define RELAY_PIN_VALVE1 33
#define RELAY_PIN_VALVE2 25
#define RELAY_PIN_VALVE3 13

DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);

// Variables para los tiempos de riego
unsigned long previousMillis = 0;
const long interval = 10000; // Intervalo de riego en milisegundos

// Variables para las lecturas de los sensores
float temp1, hum1, temp2, hum2, temp3, hum3;

void setup() {
  // Inicialización de los pines
  pinMode(RELAY_PIN_PUMP, OUTPUT);
  pinMode(RELAY_PIN_VALVE1, OUTPUT);
  pinMode(RELAY_PIN_VALVE2, OUTPUT);
  pinMode(RELAY_PIN_VALVE3, OUTPUT);

  // Inicialización de los sensores DHT
  dht1.begin();
  dht2.begin();
  dht3.begin();

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
  // Leer sensores
  temp1 = dht1.readTemperature();
  hum1 = dht1.readHumidity();
  temp2 = dht2.readTemperature();
  hum2 = dht2.readHumidity();
  temp3 = dht3.readTemperature();
  hum3 = dht3.readHumidity();

  // Comprobación de las lecturas de los sensores y control de riego
  controlarRiego(temp1, hum1, RELAY_PIN_VALVE1);
  controlarRiego(temp2, hum2, RELAY_PIN_VALVE2);
  controlarRiego(temp3, hum3, RELAY_PIN_VALVE3);

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

void controlarRiego(float temp, float hum, int valvePin) {
  if (isnan(temp) || isnan(hum)) {
    Serial.println("Error al leer el sensor");
    return;
  }

  if (hum < 50.0) { // Ajusta el umbral de humedad según sea necesario
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
    message += "Maceta 1 - Temp: " + String(temp1) + "C, Hum: " + String(hum1) + "%\n";
    message += "Maceta 2 - Temp: " + String(temp2) + "C, Hum: " + String(hum2) + "%\n";
    message += "Maceta 3 - Temp: " + String(temp3) + "C, Hum: " + String(hum3) + "%";
    bot.sendMessage(chat_id, message, "");
  }
}
