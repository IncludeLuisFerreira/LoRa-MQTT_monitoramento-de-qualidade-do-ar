import sqlite3
import hashlib

AUTH_DB_PATH = "auth.db"

def init_auth_db():
    """
    Cria a tabela de usuários e insere um admin padrão se não existir.
    Banco: SQLite (auth.db) - separado do InfluxDB.
    """
    conn = sqlite3.connect(AUTH_DB_PATH)
    cursor = conn.cursor()
    
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT 'user',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    
    # Cria usuário admin padrão se não existir
    cursor.execute('SELECT id FROM users WHERE username = ?', ('admin',))
    if not cursor.fetchone():
        # Hash SHA-256 da senha padrão
        default_pass = hashlib.sha256('admin123'.encode()).hexdigest()
        cursor.execute(
            'INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?)',
            ('admin', default_pass, 'admin')
        )
        print("[Auth] ✓ Usuário admin padrão criado: admin / admin123")

    cursor.execute('SELECT id from users WHERE username = ?', ('user',))
    if not cursor.fetchone():
        default_user_pass = hashlib.sha256('user123'.encode()).hexdigest()
        cursor.execute(
            'INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?)',
            ('user',default_user_pass, 'user')
        )
    
    conn.commit()
    conn.close()
    print("[Auth] ✓ Banco de autenticação (SQLite) inicializado.")

def verify_user(username, password, role):
    """
    Verifica credenciais do usuário.
    Retorna dict com dados do usuário ou None se inválido.
    """
    password_hash = hashlib.sha256(password.encode()).hexdigest()
    
    conn = sqlite3.connect(AUTH_DB_PATH)
    conn.row_factory = sqlite3.Row
    cursor = conn.cursor()
    
    cursor.execute(
        'SELECT * FROM users WHERE username = ? AND password_hash = ? AND role = ?',
        (username, password_hash, role)
    )
    user = cursor.fetchone()
    conn.close()
    
    if user:
        return dict(user)
    return None