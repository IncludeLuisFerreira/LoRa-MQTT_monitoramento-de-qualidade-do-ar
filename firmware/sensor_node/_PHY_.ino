#include "lora_driver.h"
#include "protocol.h"

int8_t RSSI_DBM_DL = 0;
float SNR_DL = 0.0;

void Phy_radio_receive_DL() {
    // O buffer já está em PacoteDL (alocado no loop)
    // MAC_handle_pkt na verdade é Mac_radio_receive_DL
    RSSI_DBM_DL = get_last_rssi();
    SNR_DL = get_last_snr();
    
    // Preencher os campos físicos do pacote recebido? Normalmente você lê esses valores e guarda em variáveis.
    // Mas a função abaixo chama a MAC com o buffer.
    Mac_radio_receive_DL(PacoteDL);
}

void Phy_radio_send_UL(uint8_t* buffer) {
    // Mapeia RSSI (último recebido) para um byte conforme sua fórmula
    int16_t rssi = RSSI_DBM_DL;
    uint8_t rssi_encoded;
    
    if (rssi > -10) {
        rssi_encoded = 127;
    } else if (rssi <= -10 && rssi >= -74) {
        rssi_encoded = (rssi + 74) * 2;
    } else {
        rssi_encoded = ((rssi + 74) * 2) + 256;
    }
    
    buffer[PHYSICAL_RSSI_DL] = rssi_encoded;
    
    // SNR: multiplica por 10 e converte para inteiro com sinal?
    int8_t snr_encoded = (int8_t)(SNR_DL * 10);
    buffer[PHYSICAL_SNR_DL] = snr_encoded;
    
    // Os demais campos físicos (hardware type, bateria) devem ser preenchidos
    buffer[PHYSICAL_HARDWARE_TYPE] = DEFAULT_HARDWARE_TYPE;
    buffer[PHYSICAL_BATTERY_LEVEL] = 100;  // exemplo
    
    // Envia via LoRa
    lora_send(buffer, PACKET_SIZE);
}