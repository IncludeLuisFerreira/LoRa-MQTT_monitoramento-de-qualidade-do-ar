// Não precisa de #include aqui, já está no 0_Config.h

void App_radio_receive_DL() {
    uint8_t request_type = PacoteDL[REQUEST_TYPE_DL];
    uint8_t command_value = PacoteDL[COMAND_VALUE_DL];
    
    TYPEOF_REQUEST request = REQ_DATA_SENSORS;
    
    if (request_type == REQUEST_READ_SENSORS) {
        request = REQ_DATA_SENSORS;
    }
    else if (request_type == REQUEST_CHANGE_SLEEP) {
        sleep_time_ms = command_value * 1000UL;
        Serial.print("Sleep time alterado para: ");
        Serial.print(command_value);
        Serial.println(" segundos");
        request = REQ_CHANGE_PARAM;
    }
    else if (request_type == REQUEST_CHANGE_INTERVAL) {
        read_interval_ms = command_value * 1000UL;
        Serial.print("Intervalo de leitura alterado para: ");
        Serial.print(command_value);
        Serial.println(" segundos");
        request = REQ_CHANGE_PARAM;
    }
    else if (request_type == REQUEST_CHANGE_TX_POWER) {
        if (command_value >= 2 && command_value <= 20) {
            txPower = command_value;
            LoRa.setTxPower(txPower);
            Serial.print("TX Power alterado para: ");
            Serial.print(txPower);
            Serial.println(" dBm");
        }
        request = REQ_CHANGE_PARAM;
    }
    else if (request_type == REQUEST_CHANGE_SENSORS) {
        sensors_enabled = command_value & 0x03;
        Serial.print("Sensores habilitados: ");
        if (sensors_enabled & 0x01) Serial.print("DHT22 ");
        if (sensors_enabled & 0x02) Serial.print("ZP07 ");
        Serial.println();
        request = REQ_CHANGE_PARAM;
    }
    else {
        Serial.println("Erro: comando nao especificado!");
        free(PacoteDL);
        PacoteDL = NULL;
        return;
    }

    free(PacoteDL);
    PacoteDL = NULL;
    
    App_radio_send_UL(request);
}

void App_radio_send_UL(TYPEOF_REQUEST request) {
    if (request == REQ_DATA_SENSORS) {
        Montar_pacote_leitura_sensor();
    } else if (request == REQ_CHANGE_PARAM) {
        Montar_pacote_confirmacao();
    }
    
    Transp_radio_send_UL();
}

void Montar_pacote_confirmacao() {
    PacoteUL = (byte*) calloc(20, sizeof(byte));
    
    if (PacoteUL == NULL) {
        Serial.println("Erro: up link null!");
        return;
    }
    
    // Byte 16: Status de confirmação (0xFF = OK)
    PacoteUL[SENSOR_STATS_UL] = 0xFF;
    
    // Bytes 17-19: Valores atuais dos parâmetros
    PacoteUL[VALUE_POLLUTION_UL] = (uint8_t)(sleep_time_ms / 1000);
    PacoteUL[HUMIDITY_HIGH_BYTE] = (uint8_t)(read_interval_ms / 1000);
    PacoteUL[HUMIDITY_LOW_BYTE] = (uint8_t)txPower;
    
    Serial.println("Confirmacao de configuracao enviada");
}

void Montar_pacote_leitura_sensor() {

    PacoteUL = (byte*) calloc(20, sizeof(byte)); 

    if (PacoteUL == NULL) {
        Serial.println("Erro: up link null!");
        return;
    }

    uint8_t sensor_status = 0x00;

    // ============= Leitura ZP07-MP503 (Qualidade do Ar)
    if (sensors_enabled & 0x02) {
        uint8_t a = digitalRead(ZP07_A_PIN);
        uint8_t b = digitalRead(ZP07_B_PIN);
        uint8_t nivel = (a << 1) | b;
        PacoteUL[VALUE_POLLUTION_UL] = nivel;
        sensor_status |= 0x02;
        Serial.print("Poluição: ");
        Serial.print(nivel);
    } else {
        PacoteUL[VALUE_POLLUTION_UL] = 0xFF;  // Sensor desabilitado
    }

    // ============= Leitura DHT22 (Temperatura e Umidade)
    if (sensors_enabled & 0x01) {
        temperatura = dht.readTemperature();
        umidade = dht.readHumidity();

        if (!isnan(temperatura) && !isnan(umidade)) {
            uint16_t umidade_int = (uint16_t)(umidade * 10);
            PacoteUL[HUMIDITY_HIGH_BYTE] = (umidade_int >> 8) & 0xFF;
            PacoteUL[HUMIDITY_LOW_BYTE] = umidade_int & 0xFF;
            sensor_status |= 0x01;
            Serial.print(" | Temp: ");
            Serial.print(temperatura);
            Serial.print("°C | Umid: ");
            Serial.print(umidade);
            Serial.print("%");
        } else {
            Serial.print(" | Erro DHT22");
            PacoteUL[HUMIDITY_HIGH_BYTE] = 0xFF;
            PacoteUL[HUMIDITY_LOW_BYTE] = 0xFF;
        }
    } else {
        PacoteUL[HUMIDITY_HIGH_BYTE] = 0xFF;  // Sensor desabilitado
        PacoteUL[HUMIDITY_LOW_BYTE] = 0xFF;
    }

    PacoteUL[SENSOR_STATS_UL] = sensor_status;
    Serial.println();
}