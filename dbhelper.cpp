#include "dbhelper.h"

#include <QFileInfo>
#include <QSqlError>

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
