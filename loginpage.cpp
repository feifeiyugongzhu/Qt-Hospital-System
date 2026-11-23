#include "loginpage.h"
#include "dbhelper.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
{
    // ---------- 标题 ----------
    QLabel *title = new QLabel("医院诊疗系统", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignHCenter);

    QLabel *subtitle = new QLabel("用 户 登 录", this);
    subtitle->setAlignment(Qt::AlignHCenter);
    subtitle->setStyleSheet("color:#888888;");

    // ---------- 输入区 ----------
    // 用 QGridLayout 排成两行两列，比一行一行堆着好看
    m_editUser = new QLineEdit(this);
    m_editUser->setPlaceholderText("请输入用户名");
    m_editUser->setMinimumHeight(30);

    m_editPwd = new QLineEdit(this);
    m_editPwd->setPlaceholderText("请输入密码");
    m_editPwd->setMinimumHeight(30);
    m_editPwd->setEchoMode(QLineEdit::Password);   // 输入显示成圆点

    QGridLayout *grid = new QGridLayout;
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(12);
    grid->addWidget(new QLabel("用户名", this), 0, 0);
    grid->addWidget(m_editUser, 0, 1);
    grid->addWidget(new QLabel("密    码", this), 1, 0);
    grid->addWidget(m_editPwd, 1, 1);
    grid->setColumnStretch(1, 1);                  // 输入框那一列自动拉宽

    // ---------- 提示行 ----------
    m_labelMsg = new QLabel(this);
    m_labelMsg->setStyleSheet("color:#cc0000;");   // 红色，专门放错误信息
    m_labelMsg->setMinimumHeight(22);

    // ---------- 按钮行 ----------
    m_btnLogin = new QPushButton("登  录", this);
    m_btnLogin->setMinimumHeight(34);
    m_btnQuit = new QPushButton("退  出", this);
    m_btnQuit->setMinimumHeight(34);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addWidget(m_btnLogin);
    btnLayout->addWidget(m_btnQuit);

    // ---------- 底部：学号姓名（实验要求） ----------
    QLabel *sign = new QLabel("24软件工程4班   许泽晓   2024414290438", this);
    sign->setAlignment(Qt::AlignHCenter);
    sign->setStyleSheet("color:#999999; font-size:11px;");

    // ---------- 把表单装进一个最宽 420 的盒子 ----------
    // 不这么做的话，窗口拉宽以后输入框会变成一根横跨整个屏幕的长条，很难看
    QWidget *box = new QWidget(this);
    box->setFixedWidth(400);

    QVBoxLayout *boxLayout = new QVBoxLayout(box);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    boxLayout->addWidget(title);
    boxLayout->addWidget(subtitle);
    boxLayout->addSpacing(24);
    boxLayout->addLayout(grid);
    boxLayout->addWidget(m_labelMsg);
    boxLayout->addLayout(btnLayout);

    // ---------- 整体竖排，上下各塞一个弹簧把内容挤到中间 ----------
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(30, 30, 30, 20);
    root->addStretch(1);
    root->addWidget(box, 0, Qt::AlignHCenter);
    root->addStretch(1);
    root->addWidget(sign);

    // ---------- 信号连接 ----------
    connect(m_btnLogin, &QPushButton::clicked, this, &LoginPage::onLoginClicked);
    connect(m_btnQuit, &QPushButton::clicked, this, &QWidget::close);
    // 在密码框里按回车也能登录，符合用户习惯
    connect(m_editPwd, &QLineEdit::returnPressed, this, &LoginPage::onLoginClicked);
    connect(m_editUser, &QLineEdit::returnPressed, this, &LoginPage::onLoginClicked);
}

// 退回登录页时调用：清掉密码，清掉提示，光标放回用户名
void LoginPage::reset()
{
    m_editPwd->clear();
    m_labelMsg->clear();
    m_editUser->setFocus();
}

void LoginPage::showMessage(const QString &text)
{
    m_labelMsg->setText(text);
}

void LoginPage::onLoginClicked()
{
    QString user = m_editUser->text().trimmed();   // 用户名前后的空格不算数
    QString pwd  = m_editPwd->text();              // 密码不 trim，空格也算密码

    // 前端先挡一道：空的根本没必要求数据库
    if (user.isEmpty() || pwd.isEmpty()) {
        showMessage("用户名和密码都不能为空");
        return;
    }

    QString role;
    QString err;
    if (!checkUser(user, pwd, role, err)) {
        showMessage(err);
        m_editPwd->clear();
        m_editPwd->setFocus();
        return;
    }

    m_labelMsg->clear();
    emit loginSucceeded(user, role);   // 校验通过，通知外面去切页面
}

// ============================================================
//  核心：校验账号密码
//  返回 true 表示登录成功，同时通过 role 带回角色、通过 errMsg 带回失败原因
//
//  ⚠️ 这里绝对不能用字符串拼接 SQL。原因见下面第 2 种写法：
//     QString sql = "SELECT password FROM users WHERE username='" + user + "'";
//     如果 user 输入的是  admin' --   拼出来就变成
//       SELECT password FROM users WHERE username='admin' --'
//     后面的密码判断被 -- 注释掉了，直接就能登进去。
// ============================================================
bool LoginPage::checkUser(const QString &user, const QString &pwd,
                          QString &role, QString &errMsg)
{
    QSqlQuery query(DbHelper::instance().db());

    // prepare + bindValue：先用 ? 占位，再把值单独送进去。
    // 这样无论用户输入什么（哪怕带引号、带 -- ），都只会被当成一个普通字符串，
    // 不会变成 SQL 语句的一部分。
    query.prepare("SELECT password, role FROM users WHERE username = ?");
    query.addBindValue(user);

    if (!query.exec()) {
        errMsg = "查询用户失败：" + query.lastError().text();
        return false;
    }

    // 查不到任何行 → 说明这个用户名压根不存在
    if (!query.next()) {
        errMsg = "该用户名不存在";
        return false;
    }

    // 查到了，比对密码
    QString dbPwd  = query.value(0).toString();
    QString dbRole = query.value(1).toString();

    if (dbPwd != pwd) {
        errMsg = "密码错误";
        return false;
    }

    role = dbRole;
    return true;
}
