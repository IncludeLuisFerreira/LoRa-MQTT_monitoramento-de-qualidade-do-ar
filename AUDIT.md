# 📑 Relatório de Auditoria Senior IoT - Projeto AirSense v3.0

Este documento contém a auditoria técnica exaustiva do ecossistema AirSense, abrangendo firmware, gateway, backend e infraestrutura.

---

### 🔍 TABELA DE ACHADOS

| # | Categoria | Problema | Localização (arquivo:linha) | Gravidade | Impacto | Prioridade | Sugestão de Correção |
|---|-----------|----------|----------------------------|-----------|---------|------------|----------------------|
| 1 | Recursos | Memory Leak massivo em `PacoteUL` | `firmware/sensor_node/_APP_.ino:81` | Crítica | Crash por esgotamento de RAM em poucos minutos. | P0 | Definir `PacoteUL` como buffer estático global (`uint8_t[20]`) e remover `calloc/free`. |
| 2 | Segurança | Credenciais Sensíveis Hardcoded | `firmware/gateway/config.h:6-7` | Crítica | Exposição de senhas de WiFi e Broker em controle de versão. | P0 | Utilizar arquivo `secrets.h` (fora do Git) ou variáveis de ambiente de build. |
| 3 | Conectividade | Vazamento de Threads (Thread Leak) | `border/border_mqtt.py:27` | Alta | Consumo de CPU escala a cada queda de conexão MQTT. | P1 | Mover o início da thread periódica para o método `run()` ou usar uma flag de controle. |
| 4 | Segurança | Token do InfluxDB Exposto | `border/database.py:8` | Crítica | Acesso total à base de dados por qualquer pessoa com o código. | P0 | Usar `os.environ.get('INFLUXDB_TOKEN')` e injetar via Docker ou shell. |
| 5 | Recursos | Uso de Alocação Dinâmica (Heap Fragmentation) | `firmware/sensor_node/sensor_node.ino:47` | Alta | Instabilidade imprevisível em execuções longas (meses). | P1 | Substituir `malloc/free` por buffers estáticos para processamento de pacotes. |
| 6 | Protocolos | Falta de Integridade de Aplicação (CRC) | `firmware/sensor_node/lora_packet.h` | Média | Risco de processar dados corrompidos se o CRC do rádio falhar. | P1 | Implementar CRC8 no último byte do payload de 20 bytes. |
| 7 | Inconsistências | Configuração de Rede Incompatível | `border/border_mqtt.py:10` | Média | O sistema não funciona via Docker Compose (localhost vs host). | P2 | Alterar `BROKER` para 'mosquitto' (nome do serviço no docker-compose). |
| 8 | Arquitetura | Acoplamento de ID de Sensor no Gateway | `firmware/gateway/gateway.ino:29` | Média | Gateway bloqueia qualquer nó que não seja o ID 1. | P2 | Remover filtro hardcoded e encaminhar pacotes baseado no `NET_DEST_ID`. |
| 9 | Recursos | Ausência de Deep Sleep (Consumo de Energia) | `firmware/sensor_node/sensor_node.ino` | Baixa | Inviabilidade de operação por baterias/painel solar. | P3 | Utilizar `esp_deep_sleep_start()` entre as janelas de leitura. |
| 10 | Conectividade | Falta de Autenticação e TLS no MQTT | `firmware/gateway/config.h` | Alta | Comunicação em texto claro, vulnerável a sniffing. | P1 | Configurar `WiFiClientSecure` e fornecer Root CA no Gateway. |

---

### 🚀 RESUMO EXECUTIVO: TOP 5 PROBLEMAS PARA CORRIGIR AGORA

1. **Memory Leak no Sensor Node (P0):** A função `Montar_pacote_leitura_sensor` aloca memória com `calloc` toda vez que é chamada, mas o ponteiro é sobrescrito ou nunca liberado na camada de transporte. O ESP32 irá travar em poucos ciclos de envio.
2. **Vulnerabilidade de Credenciais (P0):** Tokens do InfluxDB e senhas de WiFi estão expostos no código. Isso compromete a segurança de toda a infraestrutura.
3. **Thread Leak no Backend (P1):** A cada reconexão MQTT, uma nova thread de envio periódico é criada sem encerrar a anterior, causando exaustão de recursos do servidor.
4. **Alocação Dinâmica no Loop (P1):** O uso de `malloc/free` para pacotes de tamanho fixo em sistemas embarcados causa fragmentação do heap, levando a crashes aleatórios em longo prazo.
5. **Inconsistência de Docker (P2):** O host do broker está como `localhost` no código Python, o que impede a comunicação correta entre os containers do Docker Compose.

---

### 💡 SUGESTÕES E PRÓXIMOS PASSOS

- **Segurança:** Implementar MQTTS (MQTT over TLS) no ESP32. Para fins acadêmicos, você pode começar desativando a verificação estrita de certificado (`setInsecure()`) e evoluir para o uso de certificados Root CA gravados no SPIFFS/LittleFS.
- **Robustez:** Adicionar um Watchdog Timer (WDT) no firmware para reiniciar o dispositivo caso o loop LoRa trave.
- **Escalabilidade:** Alterar o schema do InfluxDB para usar Tags para `sensor_id` e `gateway_id` (o que já é feito parcialmente, mas deve ser padronizado).
- **Energia:** Implementar o Deep Sleep do ESP32 utilizando `esp_sleep_enable_timer_wakeup()`. Lembre-se de salvar contadores críticos na memória RTC (`RTC_DATA_ATTR`).

**Auditoria realizada por Jules (Senior IoT Engineer).**
