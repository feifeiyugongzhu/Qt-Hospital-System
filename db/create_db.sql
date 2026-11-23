-- ============================================================
--  实验三：医院诊疗系统  数据库建表脚本（SQLite）
--  姓名：许泽晓    学号：2024414290438    班级：24软件工程4班
--  用法：在 Navicat / DB Browser for SQLite 里新建一个 SQLite 连接
--        指向 hospital.db，然后执行本文件全部内容即可。
-- ============================================================

-- 一、用户表：登录用
CREATE TABLE IF NOT EXISTS users (
    id       INTEGER PRIMARY KEY AUTOINCREMENT,   -- 主键，自增
    username TEXT    NOT NULL UNIQUE,             -- 用户名，不允许重复
    password TEXT    NOT NULL,                    -- 密码
    role     TEXT    NOT NULL DEFAULT 'admin'     -- 角色：admin 管理员 / doctor 医生
);

-- 二、患者表：主业务表
CREATE TABLE IF NOT EXISTS patients (
    id        INTEGER PRIMARY KEY AUTOINCREMENT,  -- 记录编号
    name      TEXT    NOT NULL,                   -- 姓名，必填
    gender    TEXT    NOT NULL DEFAULT '男',      -- 性别
    age       INTEGER NOT NULL,                   -- 年龄
    phone     TEXT    UNIQUE,                     -- 联系电话，唯一（用来演示重复数据异常）
    dept      TEXT,                               -- 就诊科室
    diagnosis TEXT,                               -- 诊断结果
    reg_date  TEXT    NOT NULL DEFAULT (date('now','localtime'))  -- 挂号日期
);

-- 三、操作日志表（选做项：记录增删改操作）
CREATE TABLE IF NOT EXISTS logs (
    id       INTEGER PRIMARY KEY AUTOINCREMENT,
    op_type  TEXT NOT NULL,      -- INSERT / UPDATE / DELETE / LOGIN
    op_desc  TEXT,               -- 操作描述
    op_time  TEXT NOT NULL DEFAULT (datetime('now','localtime'))
);

-- ============================================================
--  初始数据
-- ============================================================
INSERT INTO users (username, password, role) VALUES
    ('admin',  '123456', 'admin'),
    ('doctor', '888888', 'doctor');

INSERT INTO patients (name, gender, age, phone, dept, diagnosis, reg_date) VALUES
    ('张伟明', '男', 45, '13800138001', '内科',   '上呼吸道感染', '2026-11-20'),
    ('李秀兰', '女', 62, '13800138002', '内科',   '高血压二级',   '2026-11-20'),
    ('王建国', '男', 38, '13800138003', '外科',   '急性阑尾炎',   '2026-11-21'),
    ('陈晓芳', '女', 29, '13800138004', '妇产科', '早期妊娠',     '2026-11-21'),
    ('刘志强', '男', 51, '13800138005', '骨科',   '腰椎间盘突出', '2026-11-21'),
    ('赵敏',   '女', 34, '13800138006', '皮肤科', '接触性皮炎',   '2026-11-22'),
    ('孙浩然', '男', 8,  '13800138007', '儿科',   '支气管炎',     '2026-11-22'),
    ('周雅琴', '女', 67, '13800138008', '内科',   '2型糖尿病',    '2026-11-22'),
    ('吴国栋', '男', 43, '13800138009', '外科',   '腹股沟疝',     '2026-11-23'),
    ('郑丽娟', '女', 26, '13800138010', '眼科',   '细菌性结膜炎', '2026-11-23');
