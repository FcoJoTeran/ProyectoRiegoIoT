#include <Wire.h>
#include <LiquidCrystal.h>
#include <DallasTemperature.h>
#include <OneWire.h>

// Pines de los sensores de humedad del suelo
#define HUMIDITY_SENSOR_1_PIN 32
#define HUMIDITY_SENSOR_2_PIN 33
#define HUMIDITY_SENSOR_3_PIN 34

// Pines de los sensores de temperatura DS18B20
#define TEMP_SENSOR_1_PIN 4
#define TEMP_SENSOR_2_PIN 16
#define TEMP_SENSOR_3_PIN 17

// Pines de los relés
#define RELAY_1_PIN 26
#define RELAY_2_PIN 27
#define RELAY_3_PIN 14
#define RELAY_4_PIN 12

// Pines del display 16x2
const int rs = 5, en = 18, d4 = 19, d5 = 21, d6 = 22, d7 = 23;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

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
  pinMode(RELAY_4_PIN, OUTPUT);

  // Inicializar los relés en estado apagado
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  digitalWrite(RELAY_4_PIN, LOW);

  // Iniciar los sensores de temperatura
  sensors1.begin();
  sensors2.begin();
  sensors3.begin();

  // Iniciar el display LCD
  lcd.begin(16, 2);
  lcd.print("Inicializando...");
  delay(2000);
}

void loop() {
  // Leer sensores de humedad del suelo
  int humidity1 = analogRead(HUMIDITY_SENSOR_1_PIN);
  int humidity2 = analogRead(HUMIDITY_SENSOR_2_PIN);
  int humidity3 = analogRead(HUMIDITY_SENSOR_3_PIN);

  // Imprimir valores de humedad
  Serial.print("Humidity 1: ");
  Serial.println(humidity1);
  Serial.print("Humidity 2: ");
  Serial.println(humidity2);
  Serial.print("Humidity 3: ");
  Serial.println(humidity3);

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

  // Mostrar datos en el display LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("H1: ");
  lcd.print(humidity1);
  lcd.print(" T1: ");
  lcd.print(temp1);
  lcd.setCursor(0, 1);
  lcd.print("H2: ");
  lcd.print(humidity2);
  lcd.print(" T2: ");
  lcd.print(temp2);

  // Lógica para activar electroválvulas y bomba
  controlarRiego(humidity1, RELAY_1_PIN);
  controlarRiego(humidity2, RELAY_2_PIN);
  controlarRiego(humidity3, RELAY_3_PIN);

  // Esperar un tiempo antes de la siguiente lectura
  delay(10000);
}

void controlarRiego(int humedad, int pinElectrovalvula) {
  if (humedad < 1000) { // Ajusta el umbral según tus necesidades
    digitalWrite(pinElectrovalvula, HIGH);  // Activar electroválvula
    delay(2000);  // Esperar 2 segundos
    digitalWrite(RELAY_4_PIN, HIGH);  // Activar bomba
    delay(5000);  // Riego durante 5 segundos
    digitalWrite(RELAY_4_PIN, LOW);  // Desactivar bomba
    digitalWrite(pinElectrovalvula, LOW);  // Desactivar electroválvula
  }
}
