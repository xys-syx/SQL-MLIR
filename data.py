import sqlite3, random, sys, time, os, itertools

DB_FILE   = sys.argv[1] if len(sys.argv) > 1 else "test.db"
BATCH     = 50_000
TARGET_S  = 5.0

def main():

    need_init = not os.path.exists(DB_FILE)
    conn = sqlite3.connect(DB_FILE)
    cur  = conn.cursor()

    if need_init:
        cur.execute("CREATE TABLE IF NOT EXISTS DATA(a INT, b INT)")
    else:
        cur.execute("DELETE FROM DATA")

    conn.execute("PRAGMA synchronous = OFF")
    conn.execute("PRAGMA journal_mode = MEMORY")

    # 4) insert loop
    total = 0
    start = time.time()
    rng   = random.randint
    while time.time() - start < TARGET_S:
        rows = [(rng(0, 1_000_000), rng(0, 1_000_000)) for _ in range(BATCH)]
        cur.executemany("INSERT INTO DATA VALUES (?,?)", rows)
        total += BATCH
        if total % 1_000_000 == 0:
            conn.commit()

    conn.commit()
    elapsed = time.time() - start

    print(f"Inserted {total:,} rows in {elapsed:.2f} s "
          f"→ {(total / elapsed):,.0f} rows/sec")

if __name__ == "__main__":
    main()