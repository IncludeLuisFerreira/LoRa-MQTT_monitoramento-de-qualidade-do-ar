#ifndef TPM_PROTOCOL_H
#define TPM_PROTOCOL_H

#include <Arduino.h>

#define TPM_PACKET_SIZE 20

struct TpMUplink {
    uint8_t rssi_mapped;
    uint16_t snr_mapped;
    uint8_t dest_id;
    uint8_t src_id;
    uint16_t dl_count;
    uint16_t ul_count;
    uint8_t sensor_status;
    uint8_t pollution_level;
    uint16_t humidity_raw;

    void serialize(uint8_t* buffer);
};

struct TpMDownlink {
    uint8_t dest_id;
    uint8_t src_id;
    uint16_t command_id;
    uint8_t request_type;
    uint8_t command_value;

    static TpMDownlink deserialize(const uint8_t* buffer);
};

#endif
