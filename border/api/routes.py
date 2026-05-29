# api/routes.py
from flask import Flask, render_template, request, redirect, url_for, session, jsonify
import sqlite3
import hashlib
import os

try:
    from border_mqtt import BorderMQTT
    import database as db
except ImportError:
    from border.border_mqtt import BorderMQTT
    import border.database as db

# Configurações do Flask
template_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'templates'))
static_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'static'))

app = Flask(__name__, template_folder=template_dir, static_folder=static_dir)
app.secret_key = 'airsense_secret_key'

# BorderMQTT será inicializado pelo app.py (não aqui)
border_service = None

def set_border_service(service):
    """Injeta a instância do BorderMQTT (chamado pelo app.py)"""
    global border_service
    border_service = service

def get_auth_connection():
    """Conexão para o banco de autenticação (SQLite)"""
    conn = sqlite3.connect('auth.db')
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

        password_hash = hashlib.sha256(password.encode()).hexdigest()
        
        conn = get_auth_connection()
        user = conn.execute(
            'SELECT * FROM users WHERE username = ? AND password_hash = ? AND role = ?',
            (username, password_hash, role)
        ).fetchone()
        conn.close()
        
        if user:
            session['username'] = username
            session['role'] = role
            return redirect(url_for('admin' if role == 'admin' else 'dashboard'))
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
    
    commands = db.get_recent_commands(limit=10)
    sensors = db.get_distinct_sensors()
    
    return render_template('admin.html', commands=commands, sensors=sensors)

@app.route('/dashboard')
def dashboard():
    if 'username' not in session:
        return redirect(url_for('login'))
    
    return render_template('dashboard.html')

@app.route('/api/data')
def api_data():
    sensor_id = request.args.get('sensor_id', 1)
    data = db.get_last_sensor_data(sensor_id)
    
    if data:
        return jsonify(data)
    return jsonify({"error": "No data found"}), 404

@app.route('/api/command', methods=['POST'])
def api_command():
    if session.get('role') != 'admin':
        return jsonify({"error": "Unauthorized"}), 403
    
    data = request.json
    sensor_id = int(data.get('sensor_id', 1))
    interval = int(data.get('interval'))
    threshold = float(data.get('threshold', 0))
    
    if not (10 <= interval <= 3600):
        return jsonify({"error": "Intervalo inválido (10-3600s)"}), 400
    
    if border_service:
        border_service.set_timeStamp(interval)  

    sleep_time = max(1, interval // 60)
    
    if border_service:
        cmd_id = border_service.send_command("gw-001", sensor_id, 0x01, sleep_time)
    else:
        cmd_id = "mock-cmd"
    
    db.update_config(sensor_id, interval, threshold)
    
    return jsonify({"status": "queued", "command_id": cmd_id})