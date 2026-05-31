#include "lora_driver.h"
#include "protocol.h"

int8_t RSSI_DBM_DL = 0;
float SNR_DL = 0.0;

void Phy_radio_receive_DL() {
    RSSI_DBM_DL = get_last_rssi();
    SNR_DL = get_last_snr();
    
    Mac_radio_receive_DL(PacoteDL);
}

void Phy_radio_send_UL(uint8_t* buffer) {
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
    
    int8_t snr_encoded = (int8_t)(SNR_DL * 10);
    buffer[PHYSICAL_SNR_DL] = snr_encoded;
    
    buffer[PHYSICAL_HARDWARE_TYPE] = DEFAULT_HARDWARE_TYPE;
    buffer[PHYSICAL_BATTERY_LEVEL] = 100;  // exemplo
    
    lora_send(buffer, PACKET_SIZE);
}