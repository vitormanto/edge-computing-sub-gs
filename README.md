# Monitoramento de Temperatura da Água e Incidência Solar

## Descrição do Projeto
Este projeto consiste em uma solução para o desafio "Blue Future" proposto pela FIAP, focando no monitoramento e registro da temperatura da água e na incidência solar para contribuir com a proteção dos oceanos. O sistema utiliza um Arduino Uno em conjunto com sensores analógicos NTC (temperatura) e LDR (luminosidade) para realizar as medições. O monitoramento é exibido em tempo real em um display LCD e inclui um alerta visual e sonoro caso a temperatura da água ultrapasse os 23°C, indicando uma anomalia.

## Especificações do Sistema
1. **Sensores Utilizados:**
   - Sensor de Temperatura (NTC) para medir a temperatura da água.
   - Sensor de Luminosidade (LDR) para medir a incidência de luz solar.

2. **Condições de Registro:**
   - O sistema registra na memória EEPROM o valor da temperatura média (ºC) e a luminosidade média (%) sempre que a temperatura da água exceder 23°C. A média é calculada a partir de 30 leituras ao longo de 1 minuto (uma leitura a cada 2 segundos).

3. **Indicação de Luminosidade:**
   - Um gráfico de barras de LEDs mostra o nível de luminosidade (0 a 100%).

4. **Detecção de Anomalias:**
   - Ao detectar temperatura acima de 23°C, o sistema ativa um LED vermelho e emite um beep de alerta.

5. **Memória EEPROM:**
   - O programa está configurado para armazenar até 900 registros na EEPROM.

6. **Display de LCD 16x2:**
   - O display mostra a temperatura e luminosidade atuais, além de um logo gráfico personalizado.

7. **Simulação:**
   - Utilize a plataforma Wokwi ou Thinkercad para simular o funcionamento do sistema.

## Código Arduino

```
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

```

## Objetivos
- Desenvolver um sistema funcional de monitoramento e registro utilizando sensores NTC e LDR.
- Implementar um sistema de alerta visual e sonoro para indicar anomalias na temperatura da água.
- Configurar a memória EEPROM para armazenamento eficiente dos dados registrados.
- Simular e validar o projeto na plataforma Wokwi ou Thinkercad.

## Instruções de Uso
1. **Montagem do Circuito:**
   - Monte o circuito conforme o esquema de conexão fornecido.
   - Conecte o Arduino a uma fonte de alimentação ou ao computador via cabo USB.

2. **Programação do Arduino:**
   - Carregue o código fornecido neste repositório utilizando a IDE do Arduino.
   - Certifique-se de ajustar os valores conforme necessário para o seu projeto específico.

3. **Operação do Sistema:**
   - O display LCD exibirá a temperatura da água e a incidência solar atualizadas a cada segundo.
   - O gráfico de barras de LEDs mostrará a intensidade da luz solar.
   - O LED vermelho acenderá e o buzzer emitirá um som quando a temperatura da água ultrapassar 23°C, indicando um alerta de anomalia.

4. **Simulação:**
   - Utilize a plataforma Wokwi ou Thinkercad para simular o funcionamento do sistema antes da implementação física.

Este README fornece todas as informações necessárias para montar, programar e operar o sistema de monitoramento de temperatura e luminosidade dos oceanos utilizando o Arduino Uno. Certifique-se de seguir as instruções detalhadas para garantir o correto funcionamento do projeto.

---

Esse README combina a proposta do projeto com o código desenvolvido, fornecendo uma visão clara das funcionalidades, especificações e instruções para uso e implementação do sistema de monitoramento.
