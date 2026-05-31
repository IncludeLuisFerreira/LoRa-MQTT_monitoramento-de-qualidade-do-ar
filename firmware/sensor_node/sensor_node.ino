#include <Arduino.h>
#include "lora_driver.h"
#include "sensors.h"
#include "protocol.h"

// Definições globais
uint8_t SENSOR_ID = 1;
uint8_t GATEWAY_ID = 100;
uint32_t read_interval_ms = 30000;
uint32_t sleep_time_ms = 10000;
uint16_t ul_count = 0;
uint16_t dl_count = 0;
int8_t last_rssi = 0;
float last_snr = 0.0;
int8_t txPower = 17;
uint8_t sensors_enabled = 0x03;
float temperatura = 0;
float umidade = 0;

// Buffers estáticos (alocados em tempo de compilação)
uint8_t PacoteUL[PACKET_SIZE];
uint8_t PacoteDL[PACKET_SIZE];

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
    if (lora_havePkt()) {
        Serial.print("Pacote recebido");
        uint8_t buffer[PACKET_SIZE];
        int len = lora_receive(buffer, PACKET_SIZE);
        if (len == PACKET_SIZE) {
            // Copia para o buffer estático global (sem alocação)
            memcpy(PacoteDL, buffer, PACKET_SIZE);
            Phy_radio_receive_DL();   // processa o pacote (sem liberar memória)
        }
    }
}