#include "tpm_protocol.h"

void TpMUplink::serialize(uint8_t* buffer) {
    memset(buffer, 0, TPM_PACKET_SIZE);
    buffer[0] = rssi_mapped;
    buffer[1] = (snr_mapped >> 8) & 0xFF;
    buffer[2] = snr_mapped & 0xFF;
    buffer[8] = dest_id;
    buffer[10] = src_id;
    buffer[12] = (dl_count >> 8) & 0xFF;
    buffer[13] = dl_count & 0xFF;
    buffer[14] = (ul_count >> 8) & 0xFF;
    buffer[15] = ul_count & 0xFF;
    buffer[16] = sensor_status;
    buffer[17] = pollution_level;
    buffer[18] = (humidity_raw >> 8) & 0xFF;
    buffer[19] = humidity_raw & 0xFF;
}

TpMDownlink TpMDownlink::deserialize(const uint8_t* buffer) {
    TpMDownlink dl;
    dl.dest_id = buffer[8];
    dl.src_id = buffer[10];
    dl.command_id = (buffer[12] << 8) | buffer[13];
    dl.request_type = buffer[16];
    dl.command_value = buffer[17];
    return dl;
}
