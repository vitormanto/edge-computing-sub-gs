// Global Solution - Blue Future
// Vitor Mantovani Camilo RM556537

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// Definição dos pinos dos sensores e atuadores
const int sensorTempPin = A0;      // Pino do sensor de temperatura (analógico)
const int sensorLDRPin = A1;       // Pino do sensor de luminosidade (analógico)
const int ledPins[] = {6, 7, 8, 9}; // Pinos dos LEDs para indicar luminosidade
const int ledRedPin = 10;          // Pino do LED vermelho para indicar anomalia
const int buzzerPin = 11;          // Pino do buzzer para alerta sonoro

// Inicialização do LCD com endereço e tamanho
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definições para o sistema de leitura e gravação em EEPROM
const int numReadings = 30;  // Número de leituras para média móvel
const int eepromSize = 900;  // Tamanho da EEPROM para armazenamento de dados
float tempReadings[numReadings];   // Array para armazenar leituras de temperatura
float lightReadings[numReadings];  // Array para armazenar leituras de luminosidade
int readIndex = 0;          // Índice para controle de leitura
float totalTemp = 0;        // Soma total das leituras de temperatura
float totalLight = 0;       // Soma total das leituras de luminosidade
float averageTemp = 0;      // Média móvel de temperatura
float averageLight = 0;     // Média móvel de luminosidade
int eepromIndex = 0;        // Índice para gravação na EEPROM

// Definição de caracteres personalizados para o LCD
byte bola[] = {
  B00100,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};

byte onda[] = {
  B00000,
  B00001,
  B00010,
  B00110,
  B01110,
  B11110,
  B11111,
  B11111
};

void setup() {
  // Inicialização do LCD
  lcd.init();
  lcd.backlight();
  
  // Criação de caracteres personalizados no LCD
  lcd.createChar(0, bola);
  lcd.createChar(1, onda);
  
  // Configuração inicial do display
  lcd.setCursor(0, 0);
  lcd.print("  Blue  Future");
  lcd.setCursor(0, 1);
  lcd.write(byte(1)); // Exibe o caractere personalizado de onda
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  lcd.write(byte(1));
  
  // Configuração dos pinos dos LEDs e buzzer
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(ledRedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  // Inicialização da comunicação serial para depuração
  Serial.begin(9600);
  
  // Inicialização dos arrays de leitura com zeros
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    tempReadings[thisReading] = 0;
    lightReadings[thisReading] = 0;
  }
  
  // Atraso inicial para estabilização dos componentes
  delay(2000);
  
  // Limpa o LCD antes de começar a exibir dados
  lcd.clear();
}

// Função para ler temperatura do sensor
float readTemperature() {
  int analogValue = analogRead(sensorTempPin);
  float voltage = analogValue * (5.0 / 1023.0); // Converte a leitura analógica para tensão
  float temperatureC = (voltage - 0.5) * 100.0; // Converte tensão para temperatura Celsius
  return temperatureC;
}

// Função para ler luminosidade do sensor
float readLight() {
  int analogValue = analogRead(sensorLDRPin);
  float lightPercent = (analogValue / 800.0) * 100.0; // Converte 0-800 para 0-100%
  return lightPercent;
}

void loop() {
  // Atualiza a soma total subtraindo o valor mais antigo
  totalTemp -= tempReadings[readIndex];
  totalLight -= lightReadings[readIndex];

  // Realiza nova leitura de temperatura e luminosidade
  tempReadings[readIndex] = readTemperature();
  lightReadings[readIndex] = readLight();

  // Adiciona o novo valor à soma total
  totalTemp += tempReadings[readIndex];
  totalLight += lightReadings[readIndex];

  // Atualiza o índice de leitura circularmente
  readIndex = (readIndex + 1) % numReadings;

  // Calcula a média móvel de temperatura e luminosidade
  averageTemp = totalTemp / numReadings;
  averageLight = totalLight / numReadings;

  // Exibe as leituras no LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(averageTemp);
  lcd.setCursor(10, 0);
  lcd.write(byte(0)); // Exibe o caractere personalizado de bola
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Luz: ");
  lcd.print(averageLight);
  lcd.print("%");

  // Controla os LEDs de acordo com a luminosidade medida
  for (int i = 0; i < 4; i++) {
    if (averageLight > i * 25) {
      digitalWrite(ledPins[i], HIGH);
    } else {
      digitalWrite(ledPins[i], LOW);
    }
  }

  // Verifica se a temperatura média excede o limite
  if (averageTemp > 23) {
    digitalWrite(ledRedPin, HIGH); // Aciona o LED vermelho
    tone(buzzerPin, 1000); // Emite um som de 1000 Hz no buzzer
    // Armazena os dados de temperatura e luminosidade na EEPROM
    EEPROM.put(eepromIndex, averageTemp);
    EEPROM.put(eepromIndex + sizeof(float), averageLight);
    eepromIndex += sizeof(float) * 2;
    // Verifica se o índice da EEPROM excedeu o tamanho e reinicia se necessário
    if (eepromIndex >= eepromSize - sizeof(float) * 2) {
      eepromIndex = 0;
    }

    // Exibe mensagem de anomalia na porta serial
    Serial.print("Anomalia detectada na agua! Temp: ");
    Serial.print(averageTemp);
    Serial.print(" °C, Luminosidade: ");
    Serial.print(averageLight);
    Serial.println("%");
  } else {
    // Desliga o LED vermelho e o buzzer se a temperatura estiver dentro do normal
    digitalWrite(ledRedPin, LOW);
    noTone(buzzerPin);
  }

  // Atraso para evitar atualizações muito rápidas
  delay(2000);
}
