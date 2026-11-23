#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;

// ============================================================
//  LoginPage —— 登录页（View + 一点点 Controller）
//
//  它自己负责：画界面、收输入、查数据库校验账号密码。
//  校验成功后不自己去切页面，而是 emit 一个信号交出去，
//  由 MainWindow 决定"登录成功后该显示什么"。
//  这就是 MVC 里 View 不直接操纵流程的好处。
// ============================================================
class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);

    void reset();          // 退回登录页时清空输入框

signals:
    // 登录成功，把用户名和角色送出去
    void loginSucceeded(const QString &username, const QString &role);

private slots:
    void onLoginClicked();

private:
    bool checkUser(const QString &user, const QString &pwd,
                   QString &role, QString &errMsg);
    void showMessage(const QString &text);

    QLineEdit *m_editUser;    // 用户名
    QLineEdit *m_editPwd;     // 密码
    QLabel *m_labelMsg;       // 下方的红色提示
    QPushButton *m_btnLogin;  // 登录按钮
    QPushButton *m_btnQuit;   // 退出按钮
};

#endif // LOGINPAGE_H
