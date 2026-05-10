import sqlite3


DATABASE_NAME = "access_log.db"


def view_logs():
    conn = sqlite3.connect(DATABASE_NAME)
    cursor = conn.cursor()

    cursor.execute("""
        SELECT id, timestamp, uid, user_name, result, fail_count
        FROM access_logs
        ORDER BY id DESC
    """)

    rows = cursor.fetchall()
    conn.close()

    if not rows:
        print("No access logs found.")
        return

    print("ID | Timestamp           | UID         | User         | Result  | Fail Count")
    print("-" * 85)

    for row in rows:
        log_id, timestamp, uid, user_name, result, fail_count = row
        print(f"{log_id:<2} | {timestamp:<19} | {uid:<11} | {user_name:<12} | {result:<7} | {fail_count}")


if __name__ == "__main__":
    view_logs()