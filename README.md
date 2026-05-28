# AirSense v3.0

Monitoramento Ambiental com LoRa/MQTT.

## Estrutura do Projeto
- `firmware/`: Código para Arduino IDE.
  - `sensor_node/`: Abrir `sensor_node.ino` no Arduino IDE.
  - `gateway/`: Abrir `gateway.ino` no Arduino IDE.
- `border/`: Elemento de borda em Python (Flask + MQTT + SQLite).
- `infra/`: Configurações Docker Compose (Mosquitto, InfluxDB, Telegraf, Grafana).
- `test/`: Simuladores de hardware e scripts de teste.
- `docs/`: Documentação do projeto (PRD).

## Como Executar (Simulação)
1. Instale as dependências: `pip install flask paho-mqtt`
2. Suba a infraestrutura: `docker-compose -f infra/docker-compose.yml up -d`
3. Inicie o Border: `python3 border/app.py`
4. Inicie os simuladores:
   - `python3 test/fake_gateway.py`
   - `python3 test/fake_sensor.py`
5. Acesse: `http://localhost:5000`

## Credenciais de Desenvolvimento
- Admin: `admin` / `admin123`
- Usuário: `user` / `user123`
