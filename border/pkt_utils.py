import base64
import struct

class PktDecoder:
    @staticmethod
    def decode_uplink(payload_b64):
        try:
            data = base64.b64decode(payload_b64)
            if len(data) != 20:
                return None
            return {
                "rssi_dl": data[0],
                "snr_dl": data[1],
                "radio_status": data[2],
                "battery_level": data[3],
                "hardware_type": data[4],
                "sleep_time": data[5],
                "protocol_version": data[6],
                "mac_reserved": data[7],
                "dest_id": data[8],
                "net_reserved1": data[9],
                "src_id": data[10],
                "net_reserved2": data[11],
                "dl_count": data[12],
                "flags_ack": data[13],
                "ul_count": data[14],
                "transport_reserved": data[15],
                "sensor_status": data[16],
                "pollution_level": data[17],
                "humidity": struct.unpack(">H", data[18:20])[0] / 10.0
            }
        except Exception:
            return None


class CommandBuilder:
    @staticmethod
    def build_downlink(dest_id, src_id, command_id, request_type, command_value):
        """
        Retorna bytes (20) do pacote downlink conforme lora_packet.h
        - dest_id: ID do sensor (byte8)
        - src_id: ID do gateway (byte10) – deve ser 100
        - command_id: número sequencial (0-255) – será DL_COUNTER (byte12)
        - request_type: 0x00 (leitura), 0x01 (mudar sleep_time), etc.
        - command_value: valor (ex: novo sleep_time em minutos)
        """
        packet = bytearray(20)
        
        # Camada Física
        packet[0] = 0          # RSSI_DL (desconhecido)
        packet[1] = 0          # SNR_DL
        packet[2] = 0          # radio_status
        packet[3] = 0          # battery_level
        packet[4] = 0x01       # hardware_type (padrão)
        
        # Camada MAC
        packet[5] = 0          # sleep_time (será alterado via comando)
        packet[6] = 0x01       # protocol_version
        packet[7] = 0          # reservado
        
        # Camada NET
        packet[8] = dest_id & 0xFF
        packet[9] = 0          # reservado
        packet[10] = src_id & 0xFF
        packet[11] = 0         # reservado
        
        # Camada Transporte
        packet[12] = command_id & 0xFF   # DL_COUNTER
        packet[13] = 0x00                # flags_ack
        packet[14] = 0x00                # UL_COUNTER
        packet[15] = 0x00                # reservado
        
        # Camada Aplicação Downlink
        packet[16] = request_type & 0xFF
        packet[17] = command_value & 0xFF
        packet[18] = 0x00
        packet[19] = 0x00
        
        return bytes(packet)