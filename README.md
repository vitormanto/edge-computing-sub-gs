# Monitoramento de Temperatura da Água e Incidência Solar
### Por Vitor Mantovani Camilo (RM556537)

## Descrição do Projeto
O projeto "Blue Future" proposto pela FIAP visa monitorar e registrar a temperatura da água e a incidência solar para contribuir com a proteção dos oceanos. Utilizamos um Arduino Uno em conjunto com sensores analógicos TMP (temperatura) e LDR (luminosidade) para realizar as medições. O sistema exibe as leituras em tempo real em um display LCD e emite alertas visuais e sonoros caso a temperatura da água ultrapasse 23°C, indicando uma anomalia.

## Especificações do Sistema

### Sensores Utilizados:
- Sensor de Temperatura (TMP) para medir a temperatura da água.
- Sensor de Luminosidade (LDR) para medir a incidência de luz solar.

### Condições de Registro:
- O sistema registra na memória EEPROM o valor da temperatura média (ºC) e a luminosidade média (%) sempre que a temperatura da água exceder 23°C. A média é calculada a partir de 30 leituras ao longo de 1 minuto (uma leitura a cada 2 segundos).

### Indicação de Luminosidade:
- Um gráfico de barras de LEDs mostra o nível de luminosidade (0 a 100%).

### Detecção de Anomalias:
- Ao detectar temperatura acima de 23°C, o sistema ativa um LED vermelho e emite um beep de alerta.

### Memória EEPROM:
- O programa está configurado para armazenar até 900 registros na EEPROM, garantindo o armazenamento eficiente dos dados.

### Display de LCD 16x2:
- O display mostra a temperatura e luminosidade atuais, além de um logo gráfico personalizado.

## Desafios e Aprendizados
Durante o desenvolvimento, enfrentei alguns desafios significativos, como a integração dos sensores analógicos e a gestão eficiente da memória EEPROM para armazenamento dos registros. O uso do Arduino Uno exigiu alguns ajustes no código para garantir leituras estáveis e precisas, especialmente com a média móvel das leituras de temperatura e luminosidade.

Implementar o alerta visual e sonoro para indicar anomalias foi uma parte crítica do projeto, pois envolveu o controle preciso de LEDs e do buzzer para garantir a eficácia na sinalização de condições adversas na água.

## Instruções de Uso

### Montagem do Circuito:
1. Monte o circuito conforme o esquema fornecido no simulador.
2. Conecte os sensores TMP e LDR aos pinos analógicos do Arduino Uno.
3. Conecte os LEDs, o LED vermelho e o buzzer aos pinos digitais conforme especificado no código.

### Carregamento do Código:
- Utilize a IDE do Arduino para carregar o código fornecido neste repositório.
- Ajuste os parâmetros necessários, como o endereço do LCD e os limites de temperatura conforme suas necessidades.

## Código Arduino:

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

### Operação do Sistema:
1. Após ligar o Arduino, o display LCD começará a exibir a temperatura da água e a luminosidade medida.
2. Os LEDs de indicação de luminosidade mostrarão a intensidade da luz solar em tempo real.
3. Caso a temperatura da água ultrapasse 23°C, o LED vermelho acenderá e o buzzer emitirá um som de alerta.
4. Os dados de temperatura e luminosidade durante as anomalias são registrados na EEPROM para análise posterior.

### Simulação:
- Antes da implementação física, recomenda-se simular o sistema usando plataformas como Wokwi ou Thinkercad para validar o funcionamento do circuito e do código.

## Conclusão
O projeto "Blue Future" não apenas demonstra a aplicação prática de tecnologias de monitoramento ambiental usando Arduino, mas também ressalta a importância de soluções tecnológicas na preservação dos oceanos. A combinação de sensores, display LCD, alertas e armazenamento de dados oferece uma solução robusta para monitorar e registrar condições críticas da água de forma eficiente e acessível.

Ao seguir as instruções detalhadas acima, você poderá montar, programar e operar seu próprio sistema de monitoramento ambiental, contribuindo para um futuro mais sustentável e consciente.

## Referências:
[Link do Simulador](https://www.tinkercad.com/things/gNK89yOjitm-projeto-blue-future-vfinal/editel?sharecode=yo9eASGkEtk0g_y8Zm01a2PPtgt9M5QOEWNqMJREKsU)
