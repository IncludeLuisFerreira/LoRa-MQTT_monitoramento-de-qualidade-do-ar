import pytest
from border.tpm_utils import TpMDecoder, CommandBuilder
import base64
import struct

class TestTpM:
    def test_decode_uplink(self):
        # Cria um pacote fake de 20 bytes
        packet = bytearray(20)
        packet[0] = 100 # RSSI
        struct.pack_into(">H", packet, 1, 85) # SNR 8.5
        packet[8] = 100 # DEST_ID
        packet[10] = 1 # SRC_ID
        struct.pack_into(">H", packet, 12, 2) # DL_COUNT
        struct.pack_into(">H", packet, 14, 50) # UL_COUNT
        packet[16] = 0x03 # STATUS
        packet[17] = 42 # POLLUTION
        struct.pack_into(">H", packet, 18, 653) # HUMIDITY 65.3
        
        b64 = base64.b64encode(packet).decode()
        res = TpMDecoder.decode_uplink(b64)
        
        assert res['rssi_mapped'] == 100
        assert res['snr_mapped'] == 85
        assert res['src_id'] == 1
        assert res['pollution_level'] == 42
        assert res['humidity'] == 65.3

    def test_build_downlink(self):
        res = CommandBuilder.build_downlink(1, 100, 1234, 0x01, 60)
        
        assert res['command_id'] == "cmd-1234"
        assert res['tpm']['size'] == 20
        
        payload = base64.b64decode(res['tpm']['payload_b64'])
        assert payload[8] == 1 # DEST_ID
        assert payload[10] == 100 # SRC_ID
        assert struct.unpack(">H", payload[12:14])[0] == 1234 # CMD_ID
        assert payload[16] == 0x01 # TYPE
        assert payload[17] == 60 # VALUE
