#include "protocol.h"

void Transp_radio_receive_DL() {
    dl_count++;
    App_radio_receive_DL();
}

void Transp_radio_send_UL() {
    // dl_count: byte 12 (um byte, sem transbordamento)
    PacoteUL[TRANSPORT_DL_COUNTER] = (uint8_t)(dl_count & 0xFF);
    // flags: byte 13 (mantém como 0 por enquanto)
    PacoteUL[TRANSPORT_FLAGS_ACK] = 0;
    // ul_count: byte 14
    PacoteUL[TRANSPORT_UL_COUNTER] = (uint8_t)(ul_count & 0xFF);
    // byte 15 reservado
    PacoteUL[TRANSPORT_RESERVED] = 0;
    
    ul_count++;
    
    Net_radio_send_UL();
}