import paho.mqtt.client as mqtt
import json
import time
from tpm_utils import TpMDecoder, CommandBuilder
import database

BROKER = "localhost"
PORT = 1883
# Inscreve em todos os gateways
TOPIC_RX = "airsense/+/rx"
TOPIC_ACK = "airsense/+/tx/ack"
TOPIC_STATUS = "airsense/+/status"

class BorderMQTT:
    def __init__(self):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_message = self.on_message

    def on_message(self, client, userdata, msg):
        topic_parts = msg.topic.split('/')
        gw_id = topic_parts[1]
        
        try:
            data = json.loads(msg.payload)
            
            if msg.topic.endswith('/rx'):
                # Processa Uplink
                payload_b64 = data['tpm']['payload_b64']
                decoded = TpMDecoder.decode_uplink(payload_b64)
                if decoded:
                    decoded['gw_id'] = gw_id
                    decoded['rssi'] = data['radio']['rssi_dbm']
                    decoded['snr'] = data['radio']['snr_db']
                    print(f"[Border] Dado recebido do sensor {decoded['src_id']} via {gw_id}")
                    database.save_sensor_data(decoded)
                    
                    # Publica dados decodificados para o Telegraf (PRD RF-DB-03)
                    telemetry = {
                        "sensor_id": decoded['src_id'],
                        "gateway_id": gw_id,
                        "humidity": decoded['humidity'],
                        "pollution": decoded['pollution_level'],
                        "rssi": decoded['rssi'],
                        "snr": decoded['snr']
                    }
                    self.client.publish(f"airsense/{gw_id}/decoded", json.dumps(telemetry))
                    
            elif msg.topic.endswith('/tx/ack'):
                # Processa ACK de downlink
                cmd_id = data.get('command_id')
                status = data.get('status')
                print(f"[Border] ACK recebido: CMD={cmd_id}, STATUS={status}")
                database.update_command_status(cmd_id, status)
                
        except Exception as e:
            print(f"[Border] Erro ao processar mensagem MQTT: {e}")

    def send_command(self, gw_id, sensor_id, req_type, value):
        cmd_id_num = int(time.time()) % 65535
        # No sensor, ID do gateway é 100
        cmd = CommandBuilder.build_downlink(sensor_id, 100, cmd_id_num, req_type, value)
        
        topic_tx = f"airsense/{gw_id}/tx"
        self.client.publish(topic_tx, json.dumps(cmd))
        
        cmd_type_str = "Intervalo" if req_type == 0x01 else "Threshold"
        database.save_command(cmd['command_id'], sensor_id, cmd_type_str, value)
        return cmd['command_id']

    def run(self):
        database.init_db()
        self.client.connect(BROKER, PORT)
        self.client.subscribe([(TOPIC_RX, 1), (TOPIC_ACK, 1), (TOPIC_STATUS, 0)])
        self.client.loop_start()
        print("[Border] MQTT Client iniciado.")

if __name__ == "__main__":
    border = BorderMQTT()
    border.run()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
