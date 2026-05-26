import paho.mqtt.client as mqtt
from paho.mqtt.enums import CallbackAPIVersion
import time
import threading
import json
import os
from datetime import datetime
import struct

PROTOCOL_VERSION = 0x01

# CONFIGURAÇÕES DO BROKER
BROKER = "broker.hivemq.com"
TOPIC_LUM = "sensor/luminosidade"
TOPIC_LED = "sensor/led"

TAMANHO_PACOTE = 52
LIMIAR = 5
TIMEOUT_RESPOSTA = 10.0

# --- CONFIGURAÇÃO DE CAMINHOS AUTOMÁTICOS ---
DIRETORIO_ATUAL = os.path.dirname(os.path.abspath(__file__))
PASTA_DADOS = os.path.join(DIRETORIO_ATUAL, "dados")
ARQUIVO_DADOS = os.path.join(PASTA_DADOS, "dados_luminosidade.json")
# --------------------------------------------

# Constantes para os comandos dos LEDs
LED_DESLIGADO = 0
LED_VERMELHO  = 1
LED_AMARELO   = 2

# Camada MAC
BYTE_SLEEPTIME = 5
BYTE_PROTOCOL_VERSION = 6

# Camada NET
BYTE_ID_DESTINO = 8
BYTE_ID_ORIGEM = 10

# Camada TRANSP
BYTE_COUNT_DL = 12
BYTE_COUNT_UL = 14
BYTE_LUMINOSIDADE = 17
BYTE_LED_CMD = 34
BYTE_PACKET_ID = 36  # NOVO: 2 bytes para identificar cada transação

sleeptime = 5
id_origem = 0
id_destino = 1

# Variáveis globais protegidas por Lock
_lock = threading.Lock()
quantidade_pacote_DL = 0
quantidade_pacote_UL = 0
packet_id_atual = 0
comando_led_atual = LED_DESLIGADO

# Evento e packet_id da transação atual (evita condição de corrida)
_tx_event = threading.Event()
_tx_packet_id = -1


def salvar_dados_json(luminosidade, pkt_id):
    """Salva dados em JSON local."""
    try:
        os.makedirs(PASTA_DADOS, exist_ok=True)
        try:
            with open(ARQUIVO_DADOS, "r") as f:
                dados = json.load(f)
                if not isinstance(dados, list):
                    dados = []
        except (FileNotFoundError, json.JSONDecodeError):
            dados = []

        novo_registro = {
            "date": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            "luminosidade": luminosidade,
            "packet_id": pkt_id
        }
        dados.append(novo_registro)

        with open(ARQUIVO_DADOS, "w") as f:
            json.dump(dados, f, indent=4)
        print(f"💾 Dados salvos com sucesso em: {ARQUIVO_DADOS}")

    except Exception as e:
        print(f"⚠️ Erro ao persistir dados no JSON: {e}")


def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code != 0:
        print(f"❌ Conexão recusada pelo broker, código: {reason_code}")
        return

    print(f"Conectado ao broker ({BROKER}), código: {reason_code}")
    client.subscribe(TOPIC_LUM)

    if not hasattr(on_connect, "thread_iniciada"):
        threading.Thread(target=loop_requisicao, args=(client,), daemon=True).start()
        on_connect.thread_iniciada = True


def on_message(client, userdata, msg):
    global quantidade_pacote_UL, comando_led_atual

    payload = msg.payload
    if len(payload) != TAMANHO_PACOTE:
        print(f"⚠️ Payload ignorado: tamanho {len(payload)} (esperado {TAMANHO_PACOTE})")
        return

    try:
        # Lê packet_id (2 bytes, big-endian)
        packet_id_recebido = struct.unpack_from(">H", payload, BYTE_PACKET_ID)[0]
        # Lê luminosidade (2 bytes, big-endian)
        luminosidade = struct.unpack_from(">H", payload, BYTE_LUMINOSIDADE)[0]
    except Exception as e:
        print(f"⚠️ Erro ao extrair dados do pacote: {e}")
        return

    with _lock:
        quantidade_pacote_UL = (quantidade_pacote_UL + 1) & 0xFFFF
        ul_local = quantidade_pacote_UL

    print(f"\n[UL] Resposta recebida do Sensor. packet_id={packet_id_recebido}, Luminosidade={luminosidade}")

    # Verifica se é a resposta esperada
    global _tx_packet_id, _tx_event
    if packet_id_recebido == _tx_packet_id:
        _tx_event.set()
    else:
        print(f"⚠️ packet_id={packet_id_recebido} não corresponde ao esperado ({_tx_packet_id}). Possível resposta tardia.")

    salvar_dados_json(luminosidade, packet_id_recebido)

    # Lógica de decisão para o PRÓXIMO envio
    if luminosidade < LIMIAR:
        print(f"→ Baixa luminosidade (< {LIMIAR}). Configurado para o próximo envio: LED VERMELHO")
        comando_led_atual = LED_VERMELHO
    else:
        print(f"→ Luminosidade adequada (>= {LIMIAR}). Configurado para o próximo envio: LED AMARELO")
        comando_led_atual = LED_AMARELO


def loop_requisicao(client):
    global quantidade_pacote_DL, comando_led_atual, packet_id_atual
    global _tx_event, _tx_packet_id

    print("Agendador de requisições iniciado (Modo: Request-Response)...")
    timeouts_seguidos = 0

    while True:
        if client.is_connected():
            # Cria um novo Event limpo para esta transação
            _tx_event = threading.Event()

            with _lock:
                packet_id_atual = (packet_id_atual + 1) & 0xFFFF
                pkt_id = packet_id_atual
                quantidade_pacote_DL = (quantidade_pacote_DL + 1) & 0xFFFF
                dl_local = quantidade_pacote_DL
                ul_local = quantidade_pacote_UL

            _tx_packet_id = pkt_id

            # Monta o pacote de 52 bytes
            pacote_requisicao = bytearray(TAMANHO_PACOTE)
            pacote_requisicao[BYTE_SLEEPTIME] = sleeptime
            pacote_requisicao[BYTE_PROTOCOL_VERSION] = PROTOCOL_VERSION
            struct.pack_into(">H", pacote_requisicao, BYTE_ID_DESTINO, id_destino)
            struct.pack_into(">H", pacote_requisicao, BYTE_ID_ORIGEM, id_origem)
            struct.pack_into(">H", pacote_requisicao, BYTE_COUNT_DL, dl_local)
            struct.pack_into(">H", pacote_requisicao, BYTE_COUNT_UL, ul_local)
            pacote_requisicao[BYTE_LED_CMD] = comando_led_atual
            struct.pack_into(">H", pacote_requisicao, BYTE_PACKET_ID, pkt_id)

            print(f"\n[DL] Enviando requisição. packet_id={pkt_id}, LED={comando_led_atual}")
            client.publish(TOPIC_LED, pacote_requisicao)

            print(f" Aguardando resposta do sensor (Timeout: {TIMEOUT_RESPOSTA}s)...")
            houve_resposta = _tx_event.wait(timeout=TIMEOUT_RESPOSTA)

            if houve_resposta:
                print(" Sucesso! Resposta confirmada. Próximo envio em 1s...")
                timeouts_seguidos = 0
                time.sleep(1)
            else:
                timeouts_seguidos += 1
                espera = min(2 ** timeouts_seguidos, 30)  # backoff simples, máx 30s
                print(f"⚠️ [TIMEOUT] packet_id={pkt_id} sem resposta. Tentando novamente em {espera}s...")
                time.sleep(espera)
        else:
            time.sleep(1)


def main():
    print("Iniciando script de automação Python (Modo Síncrono)")
    client = mqtt.Client(CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(BROKER, 1883, 60)
    client.loop_forever()


if __name__ == "__main__":
    main()