#include "protocol.h"

void App_radio_receive_DL() {
    uint8_t request_type = PacoteDL[APP_DOWN_REQUEST_TYPE];
    uint8_t command_value = PacoteDL[APP_DOWN_COMMAND_VALUE];
    
    TYPEOF_REQUEST request = REQ_DATA_SENSORS;
    
    if (request_type == REQ_TYPE_READ_SENSORS) {
        request = REQ_DATA_SENSORS;
    }
    else if (request_type == REQ_TYPE_CHANGE_SLEEPTIME) {
        sleep_time_ms = command_value * 1000UL;
        Serial.print("Sleep time alterado para: ");
        Serial.print(command_value);
        Serial.println(" segundos");
        request = REQ_CHANGE_PARAM;
    }
    else if (request_type == REQUEST_CHANGE_INTERVAL) {  // Você precisa definir REQUEST_CHANGE_INTERVAL em lora_packet.h
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
    PacoteUL = (uint8_t*) calloc(PACKET_SIZE, sizeof(uint8_t));
    
    if (PacoteUL == NULL) {
        Serial.println("Erro: up link null!");
        return;
    }
    
    PacoteUL[APP_UP_SENSOR_STATUS] = 0xFF;
    PacoteUL[APP_UP_POLLUTION] = (uint8_t)(sleep_time_ms / 1000);
    PacoteUL[APP_UP_HUMIDITY_HIGH] = (uint8_t)(read_interval_ms / 1000);
    PacoteUL[APP_UP_HUMIDITY_LOW] = (uint8_t)txPower;
    
    Serial.println("Confirmacao de configuracao enviada");
}

void Montar_pacote_leitura_sensor() {
    PacoteUL = (uint8_t*) calloc(PACKET_SIZE, sizeof(uint8_t)); 

    if (PacoteUL == NULL) {
        Serial.println("Erro: up link null!");
        return;
    }

    uint8_t sensor_status = 0x00;

   // ============= Leitura ZP07-MP503 (Qualidade do Ar) - 2 bits digitais
    if (sensors_enabled & 0x02) {
        uint8_t a = digitalRead(ZP07_A_PIN);
        uint8_t b = digitalRead(ZP07_B_PIN);
        uint8_t nivel = (a << 1) | b;   // 0, 1, 2 ou 3
        PacoteUL[APP_UP_POLLUTION] = nivel;
        sensor_status |= SENSOR_STATUS_SENSOR2;
        Serial.print("Poluição: ");
        Serial.print(nivel);
    } else {
        PacoteUL[APP_UP_POLLUTION] = 0xFF;  // sensor desabilitado
    }

    // ============= Leitura DHT22
    if (sensors_enabled & 0x01) {
        extern DHT dht; // declarado em sensors.cpp
        temperatura = dht.readTemperature();
        umidade = dht.readHumidity();

        if (!isnan(temperatura) && !isnan(umidade)) {
            uint16_t umidade_int = (uint16_t)(umidade * 10);
            PacoteUL[APP_UP_HUMIDITY_HIGH] = (umidade_int >> 8) & 0xFF;
            PacoteUL[APP_UP_HUMIDITY_LOW] = umidade_int & 0xFF;
            sensor_status |= SENSOR_STATUS_SENSOR1;
            Serial.print(" | Temp: ");
            Serial.print(temperatura);
            Serial.print("°C | Umid: ");
            Serial.print(umidade);
            Serial.print("%");
        } else {
            Serial.print(" | Erro DHT22");
            PacoteUL[APP_UP_HUMIDITY_HIGH] = 0xFF;
            PacoteUL[APP_UP_HUMIDITY_LOW] = 0xFF;
        }
    } else {
        PacoteUL[APP_UP_HUMIDITY_HIGH] = 0xFF;
        PacoteUL[APP_UP_HUMIDITY_LOW] = 0xFF;
    }

    PacoteUL[APP_UP_SENSOR_STATUS] = sensor_status;
    Serial.println();
}