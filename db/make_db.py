# -*- coding: utf-8 -*-
"""
把 create_db.sql 执行一遍，生成 / 重置 hospital.db。
以后想恢复初始数据，直接运行本脚本即可（不需要手动删文件）。

用法（在 D:\\QtProjects\\HospitalDB 目录下）：
    python make_db.py
"""

import os
import sqlite3

# ---- 1. 定好数据库文件的位置：就是本脚本所在目录下的 hospital.db ----
HERE = os.path.dirname(os.path.abspath(__file__))
DB_PATH = os.path.join(HERE, "hospital.db")
SQL_PATH = os.path.join(HERE, "create_db.sql")

# ---- 2. 连上数据库（文件不存在会自动创建） ----
conn = sqlite3.connect(DB_PATH)
cursor = conn.cursor()

# ---- 3. 先把旧表删掉，保证每次都是干净的初始数据 ----
#    为什么不直接删文件？因为文件可能被别的程序占用删不掉，
#    而 DROP TABLE 在连接内部执行，任何环境都能成功。
cursor.executescript("""
    DROP TABLE IF EXISTS users;
    DROP TABLE IF EXISTS patients;
    DROP TABLE IF EXISTS logs;
""")

# ---- 4. 读出建表脚本的内容（一定要按 utf-8 读，否则中文会乱码） ----
f = open(SQL_PATH, "r", encoding="utf-8")
sql_text = f.read()
f.close()

# ---- 5. 一次性执行整个脚本（建表 + 插入初始数据） ----
cursor.executescript(sql_text)
conn.commit()

# ---- 6. 验证一下：把表名和条数打出来 ----
print("数据库已生成：", DB_PATH)
print("--- 现有的表 ---")
cursor.execute("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name")
for row in cursor.fetchall():
    table_name = row[0]
    cursor2 = conn.cursor()
    cursor2.execute("SELECT COUNT(*) FROM " + table_name)
    count = cursor2.fetchone()[0]
    print("  %-16s 共 %d 条记录" % (table_name, count))

print("--- users 表内容 ---")
cursor.execute("SELECT id, username, password, role FROM users")
for row in cursor.fetchall():
    print("  ", row)

print("--- patients 表内容 ---")
cursor.execute("SELECT id, name, gender, age, phone, dept, diagnosis, reg_date FROM patients")
for row in cursor.fetchall():
    print("  ", row)

conn.close()
