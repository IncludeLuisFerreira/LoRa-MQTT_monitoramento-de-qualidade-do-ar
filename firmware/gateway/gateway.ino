#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LoRa.h>
#include "config.h"
#include "lora_driver.h"
#include "tpm_protocol.h"

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensagem recebida no tópico [");
    Serial.print(topic);
    Serial.println("]");

    if (String(topic) == topic_tx) {
        // Downlink recebido via MQTT
        if (length == 20) {
            Serial.println("Retransmitindo via LoRa...");
            lora_send(payload, 20);
            
            // Publica ACK (simplificado para demonstração no firmware)
            client.publish(topic_ack, "{\"status\": \"transmitted\"}");
        }
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Tentando conexão MQTT...");
        String clientId = "AirSenseGW-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("conectado");
            client.subscribe(topic_tx);
        } else {
            Serial.print("falhou, rc=");
            Serial.print(client.state());
            Serial.println(" tentando novamente em 5 segundos");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("AirSense v3.0 - Gateway");

    if (!init_lora()) {
        Serial.println("Falha ao iniciar LoRa!");
        while (1);
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado");

    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Escuta LoRa (Uplink)
    uint8_t buffer[TPM_PACKET_SIZE];
    int rx_size = lora_receive(buffer, TPM_PACKET_SIZE, 10);
    
    if (rx_size == TPM_PACKET_SIZE) {
        Serial.println("Uplink LoRa recebido! Encaminhando para MQTT...");
        
        // Montagem do JSON (APP) - Simplificado para o firmware
        // Em produção, usar uma biblioteca JSON como ArduinoJson
        String json = "{";
        json += "\"schema_version\":\"3.0\",";
        json += "\"gateway_id\":\"gw-001\",";
        json += "\"radio\":{";
        json += "\"rssi_dbm\":" + String(get_last_rssi()) + ",";
        json += "\"snr_db\":" + String(get_last_snr()) + "";
        json += "},";
        json += "\"tpm\":{";
        json += "\"payload_hex\":\"";
        for(int i=0; i<20; i++) {
            if(buffer[i] < 0x10) json += "0";
            json += String(buffer[i], HEX);
        }
        json += "\",\"size\":20";
        json += "}}";

        client.publish(topic_rx, json.c_str());
    }
}
