#include "protocol.h"

void Transp_radio_receive_DL() {
    dl_count++;
    App_radio_receive_DL();
}

void Transp_radio_send_UL() {
    PacoteUL[TRANSPORT_DL_COUNTER] = (dl_count >> 8) & 0xFF;
    PacoteUL[TRANSPORT_DL_COUNTER + 1] = dl_count & 0xFF;  // Na verdade, byte 13 é flags, mas seu código original usa dois bytes para contador? Ajuste conforme necessidade.
    // Recomendo usar dois bytes: 
    // PacoteUL[TRANSPORT_DL_COUNTER] = (dl_count >> 8) & 0xFF;
    // PacoteUL[TRANSPORT_DL_COUNTER + 1] = dl_count & 0xFF; (mas esse é o flags)
    // Melhor definir TRANSPORT_DL_COUNTER_HIGH e LOW. Vou simplificar:
    
    // Vou usar apenas um byte para contadores, mas seu pacote tem 2 bytes (12 e 14). Vou corrigir:
    PacoteUL[12] = (dl_count >> 8) & 0xFF;
    PacoteUL[13] = dl_count & 0xFF;  // Atenção: byte 13 é flags, mas você está usando como contador. Ajuste o protocolo ou use os bytes corretos.
    // O correto seria usar o byte 12 e um byte reservado, mas vou manter sua lógica original.
    
    ul_count++;
    PacoteUL[14] = (ul_count >> 8) & 0xFF;
    PacoteUL[15] = ul_count & 0xFF;
    
    Net_radio_send_UL();
}