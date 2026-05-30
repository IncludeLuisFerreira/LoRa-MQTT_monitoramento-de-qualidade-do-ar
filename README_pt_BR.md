[🇺🇸 English](README.md) | 🇧🇷 Português

<div align="center">

<img src="img/project_icon.png" width="500" alt="Logo LoRa Air Quality Monitor">

# LoRa Air Quality Monitor

Plataforma IoT profissional para monitoramento ambiental utilizando LoRaWAN, MQTT, InfluxDB e Grafana.

<br>

<img src="img/icons/mqtt-icon-transparent.svg" width="45" alt="MQTT" title="MQTT">
&nbsp;&nbsp;
<img src="img/icons/python-logo-only.svg" width="45" alt="Python" title="Python">
&nbsp;&nbsp;
<img src="img/icons/cubo-pink.svg" width="45" alt="InfluxDB" title="InfluxDB">
&nbsp;&nbsp;
<img src="img/icons/Grafana.svg" width="45" alt="Grafana" title="Grafana">
&nbsp;&nbsp;
<img src="img/icons/docker-mark-ocean-blue.svg" width="45" alt="Docker" title="Docker">

<br>

[![Licença: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![IoT](https://img.shields.io/badge/IoT-LoRa-blue.svg)](#)
[![MQTT](https://img.shields.io/badge/Protocol-MQTT-green.svg)](#)
[![InfluxDB](https://img.shields.io/badge/Database-InfluxDB%202.x-68a063.svg)](#)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](#)

</div>

O **LoRa Air Quality Monitor** é uma solução IoT completa e de nível profissional desenvolvida para monitoramento ambiental em áreas remotas. Utilizando o longo alcance da tecnologia **LoRa**, a eficiência do protocolo **MQTT** e o armazenamento de séries temporais do **InfluxDB**, este projeto fornece uma infraestrutura robusta para monitoramento de temperatura, umidade e pressão atmosférica em grandes distâncias.

---

## 🚀 Visão Geral

O sistema coleta dados ambientais de dispositivos finais (nós) remotos utilizando modulação LoRa. Esses pacotes são recebidos por um Gateway central, responsável por interligar a rede LoRa a uma rede IP por meio da publicação de mensagens em um Broker MQTT Mosquitto. Um serviço assinante dedicado consome essas mensagens e as persiste no InfluxDB para análise em tempo real e visualização através do Grafana.

### Principais Funcionalidades

* **Comunicação de Longo Alcance:** Utiliza LoRa para cobertura ampla com baixo consumo de energia.
* **Arquitetura Escalável:** Permite adicionar novos sensores e gateways facilmente.
* **Otimizado para Séries Temporais:** Armazenamento e consultas eficientes para dados de sensores.
* **Infraestrutura Containerizada:** Implantação simplificada com Docker e Docker Compose.
* **Monitoramento em Tempo Real:** Fluxo de dados pronto para dashboards.

---

## 🏗️ Arquitetura do Sistema

```text
┌─────────────────┐       ┌─────────────────┐       ┌───────────────────┐
│ Nó LoRa         │       │ Gateway LoRa    │       │ Broker MQTT       │
│ (ESP32+SX1276)  ├──────▶│ Raspberry Pi /  ├──────▶│ Mosquitto         │
│ Temp/Umid/Press │ LoRa  │ Bridge ESP32    │ MQTT  │                   │
└─────────────────┘       └─────────────────┘       └─────────┬─────────┘
                                                              │
                                                              │ Subscribe
                                                              ▼
┌─────────────────┐       ┌─────────────────┐       ┌───────────────────┐
│ Grafana         │       │ InfluxDB        │       │ Ingestor Python   │
│ Dashboards      │◀──────┤ Séries Temporais│◀──────┤ Subscriber        │
└─────────────────┘       └─────────────────┘       └───────────────────┘
```

---

## 🛠️ Requisitos de Hardware

| Componente            | Descrição                       | Exemplo                                                    |
| --------------------- | ------------------------------- | ---------------------------------------------------------- |
| **Dispositivo Final** | Microcontrolador com rádio LoRa | ESP32 + SX1276 (Heltec WiFi LoRa 32 / TTGO LoRa32)         |
| **Sensores**          | Sensores ambientais             | BME280 (Temperatura, Umidade e Pressão)                    |
| **Gateway**           | Ponte entre LoRa e Internet     | Raspberry Pi com LoRa HAT ou Gateway LoRa baseado em ESP32 |
| **Alimentação**       | Energia para nós remotos        | Baterias 18650 + Painel Solar                              |

---

## 💻 Stack de Software

* **Mosquitto:** Broker MQTT leve e eficiente.
* **InfluxDB 2.7:** Banco de dados de séries temporais de alto desempenho.
* **Python 3.x:** Lógica de ingestão e processamento de dados.
* **Docker:** Containerização e orquestração dos serviços.
* **Grafana:** Plataforma de visualização e dashboards.

---

## 📂 Estrutura do Projeto

```text
.
├── config/
│   └── mosquitto.conf
├── docker/
│   └── docker-compose.yml
├── docs/
│   └── architecture.png
├── gateway/
│   └── subscriber.py
└── README.md
```

---

## 🔧 Instalação e Configuração

### Pré-requisitos

* Docker e Docker Compose instalados.
* Python 3.10 ou superior.
* Conhecimentos básicos sobre LoRa e MQTT.

### Passo 1: Iniciar a Infraestrutura

```bash
cd docker
docker-compose up -d
```

Isso iniciará os serviços:

* Mosquitto
* InfluxDB
* Grafana

### Passo 2: Configurar o InfluxDB

O arquivo `docker-compose.yml` inicializa o InfluxDB com:

* **Org:** `my-org`
* **Bucket:** `lora_data`
* **Token:** `my-super-secret-auth-token`

### Passo 3: Instalar Dependências Python

```bash
pip install paho-mqtt influxdb-client
```

---

## <img src="img/icons/mqtt-icon-transparent.svg" width="40"> Estrutura de Tópicos MQTT e Payload

### Padrão de Tópico

```text
lora/devices/{device_id}/data
```

### Exemplo de Payload JSON

```json
{
  "temperature": 24.5,
  "humidity": 55.2,
  "pressure": 1013.2,
  "location": "field-alpha-01"
}
```

---

## <img src="img/icons/cubo-pink.svg" width="40"> Esquema de Dados no InfluxDB

O script `subscriber.py` realiza o mapeamento dos dados MQTT para o InfluxDB da seguinte forma:

* **Bucket:** `lora_data`
* **Measurement:** `sensor_data`

### Tags

* `device_id`
* `location`

### Fields

* `temperature`
* `humidity`
* `pressure`

---

## <img src="img/icons/python-logo-only.svg" width="40"> Script de Ingestão (Subscriber)

O arquivo `gateway/subscriber.py` é responsável por integrar o MQTT ao InfluxDB.

### Funcionamento

1. Conecta ao broker Mosquitto.
2. Inscreve-se no tópico `lora/devices/+/data`.
3. Recebe e interpreta o payload JSON.
4. Cria um `Point` do InfluxDB.
5. Armazena os dados no banco.

### Execução

```bash
python gateway/subscriber.py
```

---

## 📈 Utilização e Visualização

### Consultando Dados (Flux)

```flux
from(bucket: "lora_data")
  |> range(start: -1h)
  |> filter(fn: (r) => r["_measurement"] == "sensor_data")
  |> filter(fn: (r) => r["_field"] == "temperature")
```

---

## <img src="img/icons/Grafana.svg" width="40"> Integração com Grafana

1. Acesse `http://localhost:3000`.
2. Faça login (admin/admin).
3. Adicione uma nova fonte de dados.
4. Selecione **InfluxDB**.
5. Configure a linguagem de consulta como **Flux**.
6. Defina a URL `http://influxdb:8086`.
7. Informe Org, Bucket e Token.
8. Crie dashboards e painéis de séries temporais.

<p align="center">
  <img src="img/dashboard_grafana.png" width="90%" alt="Dashboard Grafana">
</p>

---

## ❓ Solução de Problemas

* **Connection Refused (MQTT):** Verifique se o Mosquitto está em execução e configurado corretamente.
* **Perda de Pacotes LoRa:** Verifique distância, antenas e configuração do Spreading Factor (SF).
* **Erro de Autenticação no InfluxDB:** Confirme se o token utilizado corresponde ao definido no Docker Compose.
* **Erro de JSON:** Certifique-se de que o Gateway está enviando mensagens JSON válidas.

---

## 🤝 Contribuindo

Contribuições são bem-vindas!

1. Faça um Fork do projeto.
2. Crie uma branch para sua funcionalidade.
3. Realize suas alterações.
4. Faça commit das mudanças.
5. Envie para seu fork.
6. Abra um Pull Request.

---

## 📜 Licença

Distribuído sob a licença MIT. Consulte o arquivo `LICENSE` para mais informações.

---

*Desenvolvido com ❤️ para a comunidade IoT.*
