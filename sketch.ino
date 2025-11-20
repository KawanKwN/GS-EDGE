/* --------------------------------------------------------------------------
PROJETO: Smart Office Monitor - Global Solutions 2025
TEMA: O Futuro do Trabalho (Saúde e Bem-Estar)

INTEGRANTES:
- Camila Mendonça (RM565491)
- Fernando Kenzo Sassaki Cunha (RM562343)
- Marcello Lassalla de Mello Castanho e Oliveira (RM561879)

DESCRIÇÃO TÉCNICA:
Este firmware implementa um sistema IoT baseado em ESP32 com arquitetura de 
multitarefa (não-bloqueante).
1. COLETA: Lê sensores de ambiente (DHT), ergonomia (Ultrassom) e biométricos (Potenciômetro).
2. PROCESSAMENTO (EDGE): Analisa os dados localmente e aciona alertas imediatos (LED/Buzzer).
3. TELEMETRIA: Envia os dados via requisições HTTP GET para a nuvem (ThingSpeak).
--------------------------------------------------------------------------
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

// --- Definição de Hardware e Pinos ---
#define DHTPIN 15        // Sensor de Temperatura/Umidade
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define POT_PIN 34       // Potenciômetro (Simulador de Sensor Cardíaco)
#define TRIGGER_PIN 5    // Ultrassom Trigger (Emissor)
#define ECHO_PIN 18      // Ultrassom Echo (Receptor)
#define ALERT_LED_PIN 2  // Atuador Visual
#define BUZZER_PIN 4     // Atuador Sonoro

// --- Configurações de Rede e IoT ---
const char* ssid = "Wokwi-GUEST"; 
const char* password = ""; 

// Chave de Escrita (Write API Key) do Canal ThingSpeak
const char* apiKey = "GNQJSPX3DX474LE4"; 
const char* server = "http://api.thingspeak.com"; 

// --- Variáveis de Controle de Tempo (Multitarefa) ---
// Usamos millis() para evitar o uso de delay() que travaria o processador
unsigned long lastTime = 0;
unsigned long timerDelay = 15000; // Intervalo de envio para nuvem (15s)

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Configuração dos modos dos pinos (Entrada/Saída)
  pinMode(POT_PIN, INPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(ALERT_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Estado inicial dos atuadores
  digitalWrite(ALERT_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Conexão Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" conectado com sucesso!");
}

void loop() {
  // ============================================================
  // BLOCO 1: PROCESSAMENTO EM TEMPO REAL (EDGE COMPUTING)
  // Executado continuamente para feedback instantâneo ao usuário
  // ============================================================
  
  // 1.1. Leitura do DHT22 com tratamento de erros
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) { h = 0; t = 0; }

  // 1.2. Simulação de Batimentos Cardíacos (Mapeamento Analógico)
  // Convertendo leitura de 0-4095 (12 bits) para faixa de 60-150 BPM
  int potValue = analogRead(POT_PIN); 
  int heartRate = map(potValue, 0, 4095, 60, 150); 

  // 1.3. Leitura do Sensor Ultrassônico
  long duracao, distancia;
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duracao = pulseIn(ECHO_PIN, HIGH);
  distancia = (duracao / 2) * 0.0344; // Cálculo da distância em cm

  // 1.4. Lógica de Decisão e Alertas Locais
  // Prioridade 1: Estresse Alto (>110 BPM)
  if (heartRate > 110) {
    digitalWrite(ALERT_LED_PIN, HIGH); 
    tone(BUZZER_PIN, 1000, 100); // Alerta Agudo
  }
  // Prioridade 2: Má Ergonomia (<30 cm da tela)
  else if (distancia < 30) {
    digitalWrite(ALERT_LED_PIN, HIGH);
    tone(BUZZER_PIN, 500, 100); // Alerta Grave
  }
  // Prioridade 3: Conforto Térmico (>35C ou Baixa Umidade)
  else if (t > 35.0 || h < 30.0) {
    digitalWrite(ALERT_LED_PIN, HIGH);
    tone(BUZZER_PIN, 1500, 100); // Alerta Ambiental
  }
  // Condição Normal
  else {
    digitalWrite(ALERT_LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }

  // ============================================================
  // BLOCO 2: COMUNICAÇÃO CLOUD (THINGSPEAK)
  // Executado apenas a cada 15 segundos para respeitar limites da API
  // ============================================================
  
  if ((millis() - lastTime) > timerDelay) {
    
    // Diagnóstico Serial
    Serial.println("\n--- ENVIANDO TELEMETRIA ---");
    Serial.printf("BPM: %d | Dist: %ldcm | Temp: %.1fC | Umid: %.1f%%\n", heartRate, distancia, t, h);

    if(WiFi.status() == WL_CONNECTED){
      HTTPClient http;
      
      // Montagem da URL para Requisição GET
      // Formato: http://api.thingspeak.com/update?api_key=XXX&field1=YY&field2=ZZ...
      String url = String(server) + "/update?api_key=" + apiKey + 
                   "&field1=" + String(t) + 
                   "&field2=" + String(h) + 
                   "&field3=" + String(heartRate) + 
                   "&field4=" + String(distancia);
      
      http.begin(url); // Inicia conexão
      int httpCode = http.GET(); // Envia requisição

      if (httpCode > 0) {
        Serial.printf("Sucesso! HTTP Code: %d\n", httpCode);
      } else {
        Serial.printf("Falha no envio. HTTP Code: %d\n", httpCode);
      }
      http.end(); // Encerra conexão para liberar recursos
    } else {
      Serial.println("Erro: WiFi desconectado.");
    }
    
    lastTime = millis(); // Reinicia o temporizador
  }
  
  delay(50); // Pequeno delay para estabilidade da simulação Wokwi
}
