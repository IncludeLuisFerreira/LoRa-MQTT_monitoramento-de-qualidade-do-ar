# Este código emula um gateway lora enviando os dados para o broker
import paho.mqtt.client as mqtt
from paho.mqtt.enums import CallbackAPIVersion
import random
import time

# CONFIGURAÇÕES DO BROKER (Iguais ao script principal)
BROKER = "broker.hivemq.com"   
TOPIC_LUM = "sensor/luminosidade"
TOPIC_LED = "sensor/led"

TAMANHO_PACOTE = 52

def on_connect(client, userdata, flags, reason_code, properties):
    print(f"🤖 [FAKE SENSOR] Conectado ao broker. Código: {reason_code}")
    # O sensor escuta o tópico onde o gateway envia as requisições/comandos
    client.subscribe(TOPIC_LED)
    print(f"📥 Escutando o tópico de comandos: {TOPIC_LED}")

def on_message(client, userdata, msg):
    payload = msg.payload
    
    # Valida se o pacote recebido do gateway está no tamanho correto
    if len(payload) != TAMANHO_PACOTE:
        print(f"⚠️ [FAKE SENSOR] Pacote inválido recebido (Tamanho: {len(payload)} bytes)")
        return

    # Extrai o comando do LED enviado pelo Python (Byte 34)
    comando_led = payload[34]
    print(f"\n📥 [REQUISIÇÃO RECEBIDA] Comando do LED recebido do Gateway: {comando_led}")

    # Simula a leitura de um sensor real (Gera valores de 0 a 15 para testar o limiar de 5)
    luminosidade_fake = random.randint(0, 15)
    print(f"🔮 [LEITURA] Sensor leu luminosidade fake: {luminosidade_fake}")

    # Monta o pacote de resposta de 52 bytes
    pacote_resposta = bytearray(TAMANHO_PACOTE)
    
    # Insere a luminosidade nos bytes 17 e 18 (Operações de Bitwise idênticas ao hardware)
    pacote_resposta[17] = (luminosidade_fake >> 8) & 0xFF
    pacote_resposta[18] = luminosidade_fake & 0xFF

    # Simula um pequeno delay de processamento do hardware (opcional)
    time.sleep(0.2)

    # Envia a resposta de volta para o script Python
    print(f"📤 [RESPOSTA] Enviando pacote de 52 bytes para o tópico: {TOPIC_LUM}")
    client.publish(TOPIC_LUM, pacote_resposta)

# ============================================================================
# EXECUÇÃO DO SENSOR SIMULADO
# ============================================================================
client = mqtt.Client(CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message

print("🚀 Iniciando Simulador de Sensor (ESP32 Fake)...")
client.connect(BROKER, 1883, 60)
client.loop_forever()

