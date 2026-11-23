#include "mainwindow.h"
#include "dbhelper.h"
#include "uistyle.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 统一浅色界面（Qt6 默认会跟随系统深色模式变黑，报告截图不好看）
    applyLightTheme();

    // 载入 Qt 自带的中文翻译。
    // 不载的话，QMessageBox 的按钮会显示成 Yes / No，中文界面里很突兀。
    QTranslator qtTranslator;
    if (qtTranslator.load("qtbase_zh_CN",
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        a.installTranslator(&qtTranslator);
    }

    // ------------------------------------------------------------
    //  第 1 件事：连数据库。连不上就别往下走了。
    //  这是"数据库连接失败应有明确提示"最实在的做法：
    //  不是在每个查询里各自报错，而是在程序入口一次性判断。
    //
    //  找库的顺序，是为了让工程"换一台机器也能跑"：
    //    ① 环境变量 HOSPITAL_DB —— 演示连接失败时，故意指一个不存在的路径
    //    ② 从程序所在目录逐级向上找 db/hospital.db 和 HospitalDB/hospital.db
    //       （因为 Qt Creator 的构建目录一般比工程目录深两三层）
    //    ③ 实在找不到，就在程序目录新建一个 —— 别人 clone 下来直接能跑
    //  注意 ①② 找不到会如实报错，只有 ③ 才会自动建库，
    //  否则"连接失败"这个测试场景就没法演示了。
    // ------------------------------------------------------------
    QString dbPath;
    bool needCreate = false;

    QString envPath = qEnvironmentVariable("HOSPITAL_DB");
    if (!envPath.isEmpty()) {
        dbPath = envPath;
    } else {
        QDir dir(QCoreApplication::applicationDirPath());
        for (int level = 0; level < 5; ++level) {
            // 每上一级都看两个位置：
            //   本级的 db/hospital.db      —— 数据库跟着工程走的情况
            //   本级的 HospitalDB/hospital.db —— 数据库单独放一个目录的情况
            QString inProject = QDir::cleanPath(dir.absoluteFilePath("db/hospital.db"));
            if (QFileInfo::exists(inProject)) { dbPath = inProject; break; }

            QString sibling = QDir::cleanPath(
                dir.absoluteFilePath("HospitalDB/hospital.db"));
            if (QFileInfo::exists(sibling)) { dbPath = sibling; break; }

            if (!dir.cdUp()) break;
        }
        if (dbPath.isEmpty()) {
            dbPath = QDir(QCoreApplication::applicationDirPath())
                         .absoluteFilePath("hospital.db");
            needCreate = true;
        }
    }

    bool dbReady = needCreate ? DbHelper::instance().createDatabase(dbPath)
                              : DbHelper::instance().open(dbPath);

    if (!dbReady) {
        QMessageBox::critical(nullptr, "数据库连接失败",
            QString("无法打开数据库：\n%1\n\n错误信息：%2\n\n"
                    "请检查文件是否存在、路径是否正确。\n"
                    "也可以设置环境变量 HOSPITAL_DB 指定数据库文件位置。")
                .arg(dbPath, DbHelper::instance().lastError()));
        return 1;   // 直接退出，避免带着一个坏连接继续跑
    }

    // ------------------------------------------------------------
    //  第 2 件事：开窗口
    // ------------------------------------------------------------
    MainWindow w;
    w.show();

    return a.exec();
}
