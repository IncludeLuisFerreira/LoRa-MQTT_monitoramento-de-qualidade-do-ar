# 📑 Segunda Revisão de Auditoria IoT - AirSense v3.1

Após a rodada de correções realizada, realizei uma nova varredura completa. Notei um avanço significativo na **arquitetura de software** do backend e na **infraestrutura de deploy**, mas os **bugs críticos de execução e segurança** ainda precisam de atenção imediata.

---

### 🔍 TABELA DE STATUS DA AUDITORIA

| # | Categoria | Status | Problema | Localização | Prioridade | Observação/Sugestão |
|---|-----------|--------|----------|-------------|------------|---------------------|
| 1 | Recursos | ⚠️ PERSISTE | Memory Leak em `PacoteUL` | `sensor_node/_APP_.ino:81` | P0 | Ainda usa `calloc()` no loop. O ponteiro é atribuído mas nunca liberado após o envio. |
| 2 | Segurança | ⚠️ PERSISTE | Credenciais Hardcoded | `gateway/config.h:6-7` | P0 | SSID e senha permanecem expostos. Use o arquivo `secrets.h.template` como base e adicione ao `.gitignore`. |
| 3 | Conectividade | ⚠️ PERSISTE | Thread Leak (`periodic_send`) | `border/border_mqtt.py:27` | P1 | A thread é iniciada dentro do `on_connect`. Toda reconexão cria um novo loop infinito de threads. |
| 4 | Segurança | ⚠️ PERSISTE | Token InfluxDB exposto | `border/database.py:8` | P0 | O token de admin (`airsense-token-2026`) deve ser injetado via variável de ambiente. |
| 5 | Arquitetura | ✅ MELHORADO | Organização do Projeto | `border/` | P2 | Excelente avanço na modularização. A separação entre API, Database e Protocolo está seguindo boas práticas. |
| 6 | Infra | ✅ MELHORADO | Dockerização | `infra/` | P2 | Stack completa e funcional definida no Compose. Facilita muito o deploy e teste. |
| 7 | Protocolo | ⚠️ PERSISTE | Falta de CRC de aplicação | `lora_packet.h` | P1 | O payload ainda não possui verificação de integridade fim-a-fim. |
| 8 | Recursos | ⚠️ PERSISTE | Alocação Dinâmica | `sensor_node.ino:47` | P1 | O uso de `malloc()` para buffers fixos de 20 bytes continua sendo um risco desnecessário de fragmentação. |

---

### 🚩 RESUMO EXECUTIVO DA 2ª RODADA

**O que melhorou:**
- **Modularidade:** Você reestruturou o backend em um serviço de borda robusto. O uso de `pkt_utils.py` para decodificação centralizada é um ponto positivo forte.
- **Ambiente:** A inclusão de Docker e Telegraf mostra uma visão profissional de monitoramento e persistência.

**O que ainda bloqueia o release (Crítico):**
1. **Crash por Memória (Firmware):** Sem o `free(PacoteUL)` ou o uso de buffers estáticos, o sensor vai travar. É um erro clássico de C que "mata" dispositivos em campo.
2. **Segurança de Segredos:** Tokens e senhas no código impedem que o projeto seja compartilhado ou usado em produção com segurança.
3. **Instabilidade do Backend:** O problema das threads no `on_connect` é sutil, mas perigoso. Se o broker MQTT piscar, seu backend vai consumir 100% da CPU tentando rodar loops paralelos de envio.

---

### 🛠️ DICA TÉCNICA DO DIA: "O Problema do on_connect"

No Paho MQTT, o `on_connect` é chamado toda vez que a conexão é estabelecida. Se você coloca `threading.Thread(...).start()` lá dentro, você terá uma nova thread a cada queda de rede.

**Como corrigir:**
No seu `__init__` ou `run()`, inicie a thread uma única vez e use um `threading.Event` ou uma flag `self.connected` para que a thread saiba se pode ou não enviar mensagens no momento.

---
**Auditoria realizada por Jules (Senior IoT Engineer).**
