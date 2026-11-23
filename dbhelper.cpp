#include "dbhelper.h"

#include <QFile>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

// 拿到全局唯一实例。
// 写法：函数内部的 static 变量，C++ 保证它在第一次调用时才创建，
// 并且整个程序运行期间只有一个 —— 比在全局搞个指针安全得多。
DbHelper &DbHelper::instance()
{
    static DbHelper obj;
    return obj;
}

bool DbHelper::open(const QString &dbPath)
{
    // 先清空上次的错误信息，避免误导
    m_lastError.clear();

    // ---- 第 1 步：检查文件在不在 ----
    // 这一步很有必要：SQLite 的特点是，你给它一个不存在的路径，它不报错，
    // 反而默默建一个空库出来。结果就是连接"成功"了，但一查表就报
    // "no such table: patients"，很多人会以为是代码写错，其实是连到了空库。
    QFileInfo info(dbPath);
    if (!info.exists()) {
        m_lastError = QString("数据库文件不存在：%1").arg(dbPath);
        return false;
    }

    // ---- 第 2 步：注册驱动并设置文件名 ----
    // 第一个参数 QSQLITE 是驱动名，必须和 Qt 插件里的名字完全一致（大写）。
    // 第二个参数是"连接名"，给了名字以后可以用 QSqlDatabase::database("...") 找回它。
    m_db = QSqlDatabase::addDatabase("QSQLITE", "hospital_conn");
    m_db.setDatabaseName(dbPath);

    // ---- 第 3 步：真正打开 ----
    // 返回 false 一般是两种情况：驱动没加载（CMakeLists 忘了链 Qt6::Sql），
    // 或者文件被别的程序独占、权限不够。
    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    return true;
}

QSqlDatabase DbHelper::db() const
{
    return m_db;
}

QString DbHelper::lastError() const
{
    return m_lastError;
}

// ============================================================
//  建库：把一段 SQL 脚本变成一个可用的数据库文件
//
//  为什么要这个函数？
//  数据库文件里是数据，不适合直接塞进 git 仓库（里面还有明文密码）。
//  所以仓库里只放建表脚本，程序第一次运行时自己把库建出来。
//  这样别人 clone 下来改都不用改，直接编译就能跑。
//
//  注意：QSqlQuery::exec() 一次只能执行一条 SQL，
//  所以这里要把脚本拆成单独的语句，循环执行。
// ============================================================
bool DbHelper::createDatabase(const QString &dbPath)
{
    m_lastError.clear();

    // ---- 第 1 步：先落一个空文件 ----
    // SQLite 的 open() 本身就爱"贴心"地自动建空库，
    // 但那样失败时看不出到底是哪一步出的问题。
    // 手动建文件，能把"文件建不出来"（没权限、目录不存在）
    // 和"库打不开"两种情况分开报错。
    QFile f(dbPath);
    if (!f.open(QIODevice::WriteOnly)) {
        m_lastError = QString("无法创建数据库文件：%1").arg(f.errorString());
        return false;
    }
    f.close();

    // ---- 第 2 步：走正常的打开流程 ----
    if (!open(dbPath)) {
        return false;      // open() 内部已经填好了 m_lastError
    }

    // ---- 第 3 步：建表 ----
    QStringList script;

    script << QStringLiteral(
        "CREATE TABLE IF NOT EXISTS users ("
        "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username TEXT NOT NULL UNIQUE,"
        "  password TEXT NOT NULL,"
        "  role     TEXT NOT NULL DEFAULT 'admin')");

    script << QStringLiteral(
        "CREATE TABLE IF NOT EXISTS patients ("
        "  id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name      TEXT NOT NULL,"
        "  gender    TEXT NOT NULL DEFAULT '男',"
        "  age       INTEGER NOT NULL,"
        "  phone     TEXT UNIQUE,"
        "  dept      TEXT,"
        "  diagnosis TEXT,"
        "  reg_date  TEXT NOT NULL DEFAULT (date('now','localtime')))");

    script << QStringLiteral(
        "CREATE TABLE IF NOT EXISTS logs ("
        "  id      INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  op_type TEXT NOT NULL,"
        "  op_desc TEXT,"
        "  op_time TEXT NOT NULL DEFAULT (datetime('now','localtime')))");

    // ---- 第 4 步：写入初始数据 ----
    script << QStringLiteral(
        "INSERT INTO users (username, password, role) VALUES "
        "('admin', '123456', 'admin'), "
        "('doctor', '888888', 'doctor')");

    script << QStringLiteral(
        "INSERT INTO patients (name, gender, age, phone, dept, diagnosis, reg_date) VALUES "
        "('张伟明', '男', 45, '13800138001', '内科',   '上呼吸道感染', '2026-11-20'), "
        "('李秀兰', '女', 62, '13800138002', '内科',   '高血压二级',   '2026-11-20'), "
        "('王建国', '男', 38, '13800138003', '外科',   '急性阑尾炎',   '2026-11-21'), "
        "('陈晓芳', '女', 29, '13800138004', '妇产科', '早期妊娠',     '2026-11-21'), "
        "('刘志强', '男', 51, '13800138005', '骨科',   '腰椎间盘突出', '2026-11-21'), "
        "('赵敏',   '女', 34, '13800138006', '皮肤科', '接触性皮炎',   '2026-11-22'), "
        "('孙浩然', '男',  8, '13800138007', '儿科',   '支气管炎',     '2026-11-22'), "
        "('周雅琴', '女', 67, '13800138008', '内科',   '2型糖尿病',    '2026-11-22'), "
        "('吴国栋', '男', 43, '13800138009', '外科',   '腹股沟疝',     '2026-11-23'), "
        "('郑丽娟', '女', 26, '13800138010', '眼科',   '细菌性结膜炎', '2026-11-23')");

    // ---- 第 5 步：逐条执行 ----
    for (const QString &sql : script) {
        QSqlQuery q(m_db);
        if (!q.exec(sql)) {
            m_lastError = QString("初始化数据库失败：%1\n出错的语句：%2")
                              .arg(q.lastError().text(), sql.left(50));
            return false;
        }
    }

    return true;
}
