/*
  MoT LoRa AirSense sensor Versão Zero | WissTek IoT
*/

//=======================================================================
//                     0 - Includes e Configurações
//=======================================================================
#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include "0_Config.h"

//=======================================================================
//                     2 - Variáveis e Mapeamento
//=======================================================================

// ============= Pinagem na placa da PK-LoRa da ligação do RFM95 com o ESP32
#define SCK_PIN    5
#define MISO_PIN  19
#define MOSI_PIN  27
#define NSS_PIN   18
#define RST_PIN   14
#define DIO0_PIN  26

// ============= CAMADA FÍSICA
// Parâmetros do LoRa (configuráveis remotamente)
#define FREQUENCY_IN_HZ       915E6    // LoRa Frequency
int txPower               = 17;        // TX power in dBm, defaults to 17
#define spreadingFactor       7        // ranges from 6-12,default 7
#define signalBandwidth       125E3    // signal bandwidth in Hz
#define codingRateDenominator 8        // denominator of the coding rate

// Parâmetros configuráveis
unsigned long sleep_time_ms = 0;       // Tempo de sleep em ms (0 = sem sleep)
unsigned long read_interval_ms = 10000; // Intervalo entre leituras (padrão 10s)
uint8_t sensors_enabled = 0x03;        // Sensores habilitados (0x01=DHT22, 0x02=ZP07, 0x03=Ambos)

// Váriáveis utilizadas no código
int RSSI_dBm_DL; // Variável com a potência rádio recebida (RSSI) em dBm
int RSSI_DL;     // Variável de mapeamento da RSSI em um valor de 0 a 255 para colocar no pacote
float SNR_DL;    // Variável com a relação sinal ruído
int SNR_DL_inteiro; // Variável inteira para enviar a SNR, que será convertida para a SNR original no Python

// ============== CAMADA MAC
byte *PacoteDL; // ponteiro global Down Link
byte *PacoteUL; // ponteiro global Up Link

// ============= CAMADA DE REDE
// Identificação do sensor e tamanho de pacote
int ID_sensor = 1; // Variável de iIdentificação do sensor que está no pacote de DL byte 8
int ID_gateway = 0;    // Variável com o ID_gateway que estará no pacote de DL byte 10

// ============== CAMADA DE TRANSPORTE
int contador_pkt_DL = 0; // Variável para o contador de pacotes de DL
int contador_pkt_UL = 0; // Variável para o contador de pacotes de UL

// ============= CAMADA DE APLICAÇÃO
// Pinos da PK-LoRa
// Pinos dos LEDs
#define LED_VERMELHO_PIN 15
#define LED_AMARELO_PIN   2
#define LED_VERDE_PIN     4

// Pinos de Entradas Digital (0 ou 1)
#define ZP07_A_PIN 32       // Saída A do sensor de qualidade do ar
#define ZP07_B_PIN 33       // Saída B do sensor de qualidade do ar

// Pino do DHT22
#define DHT_PIN 25          // Pino de dados do DHT22
#define DHT_TYPE DHT22      // Tipo do sensor

// Objeto DHT
DHT dht(DHT_PIN, DHT_TYPE);

// Variáveis de leitura do DHT22
float temperatura = 0.0;
float umidade = 0.0;


//=======================================================================
//                     3 - Setup de inicialização
//=======================================================================

void setup() {

    Serial.begin(115200);
    
    // --- Inicialização de I/O ---
    pinMode(LED_VERMELHO_PIN, OUTPUT);
    pinMode(LED_AMARELO_PIN, OUTPUT);
    pinMode(LED_VERDE_PIN, OUTPUT);
    
    // --- Inicialização Módulo RF95 (LoRa) ---
  
  // 1. Remapeia e inicializa o barramento SPI com os pinos do seu Kit
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, NSS_PIN);

  // 2. Informa à biblioteca LoRa os pinos de controle
  LoRa.setPins(NSS_PIN, RST_PIN, DIO0_PIN);

  if (!LoRa.begin(FREQUENCY_IN_HZ)) {
    Serial.println("Erro ao iniciar módulo RFM95");
  }
  LoRa.begin(FREQUENCY_IN_HZ);
  LoRa.setTxPower(txPower);
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
 
  // --- Inicialização DHT22 ---
  dht.begin();
  
  initializing_led_anim();
}

//=======================================================================
//                     4 - Loop de repetição
//=======================================================================

void loop() {
    Phy_radio_receive_DL();     // Recebe o pacote de Down Link
}


void initializing_led_anim() {
    // Apaga todos os leds

    digitalWrite(LED_VERMELHO_PIN, LOW);
    digitalWrite(LED_AMARELO_PIN, LOW);
    digitalWrite(LED_VERDE_PIN, LOW);

    // led acendendo em sequencia 
    
    digitalWrite(LED_VERMELHO_PIN, HIGH);
    delay(500);
    digitalWrite(LED_AMARELO_PIN, HIGH);
    delay(500);
    digitalWrite(LED_VERDE_PIN, HIGH);
    delay(500);

    // Apaga todos os led novamente
    digitalWrite(LED_VERMELHO_PIN, LOW);
    digitalWrite(LED_AMARELO_PIN, LOW);
    digitalWrite(LED_VERDE_PIN, LOW);
}