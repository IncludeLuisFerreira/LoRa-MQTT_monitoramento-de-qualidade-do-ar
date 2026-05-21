/*
  MQTT MoT LoRa | WissTek IoT
  Versão 1.0: Luís Felipe Ferreira e Maria Eduarda
*/

//=======================================================================
//                     1 - Bibliotecas
//=======================================================================
#include <SPI.h>  
#include <LoRa.h>
#include <WiFi.h>         // Necessário para se conectar na internet
#include <PubSubClient.h> // Necessário para o protocolo MQTT

#include "secrets.h"      // Arquivo onde estarão declaradas as senhas e IP do servidor

//=======================================================================
//                     2 - Variáveis e Mapeamento
//=======================================================================
// As variávies utlizadas estão no arquivo de bibliotecas

// ============= Pinagem na placa da PK-LoRa da ligação do RFM95 com o ESP32
#define SCK_PIN    5
#define MISO_PIN  19
#define MOSI_PIN  27
#define NSS_PIN   18
#define RST_PIN   14
#define DIO0_PIN  26

// ============= CAMADA FÍSICA ============= 
// Parâmetros do LoRa
#define FREQUENCY_IN_HZ       915E6   // LoRa Frequency
#define txPower               17              // TX power in dBm, defaults to 17
#define spreadingFactor       7        // ranges from 6-12,default 7
#define signalBandwidth       125E3    // signal bandwidth in Hz
#define codingRateDenominator 8  // denominator of the coding rate

// Pinos de Saída Digitais (Opcional, mas útil para debug no Gateway)
#define LED_VERMELHO_PIN 15
#define LED_VERDE_PIN     4

// ============== Variáveis usadas no código do gateway ==============
int RSSI_dBm_UL; // Variável com a potência rádio recebida (RSSI) no UL em dBm medida pelo RFM95
int RSSI_UL; // Variável de mapeamento da RSSI medida em um valor de 0 a 255 para colocar no pacote UL
float SNR_UL; // Variável com a relação sinal ruído de DL
int SNR_UL_inteiro; // Variável inteira para enviar a SNR, que será convertida para a SNR original no Python

// ============== CAMADA MAC ==============
#define TAMANHO_PACOTE 52
byte PacoteDL[TAMANHO_PACOTE];
byte PacoteUL[TAMANHO_PACOTE];
int ID_gateway;    // Variável com o ID_gateway que estará no pacote de DL byte 10

// ============== Variáveis para a comunicação WIFI ==============

WiFiClient gatewayClient;
PubSubClient client(gatewayClient);


//=======================================================================
//                     3 - Setup de inicialização
//=======================================================================
void setup() {

  //================= INICIALIZA SERIAL
  Serial.begin(115200);

   // Define as funções como OUTPUT dos pinos dos LEDs
  pinMode(LED_VERMELHO_PIN, OUTPUT);
  pinMode(LED_VERDE_PIN, OUTPUT);
  
  // Garante que os LEDs iniciem desligados
  digitalWrite(LED_VERMELHO_PIN, LOW);
  digitalWrite(LED_VERDE_PIN, LOW);

  //-------------------- INICIALIZAÇÃO MÓDULO RF95
  
  // 1. Remapeia o barramento SPI para os pinos do Kit PKLORa
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, NSS_PIN);

  // 2. Informa à biblioteca LoRa os pinos de controle
  LoRa.setPins(NSS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(FREQUENCY_IN_HZ)) {
    Serial.println("Erro ao iniciar LoRa");
  }

  //-------------------- CONECTA O WIFI -------------------- //
  WiFi.begin(ssid, password);

  Serial.print("Conectando ao Wifi");
  
  // Loop enquanto não se conectar no WIFI
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //-------------------- CONFIGURA CONEXÃO COM SERVIDOR MQTT -------------------- //
  Serial.print("Conectando ao Broker MQTT");

  // Informa à biblioteca o endereço (IP ou URL) e a porta do servidor MQTT que ela deve usar
  client.setServer(mqtt_server, mqtt_port);

  // Gera um ID único para este ESP32 (ex: "ESP32Client_4829"), evitando que ele caia caso outro ESP32 use o mesmo nome no broker público
  String clientId = "ESP32Client_" + String(random(0, 9999));
  
  // Cria um loop que ficará rodando ENQUANTO o ESP32 não estiver efetivamente conectado ao Broker
  while (!client.connected()) {
    // Tenta realizar a conexão usando o ID único gerado.
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado com sucesso!");
      client.subscribe(topic_DL);     // Se increve no topico
    } else {
      // Mensagens de erro caso conexão falhe
      Serial.print("Falhou, estado: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 2 segundos...");
      delay(2000);
    }
  }

  client.setCallback(Phy_MQTT_receive_DL);  // Função que será chamada caso nova mensagem
  
  Serial.println("Módulo LoRa iniciado normalmente");
  LoRa.setTxPower(txPower); 
  LoRa.setSpreadingFactor(spreadingFactor); 
  LoRa.setSignalBandwidth(signalBandwidth); 
  LoRa.setCodingRate4(codingRateDenominator); 
  
  Serial.println("Gateway LoRa Inicializado com Sucesso!");
  digitalWrite(LED_VERDE_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_VERDE_PIN, LOW);
}

//=======================================================================
//                     4 - Loop de repetição
//=======================================================================
void loop() {
  
  // Mantém conexão viva
  if (client.connected()) {
    client.loop();
  }

  Phy_radio_receive_UL(); // Verifica se recebeu pacote do nó sensor
}