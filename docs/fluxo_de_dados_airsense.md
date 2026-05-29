# AirSense v2.0 — Documento de Arquitetura de Fluxo de Dados

**Versão:** 1.0  
**Data:** 2026-05-26  
**Projeto:** AirSense IoT — Monitoramento Ambiental com LoRa/MQTT

---

## 1. Visão Geral

Este documento descreve como os dados físicos (umidade, poluição, metadados de rádio) viajam desde o sensor no campo até o dashboard em tempo real, e como comandos de configuração retornam do sistema para o sensor.

O fluxo é dividido em duas direções:
- **Uplink:** Sensor → Nuvem (dados de leitura)
- **Downlink:** Nuvem → Sensor (comandos de configuração)

---

## 2. Componentes da Stack

| Camada | Componente | Tecnologia | Função Principal |
|--------|-----------|------------|------------------|
| **Campo** | Sensor LoRa | Arduino/STM32 + DHT22 + ZP07 | Coleta dados ambientais e transmite via rádio |
| **Campo** | Gateway | ESP32 + RFM95W | Recebe LoRa, converte para JSON, publica no MQTT |
| **Rede** | Mosquitto | Eclipse Mosquitto 2.x | Broker MQTT — distribui mensagens entre publishers e subscribers |
| **Borda** | Border Python | Python 3 + paho-mqtt | Decodifica pacotes TpM, salva em JSON, envia comandos DL |
| **Coleta** | Telegraf | InfluxData Telegraf 1.30 | Consome MQTT, converte para métricas InfluxDB, escreve via HTTP |
| **Banco** | InfluxDB v2 | InfluxDB 2.7 | Armazena time-series (métricas com timestamp) |
| **Visualização** | Grafana | Grafana 11.x | Consulta InfluxDB e plota dashboards |
| **Opcional** | Node-RED | Node-RED 3.x | Fluxos visuais para automação e prototipagem |

---

## 3. Fluxo Uplink — Do Sensor ao Dashboard

O Uplink é o caminho dos **dados de leitura** (sensor → nuvem).

### 3.1 Diagrama de Sequência

```
┌─────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌─────────┐
│ Sensor  │   │ Gateway  │   │ Mosquitto│   │ Telegraf │   │ InfluxDB │   │ Grafana  │   │ Border  │
│  LoRa   │   │  ESP32   │   │  Broker  │   │ Coletor  │   │   v2     │   │Dashboard │   │ Python  │
└────┬────┘   └────┬─────┘   └────┬─────┘   └────┬─────┘   └────┬─────┘   └────┬─────┘   └────┬────┘
     │             │              │              │              │              │              │
     │ 1. Pacote   │              │              │              │              │              │
     │    TpM      │              │              │              │              │              │
     │  (20 bytes) │              │              │              │              │              │
     │────────────▶│              │              │              │              │              │
     │   Rádio     │              │              │              │              │              │
     │   LoRa      │              │              │              │              │              │
     │             │ 2. Envelope  │              │              │              │              │
     │             │    JSON      │              │              │              │              │
     │             │─────────────▶│              │              │              │              │
     │             │   WiFi/MQTT  │              │              │              │              │
     │             │              │ 3. Distribui │              │              │              │
     │             │              │─────────────▶│              │              │              │
     │             │              │   MQTT sub   │              │              │              │
     │             │              │              │ 4. Consome   │              │              │
     │             │              │              │    JSON      │              │              │
     │             │              │              │─────────────▶│              │              │
     │             │              │              │   HTTP API   │              │              │
     │             │              │              │  (Line       │              │              │
     │             │              │              │   Protocol)  │              │              │
     │             │              │              │              │ 5. Armazena  │              │
     │             │              │              │              │─────────────▶│              │
     │             │              │              │              │   Bucket     │              │
     │             │              │              │              │              │ 6. Query     │
     │             │              │              │              │              │   Flux       │
     │             │              │              │              │              │◀─────────────│
     │             │              │              │              │              │   HTTP       │
     │             │              │              │              │              │              │
     │             │              │ 3b. Distribui│              │              │              │
     │             │              │─────────────▶│              │              │              │
     │             │              │   MQTT sub   │              │              │ 7. Decodifica│
     │             │              │              │              │              │    TpM       │
     │             │              │              │              │              │◀─────────────│
     │             │              │              │              │              │   Salva      │
     │             │              │              │              │              │   JSON       │
```

### 3.2 Passo a Passo Detalhado

#### Passo 1 — Sensor: Leitura e Empacotamento
**Onde:** Hardware no campo (Arduino/STM32 com sensores DHT22 e ZP07)

1. O microcontrolador lê os sensores:
   - **DHT22:** Umidade relativa do ar (%)
   - **ZP07:** Nível de poluição (índice 0–255)
2. Monta o **pacote TpM (20 bytes)** no formato binário:

   | Byte | Campo | Descrição |
   |------|-------|-----------|
   | 0 | RSSI mapeado | Qualidade do sinal de recepção anterior |
   | 1–2 | SNR mapeado | Relação sinal/ruído (×10) |
   | 3–7 | Reservados | Padding / futuro uso |
   | 8 | DEST_ID | ID do gateway destino |
   | 9 | Reservado | — |
   | 10 | SRC_ID | ID do sensor |
   | 11 | Reservado | — |
   | 12–13 | DL_COUNT | Contador de comandos recebidos |
   | 14–15 | UL_COUNT | Contador de pacotes enviados |
   | 16 | SENSOR_STATUS | Máscara de sensores ativos |
   | 17 | POLLUTION_LEVEL | Leitura do ZP07 |
   | 18–19 | HUMIDITY_RAW | Leitura do DHT22 (×10) |

3. Transmite o pacote via **rádio LoRa** (915 MHz, 125 kHz BW, SF7).

---

#### Passo 2 — Gateway: Recepção e Conversão
**Onde:** ESP32 com módulo RFM95W (`gateway_mqtt_v2.ino`)

1. O Gateway fica em modo de escuta contínuo (`LoRa.parsePacket()`).
2. Ao receber 20 bytes:
   - Lê os bytes brutos para o buffer `PacoteUL[20]`
   - Extrai **metadados do rádio** do próprio módulo LoRa:
     - `RSSI_dBm_UL = LoRa.packetRssi()` → ex: `-52`
     - `SNR_UL = LoRa.packetSnr()` → ex: `8.5`
3. Codifica o payload binário em **Base64** (`base64::encode`).
4. Monta o **envelope JSON**:

```json
{
  "schema_version": "2.0",
  "ts": 1716745200,
  "gateway_id": "gw-001",
  "radio": {
    "rssi_dbm": -52.0,
    "snr_db": 8.5,
    "freq_hz": 915000000
  },
  "tpm": {
    "payload_b64": "eABfAAAAAAABAAAAAQAAAAEDASk=",
    "payload_hex": "78005F0000000001000100000001030129",
    "size": 20
  }
}
```

5. Publica no broker MQTT via WiFi:
   - **Tópico:** `airsense/gw-001/rx`
   - **QoS:** 1 (garantia de entrega pelo menos uma vez)

---

#### Passo 3 — Mosquitto: Distribuição
**Onde:** Container Docker `airsense-mosquitto`

1. Recebe a mensagem no tópico `airsense/gw-001/rx`.
2. Consulta sua tabela de subscribers.
3. Encaminha a mensagem para todos os clientes inscritos no tópico (ou wildcard `airsense/+/rx`).

**Subscribers ativos na stack:**
- **Telegraf** — inscrito em `airsense/+/rx`, `airsense/+/status`
- **Border Python** — inscrito em `airsense/gw-001/rx`, `airsense/gw-001/tx/ack`, `airsense/gw-001/status`

---

#### Passo 4 — Telegraf: Coleta e Conversão
**Onde:** Container Docker `airsense-telegraf`

> **⚠️ IMPORTANTE:** O Telegraf é o único componente que escreve dados no InfluxDB nesta arquitetura. Nem o Gateway, nem o Border, nem o Mosquitto falam diretamente com o InfluxDB.

1. **Consome** a mensagem JSON do tópico MQTT.
2. **Parseia** o JSON usando o parser `json` configurado.
3. **Extrai campos** configurados em `json_string_fields`:
   - `gateway_id`, `schema_version`, `command_id`
4. **Converte** os dados para o formato **InfluxDB Line Protocol**:

```text
mqtt_consumer,gateway_id=gw-001,source=gateway rssi_dbm=-52.0,snr_db=8.5,freq_hz=915000000i 1716745200000000000
```

   - **Measurement:** `mqtt_consumer` (nome padrão do plugin)
   - **Tags:** `gateway_id=gw-001`, `source=gateway`
   - **Fields:** `rssi_dbm=-52.0`, `snr_db=8.5`, `freq_hz=915000000`
   - **Timestamp:** em nanosegundos Unix

5. **Escreve** no InfluxDB via **HTTP POST** na API `/api/v2/write`:
   - **URL:** `http://influxdb:8086`
   - **Token:** `airsense-token-2024`
   - **Organização:** `airsense-org`
   - **Bucket:** `airsense-bucket`

---

#### Passo 5 — InfluxDB: Armazenamento
**Onde:** Container Docker `airsense-influxdb`

1. Recebe o POST HTTP com dados em Line Protocol.
2. Valida o token de autenticação.
3. Escreve no bucket `airsense-bucket`:
   - Indexa por **timestamp** (eixo X de todo time-series)
   - Indexa por **tags** (colunas de baixa cardinalidade, usadas para filtros)
   - Armazena **fields** (valores numéricos que variam no tempo)
4. Dados ficam disponíveis para consulta imediatamente.

---

#### Passo 6 — Grafana: Visualização
**Onde:** Container Docker `airsense-grafana`

1. O usuário abre o dashboard em `http://localhost:3000`.
2. Grafana executa **queries Flux** contra o InfluxDB:

```flux
from(bucket: "airsense-bucket")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "mqtt_consumer")
  |> filter(fn: (r) => r._field == "rssi_dbm")
  |> aggregateWindow(every: v.windowPeriod, fn: mean)
```

3. Recebe os dados em formato CSV/JSON.
4. Renderiza gráficos de linha, gauge, heatmap, etc.

---

#### Passo 7 — Border Python (paralelo)
**Onde:** Processo Python `border_mqtt.py`

> O Border é um **subscriber paralelo**. Ele recebe os mesmos dados do Mosquitto, mas faz processamento diferente do Telegraf.

1. Recebe o envelope JSON do tópico `airsense/gw-001/rx`.
2. Extrai `payload_b64` do campo `tpm`.
3. Decodifica Base64 → bytes brutos (20 bytes).
4. Interpreta o pacote TpM segundo a especificação:
   - Byte 17 = pollution_level
   - Bytes 18–19 = humidity_raw → divide por 10 para %
   - Byte 10 = src_id (ID do sensor)
5. Salva em `sensor_data.json` (compatível com `web_server.py` legado).
6. Printa no terminal para monitoramento humano:

```
[2026-05-26T16:30:00] Sensor 1
  Poluicao: 42 | Umidade: 65.3%
  RSSI: -52.0 dBm | SNR: 8.5 dB
  DL Count: 1 | UL Count: 10
```

---

## 4. Fluxo Downlink — Do Sistema para o Sensor

O Downlink é o caminho dos **comandos de configuração** (nuvem → sensor).

### 4.1 Diagrama de Sequência

```
┌──────────┐   ┌──────────┐   ┌──────────┐   ┌──────────┐   ┌─────────┐
│  Border  │   │ Mosquitto│   │  Gateway │   │  Rádio   │   │ Sensor  │
│  Python  │   │  Broker  │   │  ESP32   │   │   LoRa   │   │  LoRa   │
└────┬─────┘   └────┬─────┘   └────┬─────┘   └────┬─────┘   └────┬────┘
     │              │              │              │              │
     │ 1. Comando   │              │              │              │
     │    JSON      │              │              │              │
     │─────────────▶│              │              │              │
     │  MQTT pub    │              │              │              │
     │  airsense/   │              │              │              │
     │  gw-001/tx   │              │              │              │
     │              │ 2. Entrega   │              │              │
     │              │─────────────▶│              │              │
     │              │   MQTT sub   │              │              │
     │              │              │ 3. Decodifica│              │
     │              │              │    Base64    │              │
     │              │              │    Monta     │              │
     │              │              │    pacote    │              │
     │              │              │    TpM DL    │              │
     │              │              │─────────────▶│              │
     │              │              │   LoRa TX    │              │
     │              │              │              │ 4. Transmite │
     │              │              │              │   (20 bytes) │
     │              │              │              │─────────────▶│
     │              │              │              │              │ 5. Executa
     │              │              │              │              │    comando
```

### 4.2 Passo a Passo Detalhado

#### Passo 1 — Border: Montagem do Comando
**Onde:** `border_mqtt.py` (menu interativo ou polling automático)

1. O operador (ou sistema) solicita uma ação, ex: "alterar intervalo de leitura para 60s".
2. Border monta o **pacote TpM Downlink** (20 bytes):

   | Byte | Valor | Significado |
   |------|-------|-------------|
   | 8 | `0x01` | DEST_ID = sensor 1 |
   | 10 | `0x00` | SRC_ID = gateway |
   | 16 | `0x02` | REQUEST_TYPE = change interval |
   | 17 | `0x3C` | COMMAND_VALUE = 60 (segundos) |

3. Codifica o pacote em **Base64**.
4. Monta envelope JSON:

```json
{
  "schema_version": "2.0",
  "command_id": "cmd-1716745300",
  "priority": 1,
  "tpm": {
    "payload_b64": "AAAAAAAABQAAAAAAAAMAAAAA",
    "size": 20
  }
}
```

5. Publica no broker:
   - **Tópico:** `airsense/gw-001/tx`
   - **QoS:** 1

---

#### Passo 2 — Mosquitto: Distribuição DL
1. Recebe a mensagem no tópico `airsense/gw-001/tx`.
2. Entrega ao Gateway ESP32 (único subscriber deste tópico).

---

#### Passo 3 — Gateway: Preparação LoRa
**Onde:** `gateway_mqtt_v2.ino`

1. A função `callbackMQTT()` é acionada.
2. Chama `Phy_MQTT_receive_DL()`:
   - Parseia o JSON
   - Extrai `tpm.payload_b64`
   - Decodifica Base64 → `PacoteDL[20]`
3. Chama `Phy_radio_send_DL()`:
   - `LoRa.beginPacket()`
   - `LoRa.write(PacoteDL, 20)`
   - `LoRa.endPacket()`
4. Publica ACK no tópico `airsense/gw-001/tx/ack`.

---

#### Passo 4 — Rádio LoRa: Transmissão
1. O módulo RFM95W transmite os 20 bytes no ar (915 MHz).
2. O Sensor, em modo de escuta, recebe o pacote.

---

#### Passo 5 — Sensor: Execução
1. O sensor valida DEST_ID (se é para ele).
2. Interpreta REQUEST_TYPE e COMMAND_VALUE.
3. Aplica a configuração (ex: muda `sleep_interval` para 60s).
4. Incrementa `DL_COUNT` no próximo pacote Uplink (confirmação implícita).

---

## 5. Quem Fala com Quem? (Matriz de Comunicação)

| Origem → Destino | Protocolo | Tópico / Endpoint | Formato |
|------------------|-----------|-------------------|---------|
| Sensor → Gateway | LoRa (RF) | — | Binário TpM (20 bytes) |
| Gateway → Mosquitto | MQTT/TCP | `airsense/gw-001/rx` | JSON envelope |
| Mosquitto → Telegraf | MQTT/TCP | `airsense/+/rx` | JSON envelope |
| Mosquitto → Border | MQTT/TCP | `airsense/gw-001/rx` | JSON envelope |
| **Telegraf → InfluxDB** | **HTTP POST** | `/api/v2/write` | **Line Protocol** |
| Grafana → InfluxDB | HTTP GET | `/api/v2/query` | Flux Query |
| Border → Mosquitto | MQTT/TCP | `airsense/gw-001/tx` | JSON comando |
| Mosquitto → Gateway | MQTT/TCP | `airsense/gw-001/tx` | JSON comando |
| Gateway → Sensor | LoRa (RF) | — | Binário TpM DL (20 bytes) |
| Gateway → Mosquitto | MQTT/TCP | `airsense/gw-001/tx/ack` | JSON ACK |
| Gateway → Mosquitto | MQTT/TCP | `airsense/gw-001/status` | JSON heartbeat |

> **Destaque:** Apenas o **Telegraf** escreve no InfluxDB. O Border e o Gateway nunca falam HTTP com o InfluxDB.

---

## 6. Formatos de Dados em Cada Fronteira

### 6.1 Sensor ↔ Gateway (LoRa)
```
[78 00 5F 00 00 00 01 00 00 00 01 00 00 01 00 0A 03 2A 01 9D]
```
Pacote binário cru. Sem estrutura de texto.

### 6.2 Gateway ↔ Mosquitto (MQTT)
```json
{
  "schema_version": "2.0",
  "ts": 1716745200,
  "gateway_id": "gw-001",
  "radio": {"rssi_dbm": -52.0, "snr_db": 8.5, "freq_hz": 915000000},
  "tpm": {
    "payload_b64": "eABfAAAAAAABAAAAAQAAAAEDASk=",
    "payload_hex": "78005F0000000001000100000001030129",
    "size": 20
  }
}
```

### 6.3 Telegraf ↔ InfluxDB (HTTP)
```text
mqtt_consumer,gateway_id=gw-001,source=gateway rssi_dbm=-52.0,snr_db=8.5,freq_hz=915000000i 1716745200000000000
```
Formato: `measurement,tag_set field_set timestamp`

### 6.4 Grafana ↔ InfluxDB (HTTP)
```flux
from(bucket: "airsense-bucket")
  |> range(start: -6h)
  |> filter(fn: (r) => r._measurement == "mqtt_consumer")
  |> filter(fn: (r) => r._field == "rssi_dbm")
```

---

## 7. Troubleshooting por Etapa do Fluxo

| Sintoma | Onde está o problema | Como diagnosticar |
|---------|---------------------|-----------------|
| "Nenhum dado no Grafana" | Qualquer etapa do Uplink | Verifique `docker-compose ps` e logs |
| Gateway não conecta no MQTT | Gateway → Mosquitto | `docker logs airsense-mosquitto` + verificar IP em `secrets.h` |
| Mosquitto não recebe | Sensor → Gateway | Verificar LoRa (frequência, SF, sync word) |
| Telegraf não escreve | Mosquitto → Telegraf → InfluxDB | `docker logs -f airsense-telegraf` |
| "401 Unauthorized" no InfluxDB | Token errado | Verificar `.env` e `telegraf.conf` |
| Border não decodifica | Border Python | Verificar se `paho-mqtt` está instalado |
| Comando não chega no sensor | Downlink completo | Verificar `airsense/gw-001/tx` e ACK |

---

## 8. Glossário

| Termo | Significado |
|-------|-------------|
| **TpM** | Transport Protocol for Microcontrollers — protocolo binário de 20 bytes |
| **Uplink (UL)** | Dados que sobem do sensor para a nuvem |
| **Downlink (DL)** | Comandos que descem da nuvem para o sensor |
| **Envelope JSON** | Estrutura JSON que embrulha o payload Base64 + metadados |
| **Line Protocol** | Formato de texto do InfluxDB: `measurement,tag=value field=value timestamp` |
| **Bucket** | Equivalente a "banco de dados" no InfluxDB v2 |
| **Measurement** | Equivalente a "tabela" no InfluxDB |
| **Tag** | Campo de string usado para indexação/filtro (baixa cardinalidade) |
| **Field** | Campo numérico que varia no tempo (alta cardinalidade) |
| **Flux** | Linguagem de query do InfluxDB v2 |

---

## 9. Anexo: Diagrama Simplificado (ASCII)

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              CAMPO (Hardware)                               │
│  ┌─────────────┐         ┌─────────────┐                                    │
│  │ Sensor LoRa │──LoRa──▶│Gateway ESP32│──WiFi──┐                         │
│  │  (20 bytes) │         │  (Base64)   │        │                         │
│  └─────────────┘         └─────────────┘        │                         │
└─────────────────────────────────────────────────┼───────────────────────────┘
                                                  │
┌─────────────────────────────────────────────────┼───────────────────────────┐
│                         REDE (Docker Stack)     │                           │
│  ┌─────────────┐         ┌─────────────┐      │                         │
│  │  Mosquitto  │◀─MQTT───│  Gateway    │──────┘                         │
│  │   Broker    │         │  (JSON)     │                                  │
│  └──────┬──────┘         └─────────────┘                                  │
│         │                                                                   │
│         ├──MQTT──▶ ┌─────────────┐                                        │
│         │          │  Telegraf   │                                        │
│         │          │  (Coletor)  │                                        │
│         │          └──────┬──────┘                                        │
│         │                 │ HTTP                                          │
│         │                 ▼                                               │
│         │          ┌─────────────┐                                        │
│         │          │  InfluxDB   │                                        │
│         │          │   Bucket    │                                        │
│         │          └──────┬──────┘                                        │
│         │                 │ Flux Query                                    │
│         │                 ▼                                               │
│         │          ┌─────────────┐                                        │
│         │          │   Grafana   │                                        │
│         │          │ Dashboard │                                        │
│         │          └─────────────┘                                        │
│         │                                                                   │
│         └──MQTT──▶ ┌─────────────┐                                        │
│                    │Border Python│                                        │
│                    │(Decodifica) │                                        │
│                    └─────────────┘                                        │
└───────────────────────────────────────────────────────────────────────────┘
```

---

*Fim do documento.*

---

