#ifndef LORA_PACKET_H
#define LORA_PACKET_H

// ============================================
// TAMANHO TOTAL DO PACOTE
// ============================================
#define PACKET_SIZE 20

// ============================================
// CAMADA FÍSICA - 4 bytes (offsets 0-3)
// ============================================
#define PHYSICAL_RSSI_DL          0   // Byte 0: RSSI último downlink recebido
#define PHYSICAL_SNR_DL           1   // Byte 1: SNR último downlink recebido
#define PHYSICAL_RADIO_STATUS     2   // Byte 2: Status do radio (reservado)
#define PHYSICAL_BATTERY_LEVEL    3   // Byte 3: Nivel de bateria (reservado)
#define PHYSICAL_HARDWARE_TYPE    4   // Byte 4: Tipo de Hardware

// ============================================
// CAMADA MAC - 4 bytes (offsets 5-7)
// ============================================
#define MAC_SLEEP_TIME            5   // Byte 5: SleepTime (minutos)
#define MAC_PROTOCOL_VERSION      6   // Byte 6: Protocol Version (0x01)
#define MAC_RESERVED              7   // Byte 7: Reservado

// ============================================
// CAMADA NET - 4 bytes (offsets 8-11)
// ============================================
#define NET_DEST_ID               8   // Byte 8: ID destino
#define NET_RESERVED1             9   // Byte 9: Reservado
#define NET_SOURCE_ID             10  // Byte 10: ID origem
#define NET_RESERVED2             11  // Byte 11: Reservado

// ============================================
// CAMADA TRANSPORTE - 4 bytes (offsets 12-15)
// ============================================
#define TRANSPORT_DL_COUNTER      12  // Byte 12: Contador de pacotes DL para o nó sensor
#define TRANSPORT_FLAGS_ACK       13  // Byte 13: Flags ACK e controle
#define TRANSPORT_UL_COUNTER      14  // Byte 14: Contador de pacotes UL para o gateway
#define TRANSPORT_RESERVED        15  // Byte 15: Reservado

// ============================================
// CAMADA APLICAÇÃO UP-LINK - 4 bytes (offsets 16-19)
// ============================================
#define APP_UP_SENSOR_STATUS      16  // Byte 16: BitMask de status dos sensores
#define APP_UP_POLLUTION          17  // Byte 17: Nivel de poluicao (0-255)
#define APP_UP_HUMIDITY_HIGH      18  // Byte 18: Parte alta umidade
#define APP_UP_HUMIDITY_LOW       19  // Byte 19: Parte baixa umidade

// ============================================
// CAMADA APLICAÇÃO DOWN-LINK - 4 bytes (offsets 16-19)
// ============================================
#define APP_DOWN_REQUEST_TYPE     16  // Byte 16: Request Type (0x00=ler sensores, 0x01=alterar SleepTime)
#define APP_DOWN_COMMAND_VALUE    17  // Byte 17: Valor do comando (descartado se byte16=0x00)
#define APP_DOWN_RESERVED1        18  // Byte 18: Reservado
#define APP_DOWN_RESERVED2        19  // Byte 19: Reservado

// ============================================
// VALORES ESPECÍFICOS (enums/constantes)
// ============================================
// Protocol Version
#define PROTOCOL_VERSION_VALUE    0x01

// Request Types (Downlink)
#define REQ_TYPE_READ_SENSORS     0x00
#define REQ_TYPE_CHANGE_SLEEPTIME 0x01

// Valores padrão
#define DEFAULT_SLEEP_TIME        10
#define DEFAULT_HARDWARE_TYPE     0x01

// ============================================
// MÁSCARAS DE BITS (BitMask de status dos sensores)
// ============================================
#define SENSOR_STATUS_SENSOR1     0x01  // Bit 0
#define SENSOR_STATUS_SENSOR2     0x02  // Bit 1
#define SENSOR_STATUS_SENSOR3     0x04  // Bit 2
#define SENSOR_STATUS_SENSOR4     0x08  // Bit 3
#define SENSOR_STATUS_SENSOR5     0x10  // Bit 4
#define SENSOR_STATUS_SENSOR6     0x20  // Bit 5
#define SENSOR_STATUS_SENSOR7     0x40  // Bit 6
#define SENSOR_STATUS_SENSOR8     0x80  // Bit 7

// ============================================
// MÁSCARAS PARA FLAGS ACK (Byte 13)
// ============================================
#define FLAGS_ACK                 0x01  // Bit 0: ACK recebido
#define FLAGS_CONTROL             0x02  // Bit 1: Controle ativo
#define FLAGS_RESERVED            0xFC  // Bits 2-7: Reservado

// Extensão dos request types
#define REQUEST_CHANGE_INTERVAL     0x02
#define REQUEST_CHANGE_TX_POWER     0x03
#define REQUEST_CHANGE_SENSORS      0x04

#endif // LORA_PACKET_H