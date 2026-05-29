import time
import random
import struct
import paho.mqtt.client as mqtt
import base64

# Configurações
BROKER = "localhost"  # Usaremos localhost pois o mosquitto rodará no sandbox
PORT = 1883
RADIO_TOPIC = "airsense/radio/virtual"  # Tópico que simula o meio físico (PHY)
SENSOR_ID = 1
GATEWAY_ID = 100

class FakeSensor:
    def __init__(self):
        self.ul_count = 0
        self.dl_count = 0
        self.interval = 30
        self.client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self.client.on_message = self.on_message

    def on_message(self, client, userdata, msg):
        # Simula recebimento via LoRa (PHY)
        payload = msg.payload
        if len(payload) == 20:
            dest_id = payload[8]
            if dest_id == SENSOR_ID:
                command_id = struct.unpack(">H", payload[12:14])[0]
                req_type = payload[16]
                value = payload[17]
                print(f"[Sensor] Downlink recebido! CMD={command_id}, TYPE={req_type}, VAL={value}")
                self.dl_count += 1
                if req_type == 0x01:
                    self.interval = value
                    print(f"[Sensor] Novo intervalo: {self.interval}s")

    def run(self):
        self.client.connect(BROKER, PORT)
        self.client.subscribe(RADIO_TOPIC)
        self.client.loop_start()

        print(f"[Sensor] Iniciado. ID={SENSOR_ID}, Intervalo={self.interval}s")

        try:
            while True:
                # Simula leitura de sensores
                humidity = random.uniform(30, 90)
                pollution = random.randint(0, 150)
                status = 0x03 # DHT22 + ZP07

                # Monta pacote TpM (20 bytes)
                # Byte 0: RSSI Mapped
                # Byte 1-2: SNR Mapped (x10)
                # Byte 8: DEST_ID
                # Byte 10: SRC_ID
                # Byte 12-13: DL_COUNT
                # Byte 14-15: UL_COUNT
                # Byte 16: STATUS
                # Byte 17: POLLUTION
                # Byte 18-19: HUMIDITY (x10)
                
                self.ul_count += 1
                packet = bytearray(20)
                packet[0] = 100 # RSSI fake
                struct.pack_into(">H", packet, 1, 85) # SNR fake 8.5
                packet[8] = GATEWAY_ID
                packet[10] = SENSOR_ID
                struct.pack_into(">H", packet, 12, self.dl_count)
                struct.pack_into(">H", packet, 14, self.ul_count)
                packet[16] = status
                packet[17] = pollution
                struct.pack_into(">H", packet, 18, int(humidity * 10))

                print(f"[Sensor] Enviando Uplink #{self.ul_count}: Hum={humidity:.1f}%, Pol={pollution}")
                self.client.publish(RADIO_TOPIC, packet)
                
                time.sleep(self.interval)
        except KeyboardInterrupt:
            self.client.loop_stop()

if __name__ == "__main__":
    sensor = FakeSensor()
    sensor.run()
