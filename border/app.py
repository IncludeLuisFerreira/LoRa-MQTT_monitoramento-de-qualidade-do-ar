from api.routes import app
import database

if __name__ == '__main__':
    database.init_db()
    app.run(host='0.0.0.0', port=5000)
