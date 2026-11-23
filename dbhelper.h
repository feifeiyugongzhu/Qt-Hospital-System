#ifndef DBHELPER_H
#define DBHELPER_H

#include <QSqlDatabase>
#include <QString>

// ============================================================
//  DbHelper —— 数据库连接管理器（单例模式）
//
//  为什么要单例？
//  Qt 里每 new 一个 QSqlDatabase 连接都可能开出新的物理连接，
//  多个页面各自开连接，很容易出现「一个页面改了数据，另一个页面看不见」。
//  所以整个程序只保留一个连接对象，谁要用就来 instance() 要。
//
//  单例的两条规矩：
//    1. 构造函数私有 —— 外面不能 new DbHelper()
//    2. 禁止拷贝     —— 防止 DbHelper a = DbHelper::instance(); 复制出第二份
// ============================================================
class DbHelper
{
public:
    static DbHelper &instance();          // 取全局唯一实例

    bool open(const QString &dbPath);     // 打开数据库，成功返回 true
    QSqlDatabase db() const;              // 取出连接，给 QSqlQuery / QSqlTableModel 用
    QString lastError() const;            // 上次失败的原因（用来弹提示框）

    // 建库：把一个不存在的路径变成带表结构和初始数据的完整数据库。
    // 有了它，别人把仓库 clone 下来直接就能跑，不用先手动建库。
    bool createDatabase(const QString &dbPath);

private:
    DbHelper() {}                                  // 私有构造
    DbHelper(const DbHelper &) = delete;           // 禁止拷贝
    DbHelper &operator=(const DbHelper &) = delete;

    QSqlDatabase m_db;      // 唯一的那个连接
    QString m_lastError;    // 唯一的那个错误信息
};

#endif // DBHELPER_H
