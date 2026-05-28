#include <Arduino.h>
#include "lora_driver.h"
#include "tpm_protocol.h"
#include "sensors.h"

// Configurações do Nó
uint8_t SENSOR_ID = 1;
uint8_t GATEWAY_ID = 100;
uint32_t read_interval_ms = 30000;

uint16_t ul_count = 0;
uint16_t dl_count = 0;

int8_t last_rssi = 0;
float last_snr = 0.0;

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
    // 1. Coleta de dados (APP)
    float humidity = read_humidity();
    uint8_t pollution = read_pollution();
    uint8_t status = get_sensor_status();

    // 2. Montagem do pacote (TRANSP)
    TpMUplink pkt;
    pkt.rssi_mapped = (uint8_t)(last_rssi + 164); // Exemplo de mapeamento
    pkt.snr_mapped = (uint16_t)(last_snr * 10);
    pkt.dest_id = GATEWAY_ID;
    pkt.src_id = SENSOR_ID;
    pkt.dl_count = dl_count;
    pkt.ul_count = ++ul_count;
    pkt.sensor_status = status;
    pkt.pollution_level = pollution;
    pkt.humidity_raw = (uint16_t)(humidity * 10);

    uint8_t buffer[TPM_PACKET_SIZE];
    pkt.serialize(buffer);

    // 3. Transmissão (PHY/MAC)
    Serial.printf("Enviando Uplink #%d...\n", ul_count);
    lora_send(buffer, TPM_PACKET_SIZE);

    // 4. Escuta por Downlink (RX Window)
    Serial.println("Aguardando Downlink (500ms)...");
    uint8_t rx_buffer[TPM_PACKET_SIZE];
    int rx_size = lora_receive(rx_buffer, TPM_PACKET_SIZE, 500);

    if (rx_size == TPM_PACKET_SIZE) {
        TpMDownlink dl = TpMDownlink::deserialize(rx_buffer);
        if (dl.dest_id == SENSOR_ID) {
            Serial.printf("Downlink recebido! CMD_ID: %d, TYPE: %02X, VAL: %d\n", 
                          dl.command_id, dl.request_type, dl.command_value);
            dl_count++;
            
            // Aplica comandos (APP)
            if (dl.request_type == 0x01) { // Alterar intervalo
                read_interval_ms = dl.command_value * 1000;
                Serial.printf("Novo intervalo: %d s\n", dl.command_value);
            }
        }
    }

    // 5. Deep Sleep (Simulado por delay para fins de firmware inicial)
    delay(read_interval_ms);
}
