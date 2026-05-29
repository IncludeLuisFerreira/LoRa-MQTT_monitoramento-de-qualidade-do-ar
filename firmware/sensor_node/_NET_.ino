#include "protocol.h"

void Net_radio_receive_DL(uint8_t* buffer) {
    if (buffer[NET_DEST_ID] != SENSOR_ID || buffer[NET_SOURCE_ID] != GATEWAY_ID) {
        // descartar pacote
        memset(buffer, 0, PACKET_SIZE);
        free(PacoteDL);
        PacoteDL = NULL;
        return;
    }
    Transp_radio_receive_DL();
}

void Net_radio_send_UL() {
    PacoteUL[NET_DEST_ID] = GATEWAY_ID;
    PacoteUL[NET_SOURCE_ID] = SENSOR_ID;
    Mac_radio_send_UL();
}