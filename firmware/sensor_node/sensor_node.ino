#include <Arduino.h>
#include "lora_driver.h"
#include "sensors.h"
#include "protocol.h"

// Definições globais (inicialização)
uint8_t SENSOR_ID = 1;
uint8_t GATEWAY_ID = 100;
uint32_t read_interval_ms = 30000;
uint32_t sleep_time_ms = 10000;  // exemplo
uint16_t ul_count = 0;
uint16_t dl_count = 0;
int8_t last_rssi = 0;
float last_snr = 0.0;
int8_t txPower = 17;
uint8_t sensors_enabled = 0x03;  // ambos habilitados
float temperatura = 0;
float umidade = 0;

uint8_t* PacoteUL = NULL;
uint8_t* PacoteDL = NULL;

void setup() {
    Serial.begin(115200);
    Serial.println("AirSense v3.0 - Sensor Node");

    if (!init_lora()) {
        Serial.println("Falha ao iniciar LoRa!");
        while (1);
    }

    init_sensors();
    Serial.println("Sistema inicializado.");
}

void loop() {
    if (lora_havePkt()) {  // Verifica se há pacote
        uint8_t buffer[LORA_PACKET_SIZE];
        int len = lora_receive(buffer, LORA_PACKET_SIZE);
        if (len == LORA_PACKET_SIZE) {
            // Aloca e copia para PacoteDL
            PacoteDL = (uint8_t*) malloc(LORA_PACKET_SIZE);
            memcpy(PacoteDL, buffer, LORA_PACKET_SIZE);
            Phy_radio_receive_DL();  // Processa a camada física
        }
    }
    delay(100);
}