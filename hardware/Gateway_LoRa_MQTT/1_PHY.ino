//======================= PACOTE DOWN LINK - RECEBIDO DO MQTT E ENVIADO AO NÓ SENSOR ===========================

// Callback que escuta o Broker MQTT no tópico topic_DL
void Phy_MQTT_receive_DL(char* topic, byte* payload, unsigned int length) {
  
  Serial.print("[MQTT] Mensagem recebida no tópico: ");
  Serial.println(topic);
  
  // Descarta pacotes fora do tamanho
  if (length != TAMANHO_PACOTE) { 
    Serial.println("Pacote inválido: 'Tamanho incorreto', descartando...");
    return;
  }

  // Preenche o buffer de Downlink com os bytes recebidos do Broker
  for (unsigned int i = 0; i < TAMANHO_PACOTE; i++) {
    PacoteDL[i] = payload[i];
  }

  PacoteDL[10] = ID_GATEWAY;

  Serial.println("[MQTT -> LoRa] Repassando pacote DL via rádio...");
  Phy_radio_send_DL();
}

//========================= ENVIA PACOTE DL PARA NÓ SENSOR ATRAVÉS DO RF95 ===========================
void Phy_radio_send_DL() {
  LoRa.beginPacket();
  for (int i = 0; i < TAMANHO_PACOTE; i++) {
    LoRa.write(PacoteDL[i]);
  }
  LoRa.endPacket();                     
 
  // Feedback 
  digitalWrite(LED_VERMELHO_PIN, HIGH);
  delay(50);                            
  digitalWrite(LED_VERMELHO_PIN, LOW);   
}

//======================= RECEBE PACOTE UL LINK - VINDO DO NÓ SENSOR ===========================
void Phy_radio_receive_UL() {
  uint8_t packetSize = LoRa.parsePacket();
  if (packetSize > 0) {
    if (packetSize >= TAMANHO_PACOTE) {
      for (int i = 0; i < TAMANHO_PACOTE; i++) {
        PacoteUL[i] = LoRa.read();
      }
      
      RSSI_dBm_UL = LoRa.packetRssi();
      SNR_UL = LoRa.packetSnr();
      
      // CORREÇÃO LÓGICA 3: Chama a função correta de Uplink que calcula RSSI/SNR e publica
      Phy_MQTT_send_UL();  
    }
  }
}

//===================== PROCESSA E ENVIA O PACOTE DE UPLINK PARA O BROKER ===========================
void Phy_MQTT_send_UL() { 
  // Ajuste do RSSI para caber em um byte (0-255)
  if(RSSI_dBm_UL > -10.5) {  
   RSSI_UL = 127;
  }
  if(RSSI_dBm_UL <= -10.5 && RSSI_dBm_UL >= -74) {
   RSSI_UL = ((RSSI_dBm_UL + 74) * 2);
  }
  if(RSSI_dBm_UL < -74) {
   RSSI_UL = (((RSSI_dBm_UL + 74) * 2) + 256);
  }

  // Aloca as informações de gerência de rádio no pacote 
  PacoteUL[2] = RSSI_UL;
  SNR_UL = SNR_UL * 10;
  SNR_UL_inteiro = (int)SNR_UL;
  PacoteUL[3] = (SNR_UL_inteiro);
  
  // Transmissão do pacote via MQTT para o Broker
  if (mqtt.connected()) {
    Serial.println("[LoRa -> MQTT] Publicando pacote UL no Broker...");
    if (mqtt.publish(TOPIC_UL, PacoteUL, TAMANHO_PACOTE)) {
      Serial.println("[MQTT] Envio de UL realizado com sucesso!");
    } else {
      Serial.println("[Erro] Falha ao publicar UL no Broker.");
    }
  } else {
    Serial.println("[Erro] Não foi possível enviar UL: MQTT desconectado.");
  }

  // Feedback 
  digitalWrite(LED_VERDE_PIN, HIGH);
  delay(50);
  digitalWrite(LED_VERDE_PIN, LOW);
}