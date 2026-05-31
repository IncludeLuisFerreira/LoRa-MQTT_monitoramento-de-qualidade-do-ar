#include "protocol.h"

void Mac_radio_receive_DL(uint8_t* buffer) { 
    if (buffer[MAC_PROTOCOL_VERSION] != PROTOCOL_VERSION_VALUE) {
        // Versão diferente: descarta pacote
        return;
    }
    Net_radio_receive_DL(buffer);
}

void Mac_radio_send_UL() {
    PacoteUL[MAC_PROTOCOL_VERSION] = PROTOCOL_VERSION_VALUE;
    Phy_radio_send_UL(PacoteUL);
}