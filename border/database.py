import time
import threading
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS

# Configurações do InfluxDB
INFLUXDB_URL = "http://localhost:8086"  # Use "http://influxdb:8086" dentro do Docker
INFLUXDB_TOKEN = "airsense-token-2026"
INFLUXDB_ORG = "airsense-org"
INFLUXDB_BUCKET = "airsense-bucket"

# Singleton: variáveis globais
_client = None
_write_api = None
_query_api = None
_db_lock = threading.Lock()

def init_db():
    """Inicializa o cliente InfluxDB (singleton)."""
    global _client, _write_api, _query_api
    
    with _db_lock:
        if _client is None:
            try:
                _client = InfluxDBClient(
                    url=INFLUXDB_URL,
                    token=INFLUXDB_TOKEN,
                    org=INFLUXDB_ORG,
                    debug=False
                )
                _write_api = _client.write_api(write_options=SYNCHRONOUS)
                _query_api = _client.query_api()
                print("[Database] ✓ InfluxDB conectado!")
            except Exception as e:
                print(f"[Database] ✗ ERRO ao conectar: {e}")
                raise

def save_sensor_data(data):
    """Salva dados do sensor no InfluxDB."""
    if _write_api is None:
        print("[Database] ✗ ERRO: InfluxDB não inicializado.")
        return False
    
    try:
        point = Point("sensor_data") \
            .tag("sensor_id", str(data['src_id'])) \
            .tag("gateway_id", data['gw_id']) \
            .field("humidity", float(data['humidity'])) \
            .field("pollution", float(data['pollution_level'])) \
            .field("rssi", int(data['rssi'])) \
            .field("snr", float(data['snr'])) \
            .time(int(time.time()), write_precision='s')
        
        _write_api.write(bucket=INFLUXDB_BUCKET, record=point)
        return True
    except Exception as e:
        print(f"[Database] ✗ ERRO ao salvar sensor: {e}")
        return False

def save_command(cmd_id, sensor_id, cmd_type, value):
    """Salva comando enviado para o sensor."""
    if _write_api is None:
        print("[Database] ✗ ERRO: InfluxDB não inicializado.")
        return False
    
    try:
        point = Point("commands") \
            .tag("command_id", cmd_id) \
            .tag("sensor_id", str(sensor_id)) \
            .tag("type", cmd_type) \
            .field("value", str(value)) \
            .field("status", "pendente") \
            .time(int(time.time()), write_precision='s')
        
        _write_api.write(bucket=INFLUXDB_BUCKET, record=point)
        return True
    except Exception as e:
        print(f"[Database] ✗ ERRO ao salvar comando: {e}")
        return False

def update_command_status(cmd_id, status):
    """Atualiza status de comando (escreve novo ponto no InfluxDB)."""
    if _write_api is None or _query_api is None:
        print("[Database] ✗ ERRO: InfluxDB não inicializado.")
        return False
    
    try:
        # Busca comando original
        query = f'''
        from(bucket: "{INFLUXDB_BUCKET}")
          |> range(start: -24h)
          |> filter(fn: (r) => r._measurement == "commands")
          |> filter(fn: (r) => r.command_id == "{cmd_id}")
          |> last()
        '''
        tables = _query_api.query(query)
        
        if tables and tables[0].records:
            record = tables[0].records[0]
            point = Point("commands") \
                .tag("command_id", cmd_id) \
                .tag("sensor_id", str(record.values.get("sensor_id", "unknown"))) \
                .tag("type", record.values.get("type", "unknown")) \
                .field("value", record.values.get("value", "")) \
                .field("status", status) \
                .time(int(time.time()), write_precision='s')
            
            _write_api.write(bucket=INFLUXDB_BUCKET, record=point)
            return True
        return False
    except Exception as e:
        print(f"[Database] ✗ ERRO ao atualizar comando: {e}")
        return False

def update_config(sensor_id, interval, threshold):
    """Atualiza configuração do sensor (upsert via novo ponto)."""
    if _write_api is None:
        print("[Database] ✗ ERRO: InfluxDB não inicializado.")
        return False
    
    try:
        point = Point("sensor_configs") \
            .tag("sensor_id", str(sensor_id)) \
            .field("interval", int(interval)) \
            .field("threshold", float(threshold)) \
            .time(int(time.time()), write_precision='s')
        
        _write_api.write(bucket=INFLUXDB_BUCKET, record=point)
        return True
    except Exception as e:
        print(f"[Database] ✗ ERRO ao salvar config: {e}")
        return False

# ============================================================================
# Funções de consulta (para Flask/API)
# ============================================================================

def get_last_sensor_data(sensor_id, gateway_id=None):
    """Retorna últimos dados de um sensor."""
    if _query_api is None:
        return None
    
    try:
        query = f'''
        from(bucket: "{INFLUXDB_BUCKET}")
          |> range(start: -24h)
          |> filter(fn: (r) => r._measurement == "sensor_data")
          |> filter(fn: (r) => r.sensor_id == "{sensor_id}")
        '''
        if gateway_id:
            query += f'  |> filter(fn: (r) => r.gateway_id == "{gateway_id}")\n'
        
        # ✅ CORREÇÃO: last() ANTES do pivot
        query += '''
          |> last()
          |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
        '''
        
        tables = _query_api.query(query)
        if tables and tables[0].records:
            record = tables[0].records[0]
            return {
                'sensor_id': record.values.get('sensor_id'),
                'gateway_id': record.values.get('gateway_id'),
                'humidity': record.values.get('humidity'),
                'pollution': record.values.get('pollution'),
                'rssi': record.values.get('rssi'),
                'snr': record.values.get('snr'),
                'ts': int(record.get_time().timestamp())
            }
        return None
    except Exception as e:
        print(f"[Database] ✗ ERRO ao consultar sensor: {e}")
        return None
    
def get_sensor_config(sensor_id):
    """Retorna configuração mais recente de um sensor."""
    if _query_api is None:
        return None
    
    try:
        query = f'''
        from(bucket: "{INFLUXDB_BUCKET}")
          |> range(start: -30d)
          |> filter(fn: (r) => r._measurement == "sensor_configs")
          |> filter(fn: (r) => r.sensor_id == "{sensor_id}")
          |> last()
        '''
        tables = _query_api.query(query)
        if tables and tables[0].records:
            record = tables[0].records[0]
            return {
                'sensor_id': record.values.get('sensor_id'),
                'interval': record.values.get('interval'),
                'threshold': record.values.get('threshold'),
                'ts': int(record.get_time().timestamp())
            }
        return None
    except Exception as e:
        print(f"[Database] ✗ ERRO ao consultar config: {e}")
        return None

def get_recent_commands(limit=10):
    """Retorna últimos comandos para exibição no admin."""
    if _query_api is None:
        return []
    
    try:
        query = f'''
        from(bucket: "{INFLUXDB_BUCKET}")
          |> range(start: -24h)
          |> filter(fn: (r) => r._measurement == "commands")
          |> filter(fn: (r) => r._field == "status")
          |> sort(columns: ["_time"], desc: true)
          |> limit(n: {limit})
          |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
        '''
        tables = _query_api.query(query)
        results = []
        for table in tables:
            for record in table.records:
                results.append({
                    'command_id': record.values.get('command_id'),
                    'sensor_id': record.values.get('sensor_id'),
                    'type': record.values.get('type'),
                    'value': record.values.get('value'),
                    'status': record.values.get('status'),
                    'ts': int(record.get_time().timestamp())
                })
        return results
    except Exception as e:
        print(f"[Database] ✗ ERRO ao consultar comandos: {e}")
        return []

def get_distinct_sensors():
    """Retorna lista de IDs de sensores únicos."""
    if _query_api is None:
        return []
    
    try:
        query = f'''
        from(bucket: "{INFLUXDB_BUCKET}")
          |> range(start: -7d)
          |> filter(fn: (r) => r._measurement == "sensor_data")
          |> keep(columns: ["sensor_id"])
          |> group(columns: ["sensor_id"])
          |> distinct(column: "sensor_id")
        '''
        tables = _query_api.query(query)
        return [str(record.values.get('sensor_id')) for table in tables for record in table.records]
    except Exception as e:
        print(f"[Database] ✗ ERRO ao listar sensores: {e}")
        return []

def close_db():
    """Fecha conexões do InfluxDB (chamar no shutdown)."""
    global _client, _write_api, _query_api
    with _db_lock:
        if _write_api:
            _write_api.close()
        if _client:
            _client.close()
        _write_api = _query_api = _client = None