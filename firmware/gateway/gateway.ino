#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <LoRa.h>
#include "config.h"
#include "lora_driver.h"
#include "lora_packet.h"

// ========================= VERIFICAÇÕES =========================
// 1. Certifique-se de que TPM_PACKET_SIZE está definido em config.h ou lora_driver.h como 20.
// 2. O timeout de 10ms em lora_receive() pode ser muito curto para LoRa (SF=7 leva ~100ms).
//    Considere aumentar para 100-200ms.
// 3. A concatenação de strings para JSON pode causar fragmentação de memória.
//    Prefira ArduinoJson em produção.
// 4. O ACK é publicado mesmo se lora_send() falhar? O código atual não verifica retorno.
// 5. Certifique-se de que os tópicos MQTT (topic_tx, topic_ack, topic_rx) estão definidos no config.h.
// =================================================================

WiFiClient espClient;
PubSubClient client(espClient);

// Callback chamado quando uma mensagem MQTT chega em um tópico inscrito
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensagem recebida no tópico [");
    Serial.print(topic);
    Serial.println("]");

    if (String(topic) == topic_tx) {
        if (length == PACKET_SIZE) {
    
            Serial.println("Retransmitindo via LoRa...");
            bool success = lora_send(payload, PACKET_SIZE);
            if (success) {
                client.publish(topic_ack, "{\"status\": \"transmitted\"}");
            } else {
                client.publish(topic_ack, "{\"status\": \"lora_failed\"}");
            }
        } else {
            Serial.printf("Aviso: downlink ignorado, tamanho %d (esperado %d)\n", length, PACKET_SIZE);
        }
    }
}// Gerencia a reconexão ao broker MQTT
void reconnect() {
    while (!client.connected()) {
        Serial.print("Tentando conexão MQTT...");
        String clientId = "AirSenseGW-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str())) {
            Serial.println("conectado");
            client.subscribe(topic_tx);   // Inscreve-se no tópico de downlink
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

    // Inicializa o rádio LoRa com configurações definidas em lora_driver.h
    if (!init_lora()) {
        Serial.println("Falha ao iniciar LoRa!");
        while (1);  // trava o programa se LoRa não funcionar
    }

    // Conecta ao WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi conectado");

    // Configura o cliente MQTT
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Escuta pacotes LoRa (uplink dos sensores)
    uint8_t buffer[PACKET_SIZE];   // <-- SUBSTITUÍDO
    int rx_size = lora_receive(buffer, PACKET_SIZE, 100);  // timeout aumentado para 100ms (recomendado)
    
    if (rx_size == PACKET_SIZE) {
        Serial.println("Uplink LoRa recebido! Encaminhando para MQTT...");

        String json = "{";
        json += "\"schema_version\":\"3.0\",";
        json += "\"gateway_id\":\"gw-001\",";
        json += "\"rssi_dbm\":" + String(get_last_rssi()) + ",";
        json += "\"snr_db\":" + String(get_last_snr()) + ",";
        json += "\"payload_hex\":\"";
        for (int i = 0; i < PACKET_SIZE; i++) {
            if (buffer[i] < 0x10) json += "0";
            json += String(buffer[i], HEX);
        }
        json += "\",";
        json += "\"size\":" + String(PACKET_SIZE);
        json += "}";

        client.publish(topic_rx, json.c_str());
    }
}