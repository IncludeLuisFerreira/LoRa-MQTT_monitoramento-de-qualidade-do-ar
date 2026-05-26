/*
  AirSense Gateway v2.0 | WissTek IoT
  Bridge MQTT-LoRa transparente para protocolo MoT/TpM
  
  Funcionalidade:
  - Recebe pacotes LoRa (20 bytes TpM) do sensor
  - Envia envelope JSON com payload Base64 + metadados de rádio via MQTT
  - Recebe comandos DL via MQTT e transmite imediatamente por LoRa
  
  Hardware: PK-LoRa (ESP32 + RFM95W)
*/
#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// ============= PINAGEM PK-LoRa =============
#define SCK_PIN    5
#define MISO_PIN  19
#define MOSI_PIN  27
#define NSS_PIN   18
#define RST_PIN   14
#define DIO0_PIN  26

// ============= ID GATEWAY =============
#define ID_GATEWAY 0

// ============= PARÂMETROS LoRa =============
#define FREQUENCY_IN_HZ       915E6
#define TX_POWER              17
#define SPREADING_FACTOR      7
#define SIGNAL_BANDWIDTH      125E3
#define CODING_RATE_DENOM     8

// ============= LEDS =============
#define LED_VERMELHO_PIN 15
#define LED_VERDE_PIN     4

// ============= TAMANHO DO PACOTE TpM =============
#define TAMANHO_PACOTE 20

// ============= BUFFERS =============
static byte PacoteDL[TAMANHO_PACOTE];
static byte PacoteUL[TAMANHO_PACOTE];

// ============= VARIÁVEIS DE RÁDIO =============
int RSSI_dBm_UL;
int RSSI_UL;
float SNR_UL;
int SNR_UL_inteiro;

// ============= MQTT =============
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// ============= TIMERS =============
unsigned long lastReconnectAttempt = 0;
unsigned long lastHeartbeat = 0;
#define HEARTBEAT_INTERVAL 30000  // 30 segundos

// ============= PROTÓTIPOS =============
void setupWiFi();
bool connectMQTT();
void heartbeat();
void callbackMQTT(char* topic, byte* payload, unsigned int length);
void Phy_radio_receive_UL();
void Phy_MQTT_send_UL();
void Phy_MQTT_receive_DL(char* topic, byte* payload, unsigned int length);
void Phy_radio_send_DL();

//=======================================================================
//                     SETUP
//=======================================================================
void setup() {
  Serial.begin(115200);

  // LEDs
  pinMode(LED_VERMELHO_PIN, OUTPUT);
  pinMode(LED_VERDE_PIN, OUTPUT);

  digitalWrite(LED_VERMELHO_PIN, LOW);
  digitalWrite(LED_VERDE_PIN, LOW);

  Serial.println("\n========================================");
  Serial.println("  AirSense Gateway v2.0");
  Serial.println("  Bridge MQTT-LoRa (MoT/TpM)");
  Serial.println("========================================\n");

  // Inicializa LoRa
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, NSS_PIN);
  LoRa.setPins(NSS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(FREQUENCY_IN_HZ)) {
    Serial.println("[ERRO] Falha ao iniciar modulo LoRa!");
    while (1) {   // Sinalização do erro
      blinkLED(LED_VERMELHO_PIN, 100, -1);
      delay(100);
    }
  }

  LoRa.setTxPower(TX_POWER);
  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(SIGNAL_BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE_DENOM);
  LoRa.setSyncWord(0x12);  // Sync word padrão

  Serial.println("[OK] Modulo LoRa iniciado");
  Serial.print("     Frequencia: "); Serial.print(FREQUENCY_IN_HZ / 1E6); Serial.println(" MHz");
  Serial.print("     TX Power: "); Serial.print(TX_POWER); Serial.println(" dBm");
  Serial.print("     SF: "); Serial.println(SPREADING_FACTOR);

  // WiFi
  setupWiFi();

  // MQTT
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callbackMQTT);
  mqtt.setBufferSize(1024);  // Buffer maior para JSON envelope

  if (connectMQTT()) {
    Serial.println("[OK] Gateway inicializado com sucesso!");
    blinkLED(LED_VERDE_PIN, 500, 2);
  } else {
    Serial.println("[AVISO] Conexao MQTT pendente. Tentando no loop...");
  }
}

//=======================================================================
//                     LOOP
//=======================================================================
void loop() {
  // Reconexão MQTT se necessário
  if (!mqtt.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (connectMQTT()) {
        lastReconnectAttempt = 0;
      }
    }
  } else {
    mqtt.loop();
  }

  // Heartbeat periódico
  unsigned long now = millis();
  if (now - lastHeartbeat > HEARTBEAT_INTERVAL) {
    lastHeartbeat = now;
    heartbeat();
  }

  // Verifica pacotes LoRa (Uplink do sensor)
  Phy_radio_receive_UL();
}

//=======================================================================
//                     WIFI
//=======================================================================
void setupWiFi() {
  Serial.print("[WiFi] Conectando a ");
  Serial.print(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" [OK]");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("[WiFi] RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println(" [FALHA]");
    Serial.println("[ERRO] Nao foi possivel conectar ao WiFi!");
  }
}

//=======================================================================
//                     MQTT
//=======================================================================
bool connectMQTT() {
  if (mqtt.connected()) return true;

  Serial.print("[MQTT] Conectando ao broker ");
  Serial.print(MQTT_SERVER);
  Serial.print(":");
  Serial.print(MQTT_PORT);
  Serial.print(" ...");

  String clientId = "AirSense-GW-" + String(GATEWAY_ID);

  if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
    Serial.println(" [OK]");
    
    // Subscreve no tópico de comandos DL
    mqtt.subscribe(TOPIC_DL);
    Serial.print("[MQTT] Subscrito em: ");
    Serial.println(TOPIC_DL);

    // Publica status online (retained)
    StaticJsonDocument<256> doc;
    doc["online"] = true;
    doc["ts"] = millis() / 1000;
    doc["ip"] = WiFi.localIP().toString();
    doc["rssi_wifi"] = WiFi.RSSI();
    
    char buffer[256];
    serializeJson(doc, buffer);
    mqtt.publish(TOPIC_STATUS, buffer, true);
    
    return true;
  } else {
    Serial.print(" [FALHA: ");
    Serial.print(mqtt.state());
    Serial.println("]");
    return false;
  }
}

void heartbeat() {
  if (!mqtt.connected()) return;

  StaticJsonDocument<256> doc;
  doc["online"] = true;
  doc["uptime_s"] = millis() / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  doc["rssi_wifi"] = WiFi.RSSI();
  doc["ts"] = millis() / 1000;

  char buffer[256];
  serializeJson(doc, buffer);
  mqtt.publish(TOPIC_STATUS, buffer, true);
}

void callbackMQTT(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, TOPIC_DL) == 0) {
    Phy_MQTT_receive_DL(topic, payload, length);
  }
}
