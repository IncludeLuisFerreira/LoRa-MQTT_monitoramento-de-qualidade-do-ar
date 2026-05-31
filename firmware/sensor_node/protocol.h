#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <Arduino.h>
#include "lora_packet.h"

// IDs
extern uint8_t SENSOR_ID;
extern uint8_t GATEWAY_ID;
extern uint32_t read_interval_ms;
extern uint32_t sleep_time_ms;
extern uint16_t ul_count;
extern uint16_t dl_count;
extern int8_t last_rssi;
extern float last_snr;
extern int8_t txPower;
extern uint8_t sensors_enabled;
extern float temperatura;
extern float umidade;

// Buffers estáticos para os pacotes (substitui os ponteiros dinâmicos)
extern uint8_t PacoteUL[PACKET_SIZE];
extern uint8_t PacoteDL[PACKET_SIZE];

// Definição do tipo de requisição
typedef enum {
    REQ_DATA_SENSORS,
    REQ_CHANGE_PARAM
} TYPEOF_REQUEST;

// Protótipos das funções das camadas
void App_radio_receive_DL();
void App_radio_send_UL(TYPEOF_REQUEST request);
void Montar_pacote_confirmacao();
void Montar_pacote_leitura_sensor();
void Transp_radio_send_UL();
void Transp_radio_receive_DL();
void Net_radio_receive_DL(uint8_t* buffer);
void Net_radio_send_UL();
void Mac_radio_receive_DL(uint8_t* buffer);
void Mac_radio_send_UL();
void Phy_radio_send_UL(uint8_t* buffer);
void Phy_radio_receive_DL();

// Funções auxiliares (de sensors.h)
void init_sensors();
float read_humidity();
uint8_t read_pollution();
uint8_t get_sensor_status();

#endif