#ifndef PACKAGE_HANDLE_H
#define PACKAGE_HANDLE_H

#include <stdint.h>
#include <stdbool.h>

// ============================================
// CAMADA FÍSICA (Bytes 0-4)
// ============================================
#define RSSI_BYTE           0x00
#define SNR_BYTE            0x01
#define RADIO_STATS_BYTE    0x02
#define BATTERY_LEVEL_BYTE  0x03
#define HARDWARE_TYPE_BYTE  0x04

// ============================================
// CAMADA MAC (Bytes 5-7)
// ============================================
#define SLEEP_TIME_BYTE     0x05
#define PROTOCOL_VERSION    0x06
#define MAC_RESERVED_BYTE   0x07

// ============================================
// CAMADA DE REDE (Bytes 8-11)
// ============================================
#define DEST_ID_BYTE        0x08
#define NET_RESERVED1_BYTE  0x09
#define SRC_ID_BYTE         0x0A
#define NET_RESERVED2_BYTE  0x0B

// ============================================
// CAMADA DE TRANSPORTE (Bytes 12-15)
// ============================================
#define DL_PACKET_COUNT     0x0C
#define FLAGS_ACK_BYTE      0x0D
#define UL_PACKET_COUNT     0x0E
#define TRANS_RESERVED_BYTE 0x0F

// ============================================
// CAMADA DE APLICAÇÃO - UPLINK (Bytes 16-19)
// ============================================
#define SENSOR_STATS_UL     0x10  // BitMask de status dos sensores
#define VALUE_POLLUTION_UL  0x11  // Nível de poluição (0-255)
#define HUMIDITY_HIGH_BYTE  0x12  // Parte alta umidade
#define HUMIDITY_LOW_BYTE   0x13  // Parte baixa umidade

// ============================================
// CAMADA DE APLICAÇÃO - DOWNLINK (Bytes 16-19)
// ============================================
#define REQUEST_TYPE_DL     0x10  // 0x00=ler sensores, 0x01=alterar SleepTime
#define COMAND_VALUE_DL     0x11  // Valor do comando
#define APP_DL_RESERVED1    0x12
#define APP_DL_RESERVED2    0x13

// ============================================
// TAMANHOS DE PACOTE
// ============================================
#define DATA_SENSOR_PK      20    // Pacote completo de dados dos sensores
#define CHANGE_PARAM_PK     20    // Downlink também tem 20 bytes (mesma estrutura)
#define MIN_PACKET_SIZE     20    // Tamanho mínimo válido
#define MAX_PACKET_SIZE     20    // Tamanho máximo (por enquanto fixo em 20)

// ============================================
// VALORES POSSÍVEIS PARA REQUEST_TYPE_DL
// ============================================
#define REQUEST_READ_SENSORS    0x00
#define REQUEST_CHANGE_SLEEP    0x01
#define REQUEST_CHANGE_INTERVAL 0x02
#define REQUEST_CHANGE_TX_POWER 0x03
#define REQUEST_CHANGE_SENSORS  0x04  // Habilitar/desabilitar sensores

// ============================================
// MÁSCARAS PARA CONTROLE DE SENSORES
// ============================================
#define SENSOR_DHT22_ENABLED    0x01  // Bit 0: DHT22
#define SENSOR_ZP07_ENABLED     0x02  // Bit 1: ZP07
#define SENSOR_ALL_ENABLED      0x03  // Ambos

// ============================================
// TIPOS ENUMERADOS
// ============================================
typedef enum {
    REQ_DATA_SENSORS,
    REQ_CHANGE_PARAM
} TYPEOF_REQUEST;


/**
 * @brief Função que verifica qual o tamanho do pacote e se ele está
 * entre os tamanhos válidos
 */
bool is_valid_package_size(uint8_t packageSize);



#endif