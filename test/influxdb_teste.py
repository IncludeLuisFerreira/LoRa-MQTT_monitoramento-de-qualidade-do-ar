from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import time

client = InfluxDBClient(
    url="http://localhost:8086",
    token="airsense-token-2026",
    org="airsense-org"
)

# Teste de escrita
write_api = client.write_api(write_options=SYNCHRONOUS)

point = Point("sensor_data") \
    .tag("sensor_id", "1") \
    .tag("gateway_id", "gw-001") \
    .field("humidity", 65.5) \
    .field("pollution", 42.3) \
    .field("rssi", -85) \
    .field("snr", 7.5) \
    .time(int(time.time()), write_precision='s')

write_api.write(bucket="airsense-bucket", record=point)
print("✓ Dado de teste escrito!")

write_api.close()
client.close()