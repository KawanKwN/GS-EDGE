
PROJETO: Smart Office Monitor - Global Solutions 2025
TEMA: O Futuro do Trabalho (Saúde e Bem-Estar)

INTEGRANTES:
- Kawan Oliveira Amorim 562197
- Alana Vieira Batista Rm563796

DESCRIÇÃO:
Este firmware implementa um sistema IoT baseado em ESP32 com arquitetura de 
multitarefa (não-bloqueante).
1. COLETA: Lê sensores de ambiente (DHT), ergonomia (Ultrassom) e biométricos (Potenciômetro).
2. PROCESSAMENTO (EDGE): Analisa os dados localmente e aciona alertas imediatos (LED/Buzzer).
3. TELEMETRIA: Envia os dados via requisições HTTP GET para a nuvem (ThingSpeak).

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// --- Definição de Hardware e Pinos ---
#define DHTPIN 15        // Sensor de Temperatura/Umidade
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define POT_PIN 34       // Potenciômetro (Simulador de Sensor Cardíaco)
#define TRIGGER_PIN 5    // Ultrassom Trigger
#define ECHO_PIN 18      // Ultrassom Echo
#define ALERT_LED_PIN 2  // LED de alerta
#define BUZZER_PIN 4     // Buzzer

// Configurações de Rede e IoT
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// API DO THINGSPEAK
const char* apiKey = "SB6WXWSD3LI5SHQC";    
const char* server = "http://api.thingspeak.com";

// --- Controle de tempo
unsigned long lastTime = 0;
unsigned long timerDelay = 15000; // Envio a cada 15s

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(POT_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(ALERT_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(ALERT_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" conectado com sucesso!");
}

void loop() {
  // BLOCO 1: EDGE COMPUTING – alerta imediato ao usuário

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) { h = 0; t = 0; }

  int potValue = analogRead(POT_PIN);
  int heartRate = map(potValue, 0, 4095, 60, 150);

  long duracao, distancia;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duracao = pulseIn(ECHO_PIN, HIGH);
  distancia = (duracao / 2) * 0.0344;

  // Lógica de decisão
  if (heartRate > 110) {
    digitalWrite(ALERT_LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000, 100);
  }
  else if (distancia < 30) {
    digitalWrite(ALERT_LED_PIN, HIGH);
    tone(BUZZER_PIN, 500, 100);
  }
  else if (t > 35 || h < 30) {
    digitalWrite(ALERT_LED_PIN, HIGH);
    tone(BUZZER_PIN, 1500, 100);
  }
  else {
    digitalWrite(ALERT_LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }

  // BLOCO 2: TELEMETRIA – envio ao ThingSpeak
  
  if ((millis() - lastTime) > timerDelay) {

    Serial.println("\n--- ENVIANDO TELEMETRIA ---");
    Serial.printf("BPM: %d | Dist: %ldcm | Temp: %.1fC | Umid: %.1f%%\n",
      heartRate, distancia, t, h
    );

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String url = String(server) + "/update?api_key=" + apiKey +
                   "&field1=" + String(t) +
                   "&field2=" + String(h) +
                   "&field3=" + String(heartRate) +
                   "&field4=" + String(distancia);

      http.begin(url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        Serial.printf("Sucesso! HTTP Code: %d\n", httpCode);
      } else {
        Serial.printf("Falha no envio. HTTP Code: %d\n", httpCode);
      }
      http.end();
    } else {
      Serial.println("Erro: WiFi desconectado.");
    }

    lastTime = millis();
  }

  delay(50);
}
