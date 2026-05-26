//================ CAMADA DE TRANSPORT DL ========
void Transp_radio_receive_DL() { 
  // Faz o controle fluzo entre o nó sensor e o gateway
  // Por exemplo contagem de pacotes recedidos de DL
  contador_pkt_DL = contador_pkt_DL + 1;

  //============== CHAMA A CAMADA DE APLICAÇÃO DE DL
  App_radio_receive_DL();
}

//================ CAMADA DE TRANSPORTE DE UL ========
void Transp_radio_send_UL() { 
    
    // Contador de pacotes DL (bytes 12-13)
    PacoteUL[12] = (contador_pkt_DL >> 8) & 0xFF;
    PacoteUL[13] = contador_pkt_DL & 0xFF;
    
    // Incrementa e armazena contador de pacotes UL (bytes 14-15)
    contador_pkt_UL = contador_pkt_UL + 1;
    PacoteUL[14] = (contador_pkt_UL >> 8) & 0xFF;
    PacoteUL[15] = contador_pkt_UL & 0xFF;

    Net_radio_send_UL();
}