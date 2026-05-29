from flask import Flask, render_template, request, redirect, url_for, session, jsonify
import sqlite3
import os
try:
    from border_mqtt import BorderMQTT
    import database as db
except ImportError:
    from border.border_mqtt import BorderMQTT
    import border.database as db

import os

template_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
static_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'static'))
app = Flask(__name__, template_folder=template_dir, static_folder=static_dir)
app.secret_key = 'airsense_secret_key'

# Inicializa o Border MQTT (compartilhado com a API)
border_service = BorderMQTT()
border_service.run()

def get_db_connection():
    conn = sqlite3.connect('airsense.db')
    conn.row_factory = sqlite3.Row
    return conn

@app.route('/')
def index():
    if 'username' in session:
        if session['role'] == 'admin':
            return redirect(url_for('admin'))
        return redirect(url_for('dashboard'))
    return redirect(url_for('login'))

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        role = request.form['role']
        
        conn = get_db_connection()
        user = conn.execute('SELECT * FROM users WHERE username = ? AND password_hash = ? AND role = ?',
                           (username, password, role)).fetchone()
        conn.close()
        
        if user:
            session['username'] = username
            session['role'] = role
            if role == 'admin':
                return redirect(url_for('admin'))
            return redirect(url_for('dashboard'))
        else:
            return render_template('login.html', error='Credenciais inválidas')
            
    return render_template('login.html')

@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('login'))

@app.route('/admin')
def admin():
    if session.get('role') != 'admin':
        return redirect(url_for('login'))
    
    conn = get_db_connection()
    commands = conn.execute('SELECT * FROM commands ORDER BY ts DESC LIMIT 10').fetchall()
    sensors = conn.execute('SELECT DISTINCT sensor_id FROM sensor_data').fetchall()
    conn.close()
    
    return render_template('admin.html', commands=commands, sensors=sensors)

@app.route('/dashboard')
def dashboard():
    if 'username' not in session:
        return redirect(url_for('login'))
    
    return render_template('dashboard.html')

# API Endpoints
@app.route('/api/data')
def api_data():
    sensor_id = request.args.get('sensor_id', 1)
    conn = get_db_connection()
    data = conn.execute('SELECT * FROM sensor_data WHERE sensor_id = ? ORDER BY ts DESC LIMIT 1', (sensor_id,)).fetchone()
    conn.close()
    
    if data:
        return jsonify(dict(data))
    return jsonify({"error": "No data found"}), 404

@app.route('/api/command', methods=['POST'])
def api_command():
    if session.get('role') != 'admin':
        return jsonify({"error": "Unauthorized"}), 403
    
    data = request.json
    sensor_id = int(data.get('sensor_id'))
    interval = int(data.get('interval'))
    threshold = int(data.get('threshold'))
    
    # Validação (PRD 8.3-07)
    if not (10 <= interval <= 3600):
        return jsonify({"error": "Intervalo inválido"}), 400
    
    # Mudar o tempo entre as requisicoes
    border_service.set_timeStamp(interval)  

    sleep_time = max(1, interval//60)
    
    # Envia comando via MQTT (gateway fixo para demo: gw-001)
    cmd_id = border_service.send_command("gw-001", 1, 0x01, sleep_time)
    
    # Atualiza cache local
    db.update_config(sensor_id, interval, threshold)
    
    return jsonify({"status": "queued", "command_id": cmd_id})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
