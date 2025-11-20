1. Descrição do Problema

Ambientes de trabalho modernos exigem monitoramento constante de condições que afetam a saúde: temperatura, umidade, postura e nível de estresse físico. Muitas empresas não possuem ferramentas acessíveis para acompanhar esses fatores em tempo real.
O Smart Office Monitor é um sistema IoT baseado em ESP32 que coleta dados ambientais, ergonômicos e biométricos para:
alertar o usuário imediatamente em caso de risco;
registrar telemetria na nuvem para análise posterior.

2. Solução Desenvolvida

O projeto utiliza arquitetura não bloqueante, com processamento local e envio periódico para a nuvem.
Componentes monitorados:
Temperatura e Umidade (DHT22)
Distância/Postura (Ultrassom HC-SR04)
Sinal biométrico simulado (Potenciômetro → BPM)
Ações automáticas:
LED e buzzer acionados quando valores saem do limite seguro
Telemetria enviada para ThingSpeak via HTTP GET

3. Hardware Utilizado

ESP32 DevKit
Sensor DHT22
Módulo Ultrassônico HC-SR04
Potenciômetro 10k
LED
Buzzer
Jumpers

4. Estrutura do Repositório
/SmartOfficeMonitor
│
├── src/
│   └── smart_office_monitor.ino  
│
├── README.md
└── imagens/
    └── circuito.png             

5. Dependências

Instalar pelas bibliotecas do Arduino:
DHT sensor library
HTTPClient
WiFi (nativa do ESP32)
Placa utilizada: ESP32 Dev Module

6. Comunicação IoT (HTTP / MQTT)
Protocolo usado no projeto:
HTTP GET para envio de dados ao ThingSpeak.
Exemplo da requisição gerada pelo ESP32:

http://api.thingspeak.com/update?api_key=UN12UJN0ULVIQ5FC
&field1=temperatura
&field2=umidade
&field3=bpm
&field4=distancia

Por que HTTP?
Simples, rápido e compatível com ThingSpeak
Ideal para telemetria periódica

7. Como Executar o Projeto
Abra o código no Arduino IDE.

Instale as bibliotecas indicadas.
Se necessário, edite o SSID e senha do Wi-Fi.
Faça upload para o ESP32
Abra o monitor serial em 115200 baud.
Veja os dados sendo enviados e os alertas sendo acionados.

8. Simulação no Wokwi

Acesse aqui:
https://wokwi.com/projects/447986263591639041

