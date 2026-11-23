#include "mainwindow.h"
#include "dbhelper.h"
#include "uistyle.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>
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
    // ------------------------------------------------------------
    QString dbPath = "D:/QtProjects/HospitalDB/hospital.db";

    if (!DbHelper::instance().open(dbPath)) {
        QMessageBox::critical(nullptr, "数据库连接失败",
            QString("无法打开数据库：\n%1\n\n错误信息：%2\n\n"
                    "请检查文件是否存在、路径是否正确。")
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
