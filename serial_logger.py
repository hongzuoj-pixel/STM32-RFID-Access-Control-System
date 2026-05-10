import serial
import sqlite3
from datetime import datetime


SERIAL_PORT = "COM5"
BAUD_RATE = 115200
DATABASE_NAME = "access_log.db"


def init_database():
    conn = sqlite3.connect(DATABASE_NAME)
    cursor = conn.cursor()

    cursor.execute("""
        CREATE TABLE IF NOT EXISTS access_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            uid TEXT NOT NULL,
            user_name TEXT NOT NULL,
            result TEXT NOT NULL,
            fail_count INTEGER NOT NULL
        )
    """)

    conn.commit()
    conn.close()


def insert_log(uid, user_name, result, fail_count):
    conn = sqlite3.connect(DATABASE_NAME)
    cursor = conn.cursor()

    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    cursor.execute("""
        INSERT INTO access_logs (timestamp, uid, user_name, result, fail_count)
        VALUES (?, ?, ?, ?, ?)
    """, (timestamp, uid, user_name, result, fail_count))

    conn.commit()
    conn.close()

    print(f"[DB] Saved: {timestamp}, {uid}, {user_name}, {result}, {fail_count}")


def parse_log_line(line):
    line = line.strip()

    if not line.startswith("LOG,"):
        return None

    parts = line.split(",")

    if len(parts) != 5:
        print(f"[WARN] Invalid LOG format: {line}")
        return None

    uid = parts[1].strip()
    user_name = parts[2].strip()
    result = parts[3].strip()

    try:
        fail_count = int(parts[4].strip())
    except ValueError:
        print(f"[WARN] Invalid fail count: {line}")
        return None

    return uid, user_name, result, fail_count


def main():
    init_database()

    print("================================")
    print(" STM32 RFID Serial Logger")
    print(" Listening on:", SERIAL_PORT)
    print(" Baud rate:", BAUD_RATE)
    print(" Database:", DATABASE_NAME)
    print("================================")
    print("Waiting for STM32 LOG data...\n")

    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    except serial.SerialException as e:
        print("[ERROR] Failed to open serial port.")
        print("Reason:", e)
        print("\nPlease check:")
        print("1. Serial monitor is closed")
        print("2. COM port is correct")
        print("3. USB-TTL is connected")
        return

    try:
        while True:
            raw_data = ser.readline()

            if not raw_data:
                continue

            line = raw_data.decode("utf-8", errors="ignore").strip()

            if line:
                print("[SERIAL]", line)

            parsed = parse_log_line(line)

            if parsed is not None:
                uid, user_name, result, fail_count = parsed
                insert_log(uid, user_name, result, fail_count)

    except KeyboardInterrupt:
        print("\n[INFO] Logger stopped by user.")

    finally:
        ser.close()
        print("[INFO] Serial port closed.")


if __name__ == "__main__":
    main()