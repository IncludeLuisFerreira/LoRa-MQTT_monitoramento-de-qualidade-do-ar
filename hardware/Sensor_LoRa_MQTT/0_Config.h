#ifndef CONFIG_H
#define CONFIG_H

#include "Package_handle.h"

// ============= CAMADA FÍSICA
extern int RSSI_dBm_DL;
extern int RSSI_DL;
extern float SNR_DL;
extern int SNR_DL_inteiro;

// ============= CAMADA MAC
extern byte *PacoteDL;
extern byte *PacoteUL;

// ============= CAMADA DE REDE
extern int ID_sensor;
extern int ID_gateway;

// ============= CAMADA DE TRANSPORTE
extern int contador_pkt_DL;
extern int contador_pkt_UL;

// ============= CAMADA DE APLICAÇÃO
extern float temperatura;
extern float umidade;
extern int txPower;
extern unsigned long sleep_time_ms;
extern unsigned long read_interval_ms;
extern uint8_t sensors_enabled;

// ============= PROTÓTIPOS DAS FUNÇÕES (NA ORDEM CORRETA)
void Phy_radio_receive_DL();
void Phy_radio_send_UL();

void Mac_radio_receive_DL();
void Mac_radio_send_UL();

void Net_radio_receive_DL();
void Net_radio_send_UL();

void Transp_radio_receive_DL();
void Transp_radio_send_UL();

void App_radio_receive_DL();
void App_radio_send_UL(TYPEOF_REQUEST request);
void Montar_pacote_leitura_sensor();
void Montar_pacote_confirmacao();

#endif