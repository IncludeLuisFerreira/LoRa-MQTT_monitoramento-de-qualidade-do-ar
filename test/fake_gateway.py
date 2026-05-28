import time
import json
import paho.mqtt.client as mqtt
import base64
import struct

# Configurações
BROKER = "localhost"
PORT = 1883
RADIO_TOPIC = "airsense/radio/virtual"
GW_ID = "gw-001"
GW_NUM_ID = 100

TOPIC_RX = f"airsense/{GW_ID}/rx"
TOPIC_TX = f"airsense/{GW_ID}/tx"
TOPIC_ACK = f"airsense/{GW_ID}/tx/ack"
TOPIC_STATUS = f"airsense/{GW_ID}/status"

class FakeGateway:
    def __init__(self):
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_message = self.on_message

    def on_message(self, client, userdata, msg):
        if msg.topic == RADIO_TOPIC:
            # Recebeu do rádio (Uplink LoRa)
            payload = msg.payload
            if len(payload) == 20:
                dest_id = payload[8]
                if dest_id == GW_NUM_ID:
                    print(f"[Gateway] Uplink recebido do sensor {payload[10]}")
                    # Converte para Envelope JSON
                    envelope = {
                        "schema_version": "3.0",
                        "ts": int(time.time()),
                        "gateway_id": GW_ID,
                        "radio": {
                            "rssi_dbm": -50.0,
                            "snr_db": 9.0,
                            "freq_hz": 915000000
                        },
                        "tpm": {
                            "payload_b64": base64.b64encode(payload).decode(),
                            "payload_hex": payload.hex(),
                            "size": 20
                        }
                    }
                    self.client.publish(TOPIC_RX, json.dumps(envelope))

        elif msg.topic == TOPIC_TX:
            # Recebeu comando do Border (Downlink MQTT)
            print(f"[Gateway] Comando downlink recebido via MQTT")
            try:
                data = json.loads(msg.payload)
                payload_b64 = data['tpm']['payload_b64']
                tpm_payload = base64.b64decode(payload_b64)
                
                if len(tpm_payload) == 20:
                    print(f"[Gateway] Retransmitindo para o rádio LoRa...")
                    self.client.publish(RADIO_TOPIC, tpm_payload)
                    
                    # Publica ACK
                    ack = {
                        "command_id": data.get("command_id", "unknown"),
                        "status": "transmitted",
                        "ts": int(time.time())
                    }
                    self.client.publish(TOPIC_ACK, json.dumps(ack))
            except Exception as e:
                print(f"[Gateway] Erro ao processar downlink: {e}")

    def run(self):
        self.client.connect(BROKER, PORT)
        self.client.subscribe([(RADIO_TOPIC, 0), (TOPIC_TX, 0)])
        self.client.loop_start()
        
        print(f"[Gateway] Iniciado. ID={GW_ID}")
        
        try:
            while True:
                # Heartbeat
                status = {"status": "online", "ts": int(time.time())}
                self.client.publish(TOPIC_STATUS, json.dumps(status))
                time.sleep(30)
        except KeyboardInterrupt:
            self.client.loop_stop()

if __name__ == "__main__":
    gw = FakeGateway()
    gw.run()
