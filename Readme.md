# 📡 LoRa → MQTT → Python (Request-Response IoT)

![Licença: MIT](https://img.shields.io/badge/Licen%C3%A7a-MIT-yellow.svg)
![Python](https://img.shields.io/badge/Python-3.13-blue.svg)
![MQTT](https://img.shields.io/badge/Protocol-MQTT-green.svg)
![LoRa](https://img.shields.io/badge/Communication-LoRa-orange.svg)
![Status](https://img.shields.io/badge/status-acad%C3%AAmico-blueviolet.svg)
![IoT](https://img.shields.io/badge/area-IoT-informational.svg)
![Arquitetura](https://img.shields.io/badge/MAC-CENTRALIZADA-green)

Sistema de comunicação IoT baseado em **LoRa + MQTT**, com lógica de controle implementada em Python.

---

## 🚀 Visão Geral

Este projeto implementa um fluxo completo de comunicação entre:

- 📟 **Sensor LoRa (ESP32 + LDR)** → envia dados de luminosidade  
- 🌐 **Broker MQTT (HiveMQ)** → intermedia mensagens  
- 🧠 **Cliente Python** → processa dados e envia comandos de volta  

A arquitetura segue um modelo **request-response síncrono**, garantindo controle determinístico sobre o comportamento do dispositivo remoto.

---

## 🧠 Lógica do Sistema

O cliente Python:

1. Envia uma requisição ao sensor  
2. Aguarda resposta (com timeout)  
3. Processa a luminosidade recebida  
4. Decide o estado do LED  
5. Envia o próximo comando  

### 📊 Regra de decisão

| Luminosidade | Ação |
|--------------|------|
| `< LIMIAR`   | LED Vermelho 🔴 |
| `>= LIMIAR`  | LED Amarelo 🟡 |

---

## 🏗️ Arquitetura


![Diagrama mostrando arquitetura de comunicação entre sensores LoRa, gateway LoRaWAN monocanal, broker MQTT e cliente Python. O esquema inclui pilhas de protocolo rotuladas Nível 1 APP, TRANSP, NET, MAC, PHY de um lado, um gateway LoRaWAN monocanal no meio, o broker MQTT no centro e, do outro lado, outra pilha de protocolo rotulada Nível 3 APP, TRANSP, NET, MAC, PHY. Também exibe conexões de dispositivos físicos com antenas e um bloco de processamento Python representado à direita.](img/diagrama.png)


---

## 📦 Tecnologias Utilizadas

- Python 3.13  
- Biblioteca MQTT: `paho-mqtt`  
- Broker público: `broker.hivemq.com`  
- Comunicação IoT via LoRa (ESP32 + RFM95)  

---

## 📁 Estrutura do Projeto

```

lora_mqtt/
├── computador/
│   └── mqtt_client.py      # Cliente MQTT 
├── requirements.txt
└── README.md

````

> ⚠️ O diretório `venv/` e `dados/` não deve ser versionado (adicione ao `.gitignore`)

---

## ⚙️ Configuração

### 1. Clone o repositório

```bash
git clone <repo-url>
cd lora_mqtt
````

### 2. Crie um ambiente virtual

```bash
python -m venv venv

# Linux/macOS
source venv/bin/activate

# Windows
venv\Scripts\activate
```

### 3. Instale as dependências

```bash
pip install -r requirements.txt
```

---

### 4. Configurações de Rede

#### 📁 4.1. Criar o arquivo de configuração

Renomeie o arquivo:

```bash
secrets.h.template → secrets.h
```

#### ✏️ 4.2. Preencher as credenciais

```C
// Credenciais do seu WiFi   
const char* ssid     = "NOME_DA_SUA_REDE";
const char* password = "SENHA_DA_SUA_REDE";
```

#### 🌐 4.3. Configuração do Broker MQTT

Você pode usar um broker público (já configurado por padrão):

```C
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port     = 1883;
```

#### 📡 4.4. Tópicos MQTT

```C
const char* topic_DL = "sensor/led";
const char* topic_UL = "sensor/luminosidade";
```
🔎 O que isso significa:
* topic_UL → onde o sensor envia dados
* topic_DL → onde o dispositivo recebe comandos

#### 🚨 Erros comuns
❌ Esquecer as aspas "..."  
❌ Colocar espaço no final do nome da rede   
❌ Usar WiFi 5GHz (pode não funcionar dependendo do setup)  
❌ Digitar senha incorreta


## 🔧 Configurações do MQTT Client

No arquivo `python.py`:

```pythonzer uma coisa mais p
BROKER = "broker.hivemq.com"
TOPIC_LUM = "sensor/luminosidade"
TOPIC_LED = "sensor/led"

LIMIAR = 2000
TIMEOUT_RESPOSTA = 10.0
```
---

## ▶️ Execução e Testes

```bash
python computador/mqtt_client.py
```
Você pode testar a aplicação de ponta a ponta localmente utilizando o sensor simulado sem a necessidade do hardware físico:

### Passo 1: Inicializar o Sensor Fake
Em um terminal dedicado, execute o simulador:

```bash
python test/fake_sensor.py
```

### Passo 2: Inicializar o Cliente Principal

Em outro terminal, execute o script do ecossistema de controle e armazenamento:

```bash
python computador/mqtt_client.py
```
## 💾 Armazenamento Local (Persistência JSON)

Ao receber respostas válidas, o sistema gera de forma automática uma pasta chamada dados/ no mesmo diretório do arquivo executado e armazena os dados sob o seguinte formato estruturado:

```JSON
[
    {
        "date": "2026-10-05 12:30:01",
        "luminosidade": 1234
    }
]
```
Esta implementação atende ao Nível 4 de maturidade da TpM, viabilizando o processamento em lotes (Nível 5) e exibição em Dashboards históricos (Nível 6).

---

## 📡 Protocolo de Comunicação

### 📤 Requisição (Python → Sensor)

* Pacote de 52 bytes
* Byte relevante:

  * `byte[34]`: comando do LED

### 📥 Resposta (Sensor → Python)

* Pacote de 52 bytes
* Bytes relevantes:

  * `byte[17-18]`: valor de luminosidade (`uint16`)

---

## 🔁 Modo de Operação

O sistema roda em loop contínuo:

1. Envia pacote
2. Aguarda resposta (`threading.Event`)
3. Timeout de segurança (10s)
4. Reenvio automático

✔ Evita flooding
✔ Garante sincronização
✔ Detecta falhas de comunicação

---

## 🧵 Concorrência

O projeto utiliza:

* `threading.Thread` → envio contínuo
* `threading.Event` → sincronização entre envio e resposta

Implementando um padrão robusto de **controle síncrono sobre rede assíncrona**.

---

## ⚠️ Pontos de Atenção

* Uso de broker público (sem autenticação)
* Sem criptografia (MQTT plain)
* Dependência de conectividade estável
* Pacote binário fixo (hardcoded)

---

## 📈 Possíveis Melhorias

* 🔐 TLS + autenticação MQTT
* 📊 Persistência de dados (InfluxDB, PostgreSQL)
* 🌍 Dashboard (Grafana / Web UI)
* 📦 Serialização estruturada (CBOR / Protobuf)
* 🔄 QoS MQTT configurável
* 🧩 Modularização do código

---

## 🧪 Cenário de Uso

Ideal para:

* Projetos acadêmicos de IoT
* Testes com LoRaWAN / gateways próprios
* Prototipação de sistemas embarcados distribuídos
* Estudos de comunicação assíncrona controlada

---

## 📄 Licença

Este projeto está licenciado sob a licença MIT — veja o arquivo [LICENSE](LICENSE) para mais detalhes.

Você é livre para usar, modificar e distribuir este projeto para fins de estudo e ensino.

---