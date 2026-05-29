import sqlite3
import time
import os
import threading

DB_PATH = "airsense.db"
_db_lock = threading.Lock()   # Lock para serializar escritas

def init_db():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    # ... (criação das tabelas, igual ao original) ...
    conn.commit()
    conn.close()

def save_sensor_data(data):
    with _db_lock:
        conn = sqlite3.connect(DB_PATH, timeout=5.0)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO sensor_data (sensor_id, gateway_id, humidity, pollution, rssi, snr, ts)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (data['src_id'], data['gw_id'], data['humidity'], data['pollution_level'], 
              data['rssi'], data['snr'], int(time.time())))
        conn.commit()
        conn.close()

def save_command(cmd_id, sensor_id, cmd_type, value):
    with _db_lock:
        conn = sqlite3.connect(DB_PATH, timeout=5.0)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT INTO commands (command_id, sensor_id, type, value, status, ts)
            VALUES (?, ?, ?, ?, ?, ?)
        ''', (cmd_id, sensor_id, cmd_type, value, 'pendente', int(time.time())))
        conn.commit()
        conn.close()

def update_command_status(cmd_id, status):
    with _db_lock:
        conn = sqlite3.connect(DB_PATH, timeout=5.0)
        cursor = conn.cursor()
        cursor.execute('UPDATE commands SET status = ? WHERE command_id = ?', (status, cmd_id))
        conn.commit()
        conn.close()

def update_config(sensor_id, interval, threshold):
    with _db_lock:
        conn = sqlite3.connect(DB_PATH, timeout=5.0)
        cursor = conn.cursor()
        cursor.execute('''
            INSERT OR REPLACE INTO configs (sensor_id, interval, threshold, last_updated)
            VALUES (?, ?, ?, ?)
        ''', (sensor_id, interval, threshold, int(time.time())))
        conn.commit()
        conn.close()