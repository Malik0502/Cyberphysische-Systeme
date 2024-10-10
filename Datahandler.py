import serial
import time
import json
import os
from flask import Flask, jsonify, request
from datetime import datetime
import sqlite3

app = Flask(__name__)

# Klasse zur Verwaltung des Zustands
class CrossroadState:
    def __init__(self):
        self.last_crossroad = None  # Speichert den letzten Kreuzungswert

    def update(self, new_crossroad):
        """Aktualisiert den Kreuzungswert, wenn sich der Wert geändert hat."""
        if new_crossroad != self.last_crossroad:
            self.last_crossroad = new_crossroad
            return True  # Signalisiert, dass der Wert geändert wurde
        return False  # Signalisiert, dass der Wert unverändert ist

crossroad_state = CrossroadState()  # Instanz der Klasse für den Zustand

def init_db():
    sql_statement = [
        """CREATE TABLE IF NOT EXISTS traffictable2 (
            id INTEGER PRIMARY KEY,
            ConvertedTime TEXT,
            Crossroad INT,
            TimeConvertable INTEGER
        );"""
    ]
   
    try:
        with sqlite3.connect('trafficdatabase3.db') as conn:
            cursor = conn.cursor()
            for statement in sql_statement:
                cursor.execute(statement)
            conn.commit()
    except sqlite3.Error as e:
        print(e)

@app.route('/data', methods=['GET'])
def get_data():
    conn = sqlite3.connect('trafficdatabase3.db')
    c = conn.cursor()
    c.execute("SELECT * FROM traffictable2")
    rows = c.fetchall()
    conn.close()
    return jsonify(rows)

def insert_data(converted_time, crossroad, time_convertable):
    conn = sqlite3.connect('trafficdatabase3.db')
    c = conn.cursor()
    c.execute("INSERT INTO traffictable2 (ConvertedTime, Crossroad, TimeConvertable) VALUES (?, ?, ?)",
              (converted_time, crossroad, time_convertable))
    conn.commit()
    conn.close()

if __name__ == '__main__':
    init_db()
    print("Server gestartet")
   
    # Seriellen Port konfigurieren
    ser = serial.Serial('/dev/ttyACM0', 9600)
    time.sleep(2)  # Warten, bis die Verbindung hergestellt ist

    while True:
        try:
            if ser.in_waiting > 0:
                # Lese eine Zeile von der seriellen Schnittstelle
                line = ser.readline().decode('utf-8').rstrip()
                print(f"Empfangene Daten: {line}")

                # Prüfen, ob die empfangene Zeile ein JSON-String ist
                if line.startswith('{') and line.endswith('}'):
                    try:
                        # Versuche, den JSON-String zu deserialisieren
                        data = json.loads(line)

                        # Extrahiere die Daten aus dem JSON-Objekt
                        crossroad = data.get("kreuzung", None)
                        current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        time_convertable = int(time.time())

                        # Überprüfen, ob die Kreuzung nicht 10 ist und sich geändert hat
                        if crossroad is not None and crossroad != 10:
                            if crossroad_state.update(crossroad):  # Nur wenn sich der Wert ändert
                                # Speichere die Daten in die SQLite-Datenbank
                                insert_data(current_time, crossroad, time_convertable)
                                print(f"Daten eingefügt: Kreuzung {crossroad}, Zeit {current_time}")
                            else:
                                print(f"Kreuzung {crossroad} bereits gespeichert, ignoriere.")
                        else:
                            print(f"JSON mit Kreuzung 10 oder ungültig ignoriert: {data}")

                    except json.JSONDecodeError as e:
                        print(f"Fehler beim Parsen des JSON-Strings: {e}")
                else:
                    # Wenn die empfangene Zeile kein JSON-String ist, ignoriere sie
                    print("Keine JSON-Daten, ignoriere diese Zeile.")

        except serial.SerialException as e:
            print(f"Fehler bei der seriellen Kommunikation: {e}")
            break