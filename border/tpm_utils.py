import base64
import struct

class TpMDecoder:
    @staticmethod
    def decode_uplink(payload_b64):
        try:
            data = base64.b64decode(payload_b64)
            if len(data) != 20:
                return None
            
            # Mapeamento conforme PRD 5.1.1
            res = {
                "rssi_mapped": data[0],
                "snr_mapped": struct.unpack(">H", data[1:3])[0],
                "dest_id": data[8],
                "src_id": data[10],
                "dl_count": struct.unpack(">H", data[12:14])[0],
                "ul_count": struct.unpack(">H", data[14:16])[0],
                "sensor_status": data[16],
                "pollution_level": data[17],
                "humidity": struct.unpack(">H", data[18:20])[0] / 10.0
            }
            return res
        except Exception:
            return None

class CommandBuilder:
    @staticmethod
    def build_downlink(dest_id, src_id, command_id, request_type, command_value):
        # Mapeamento conforme PRD 5.1.2
        packet = bytearray(20)
        packet[8] = dest_id
        packet[10] = src_id
        struct.pack_into(">H", packet, 12, command_id)
        packet[16] = request_type
        packet[17] = command_value
        
        return {
            "schema_version": "3.0",
            "command_id": f"cmd-{command_id}",
            "priority": 1,
            "tpm": {
                "payload_b64": base64.b64encode(packet).decode(),
                "size": 20
            }
        }
