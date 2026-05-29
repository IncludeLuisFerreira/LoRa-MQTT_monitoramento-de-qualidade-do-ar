import base64

import paho.mqtt.client as mqtt
import json
import time
from tpm_utils import TpMDecoder, CommandBuilder
import database
import threading

BROKER = "localhost"
PORT = 1883
TOPIC_RX = "airsense/+/rx"
TOPIC_ACK = "airsense/+/tx/ack"
TOPIC_STATUS = "airsense/+/status"

class BorderMQTT:
    def __init__(self):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_message = self.on_message
        self.client.on_connect = self.on_connect
        self.timeStamp = 30
        self.dl_counter = 0
        self.cmd_sequence = 0   # novo contador incremental

    def on_connect(self, client, userdata, flags, rc, properties=None):
        print(f"[Border] Conectado ao broker com código {rc}")
        self.client.subscribe([(TOPIC_RX, 1), (TOPIC_ACK, 1), (TOPIC_STATUS, 0)])
        
        def periodic_send():
            while True:
                time.sleep(self.timeStamp)
                try:
                    # Envia comando de leitura (request_type=0x00, value=0) para sensor 1
                    self.send_command("gw-001", dest_id=1, req_type=0x00, value=0)
                    print("[Periodic] Pacote de requisição enviado")
                except Exception as e:
                    print(f"[Periodic] Erro: {e}")
        threading.Thread(target=periodic_send, daemon=True).start()

    def on_message(self, client, userdata, msg):
        topic_parts = msg.topic.split('/')
        gw_id = topic_parts[1]
        
        try:
            if msg.topic.endswith('/rx'):
                # O gateway publica JSON com payload_hex, não base64
                data = json.loads(msg.payload)
                hex_payload = data.get('payload_hex', '')
                if hex_payload:
                    payload_bytes = bytes.fromhex(hex_payload)
                    decoded = TpMDecoder.decode_uplink(base64.b64encode(payload_bytes).decode())
                    if decoded:
                        decoded['gw_id'] = gw_id
                        decoded['rssi'] = data.get('rssi_dbm', 0)
                        decoded['snr'] = data.get('snr_db', 0)
                        print(f"[Border] Dado recebido do sensor {decoded['src_id']} via {gw_id}")
                        database.save_sensor_data(decoded)
                        
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
                # ACK do gateway: {"status": "transmitted"} ou similar
                ack_data = json.loads(msg.payload)
                status = ack_data.get('status')
                print(f"[Border] ACK recebido: {status}")
                # Se quiser associar a um comando, precisaria de um mecanismo (ex: salvar último command_id)
                # Por simplicidade, ignoramos o command_id específico
                
        except Exception as e:
            print(f"[Border] Erro ao processar mensagem MQTT: {e}")

    def send_command(self, gw_id, dest_id, req_type, value):
        src_id = 100
        # Incrementa contador de sequência
        self.cmd_sequence = (self.cmd_sequence + 1) % 1000000
        # Gera ID único: timestamp (ms) + sequência
        unique_id = f"{int(time.time() * 1000)}-{self.cmd_sequence}"
        # Usa o self.dl_counter para o campo DL_COUNTER (0-255)
        dl_counter_byte = self.dl_counter
        self.dl_counter = (self.dl_counter + 1) % 256
        
        packet = CommandBuilder.build_downlink(dest_id, src_id, dl_counter_byte, req_type, value)
        topic_tx = f"airsense/{gw_id}/tx"
        self.client.publish(topic_tx, packet)
        
        cmd_type_str = "Leitura" if req_type == 0x00 else "SleepTime" if req_type == 0x01 else f"Tipo{req_type}"
        database.save_command(unique_id, dest_id, cmd_type_str, value)
        return unique_id

    def run(self):
        database.init_db()
        self.client.connect(BROKER, PORT)
        self.client.loop_start()
        print("[Border] MQTT Client iniciado.")

    def set_timeStamp(self, timeStamp):
        self.timeStamp = timeStamp