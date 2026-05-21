import paho.mqtt.client as mqtt
from paho.mqtt.enums import CallbackAPIVersion
import time
import threading

# CONFIGURAÇÕES DO BROKER
BROKER = "broker.hivemq.com"   
TOPIC_LUM = "sensor/luminosidade"
TOPIC_LED = "sensor/led"

TAMANHO_PACOTE = 52
LIMIAR = 5  # Ajustado para escala do ESP32 (0 a 4095)
TIMEOUT_RESPOSTA = 10.0  # Tempo máximo de espera em segundos

# Constantes para os comandos dos LEDs
LED_DESLIGADO = 0
LED_VERMELHO  = 1
LED_AMARELO   = 2

# VARIÁVEL GLOBAL: Guarda o último estado decidido pelo Python
comando_led_atual = LED_DESLIGADO

# EVENTO DE SINCRONIZAÇÃO: Controla a espera da resposta do sensor
resposta_recebida = threading.Event()

def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Conectado ao broker ({BROKER}), código de retorno: {reason_code}")
    client.subscribe(TOPIC_LUM)
    
    # Inicia a thread de requisições apenas uma vez na primeira conexão
    if not hasattr(on_connect, "thread_iniciada"):
        threading.Thread(target=loop_requisicao, args=(client,), daemon=True).start()
        on_connect.thread_iniciada = True

def on_message(client, userdata, msg):
    global comando_led_atual
    payload = msg.payload
    
    if len(payload) != TAMANHO_PACOTE:
        return

    try:
        # Extrai a luminosidade (bytes 17 e 18)
        luminosidade = (payload[17] << 8) | payload[18]
        print(f"\n[UL] Resposta recebida do Sensor. Luminosidade: {luminosidade}")

        # Lógica de decisão para o PRÓXIMO envio
        if luminosidade < LIMIAR:
            print(f"→ Baixa luminosidade (< {LIMIAR}). Configurado para o próximo envio: LED VERMELHO")
            comando_led_atual = LED_VERMELHO
        else:
            print(f"→ Luminosidade adequada (>= {LIMIAR}). Configurado para o próximo envio: LED AMARELO")
            comando_led_atual = LED_AMARELO

        # CRÍTICO: Avisa a thread de envio que a resposta chegou!
        resposta_recebida.set()

    except Exception as e:
        print(f"Erro ao processar pacote recebido: {e}")

def loop_requisicao(client):
    """ Envia o pacote, aguarda a resposta ou dá timeout de 10 segundos """
    print("Agendador de requisições iniciado (Modo: Request-Response)...")
    
    while True:
        if client.is_connected():
            # Garante que o evento está limpo (falso) antes de enviar
            resposta_recebida.clear()
            
            # Monta o pacote de 52 bytes
            pacote_requisicao = bytearray(TAMANHO_PACOTE)
            pacote_requisicao[8] = 0x01   # ID do sensor
            pacote_requisicao[10] = 0x00  # ID do gateway
            pacote_requisicao[34] = comando_led_atual
            
            print(f"\n[DL] Enviando requisição. Comando LED atual (byte 34): {comando_led_atual}")
            client.publish(TOPIC_LED, pacote_requisicao)
            
            print(f" Aguardando resposta do sensor (Timeout: {TIMEOUT_RESPOSTA}s)...")
            
            # Bloqueia a thread aqui até receber o .set() ou estourar o tempo de 10s
            houve_resposta = resposta_recebida.wait(timeout=TIMEOUT_RESPOSTA)
            
            if houve_resposta:
                print(" Sucesso! Resposta confirmada. Preparando próximo envio em 1 segundo...")
                time.sleep(1) # Pequena pausa opcional para não inundar a rede se a resposta for instantânea
            else:
                print(f"⚠️ [TIMEOUT] Passaram-se {TIMEOUT_RESPOSTA}s sem resposta do sensor. Tentando novamente...")
        else:
            # Se o cliente MQTT desconectar, espera 1 segundo antes de checar novamente
            time.sleep(1)

# ============================================================================
# CONFIGURAÇÃO DO CLIENTE - PADRÃO OFICIAL API V2
# ============================================================================
client = mqtt.Client(CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message

print("Iniciando script de automação Python (Modo Síncrono)")
client.connect(BROKER, 1883, 60)
client.loop_forever()