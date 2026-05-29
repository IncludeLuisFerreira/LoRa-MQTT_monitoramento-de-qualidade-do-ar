from api.routes import app, set_border_service
from auth import init_auth_db
import database as db

# Importa BorderMQTT com fallback para estrutura de pastas
try:
    from border_mqtt import BorderMQTT
except ImportError:
    from border.border_mqtt import BorderMQTT

if __name__ == '__main__':
    print("[App] Inicializando AirSense Monitor...")
    
    # 1. Inicializa InfluxDB (dados IoT)
    db.init_db()
    
    # 2. Inicializa banco de autenticação (SQLite)
    init_auth_db()
    
    # 3. Inicializa e inicia o BorderMQTT
    border_service = BorderMQTT()
    set_border_service(border_service)  # Injeta no Flask
    border_service.run()  # Inicia loop MQTT em thread
    
    print("[App] Serviços inicializados. Iniciando Flask...")
    
    # 4. Inicia servidor Flask
    app.run(host='0.0.0.0', port=5000, debug=True)