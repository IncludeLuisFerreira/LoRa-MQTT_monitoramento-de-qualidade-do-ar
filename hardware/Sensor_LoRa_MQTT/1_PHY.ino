void Phy_radio_receive_DL() {
    uint8_t packageSize = LoRa.parsePacket();
    if (packageSize == 0 || !is_valid_package_size(packageSize)) return;
    
    Serial.println("Tamanho: " + String(packageSize));

    PacoteDL = (byte*) calloc(packageSize, sizeof(byte));
    
    for (int i = 0; i < packageSize; i++) {
        PacoteDL[i] = LoRa.read();
    }

    digitalWrite(LED_VERDE_PIN, HIGH);
    delay(100);
    digitalWrite(LED_VERDE_PIN, LOW);

    RSSI_dBm_DL = LoRa.packetRssi();
    SNR_DL = LoRa.packetSnr();

    Mac_radio_receive_DL(); 
}

void Phy_radio_send_UL() {
    // Garante alocação
    if (PacoteUL == NULL) {
        PacoteUL = (byte*) calloc(20, sizeof(byte));
    }

    // RSSI
    if(RSSI_dBm_DL > -10.5) {
        RSSI_DL = 127;
    }
    else if(RSSI_dBm_DL >= -74) {
        RSSI_DL = ((RSSI_dBm_DL + 74) * 2);
    }
    else {
        RSSI_DL = (((RSSI_dBm_DL + 74) * 2) + 256);
    }

    PacoteUL[0] = RSSI_DL;
    SNR_DL = SNR_DL * 10;
    SNR_DL_inteiro = (int)SNR_DL;
    PacoteUL[1] = (SNR_DL_inteiro >> 8) & 0xFF;
    PacoteUL[2] = SNR_DL_inteiro & 0xFF;

    LoRa.beginPacket();
    for (int i = 0; i < 20; i++) {
        LoRa.write(PacoteUL[i]);
    }
    LoRa.endPacket();
    
    digitalWrite(LED_VERMELHO_PIN, HIGH);
    delay(100);
    digitalWrite(LED_VERMELHO_PIN, LOW);
    
    // Libera memória após enviar
    free(PacoteUL);
    PacoteUL = NULL;
}