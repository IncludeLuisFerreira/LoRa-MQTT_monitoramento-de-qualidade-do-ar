#include "lora_driver.h"

bool init_lora() {
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, NSS_PIN);
    LoRa.setPins(NSS_PIN, RST_PIN, DIO0_PIN);

    if (!LoRa.begin(FREQUENCY_IN_HZ)) {
        return false;
    }

    LoRa.setTxPower(TX_POWER);
    LoRa.setSpreadingFactor(SPREADING_FACTOR);
    LoRa.setSignalBandwidth(SIGNAL_BANDWIDTH);
    LoRa.setCodingRate4(CODING_RATE_DENOMINATOR);
    return true;
}

void lora_send(uint8_t* buffer, size_t size) {
    LoRa.beginPacket();
    LoRa.write(buffer, size);
    LoRa.endPacket();
}


int lora_havePkt() {
    return LoRa.parsePacket();
}

int lora_receive(uint8_t* buffer, size_t size) {
    int i = 0;

    while (LoRa.available() && i < size) {
        buffer[i++] = LoRa.read();
    }

    return i;
}

int8_t get_last_rssi() {
    return LoRa.packetRssi();
}

float get_last_snr() {
    return LoRa.packetSnr();
}
