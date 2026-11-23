#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class QLabel;
class QTabWidget;
class LoginPage;
class PatientPage;
class LogPage;

// ============================================================
//  MainWindow —— 主窗口，起两个作用：
//    1. 当"容器"：用 QStackedWidget 把登录页和主界面叠在一起，一次只显示一个
//    2. 当"调度员"：接收各页面发出的信号，决定切到哪一页（Controller 的角色）
//
//  主界面里再用 QTabWidget 分成两个标签页：患者信息 / 操作日志。
// ============================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoginSucceeded(const QString &username, const QString &role);
    void onLogout();

private:
    QWidget *buildHomePage();   // 搭登录成功后的主界面

    QStackedWidget *m_stack;        // 页面容器：0=登录页  1=主界面
    LoginPage *m_loginPage;         // 登录页
    QWidget *m_homePage;            // 主界面
    QLabel *m_labelCurrentUser;     // 顶部显示"当前用户"
    QTabWidget *m_tabs;             // 主界面里的标签页
    PatientPage *m_patientPage;     // 患者信息管理
    LogPage *m_logPage;             // 操作日志
};

#endif // MAINWINDOW_H
