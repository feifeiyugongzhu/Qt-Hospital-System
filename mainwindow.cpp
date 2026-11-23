#include "mainwindow.h"
#include "loginpage.h"
#include "patientpage.h"
#include "logpage.h"

#include <QStackedWidget>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QFont>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_stack(nullptr)
    , m_loginPage(nullptr)
    , m_homePage(nullptr)
    , m_labelCurrentUser(nullptr)
    , m_tabs(nullptr)
    , m_patientPage(nullptr)
    , m_logPage(nullptr)
{
    setWindowTitle("医院诊疗系统 —— 许泽晓 2024414290438");
    resize(1000, 640);

    // ---- 容器：QStackedWidget 可以塞很多页，一次只显示 index 指定的那一页 ----
    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // ---- 第 0 页：登录页 ----
    m_loginPage = new LoginPage(this);
    m_stack->addWidget(m_loginPage);        // index = 0

    // ---- 第 1 页：主界面 ----
    m_homePage = buildHomePage();
    m_stack->addWidget(m_homePage);         // index = 1

    m_stack->setCurrentIndex(0);            // 默认显示登录页

    // ---- 页面之间靠信号通信：登录成功 → 切到主界面 ----
    connect(m_loginPage, &LoginPage::loginSucceeded,
            this, &MainWindow::onLoginSucceeded);

    // ---- 状态栏常驻学号姓名（实验要求"界面上要有开发者学号姓名"） ----
    statusBar()->showMessage("开发：许泽晓    学号：2024414290438    班级：24软件工程4班");
}

MainWindow::~MainWindow()
{
    // 所有控件都挂在 this / m_stack 下面，Qt 的父子对象机制会自动释放，
    // 这里不需要手写 delete。
}

// 登录成功：切到主界面，显示当前用户，并把数据刷新一遍
void MainWindow::onLoginSucceeded(const QString &username, const QString &role)
{
    QString roleText = (role == "admin") ? "管理员" : "医生";
    m_labelCurrentUser->setText(QString("当前用户：%1（%2）").arg(username, roleText));

    // 每次登录都重新读一遍数据，避免显示的是上一次登录留下的旧内容
    m_patientPage->refresh();
    m_logPage->refresh();

    m_stack->setCurrentIndex(1);    // 0 → 1，界面切过去了
}

// 退出登录：切回登录页，并且把密码框清空
void MainWindow::onLogout()
{
    m_loginPage->reset();
    m_stack->setCurrentIndex(0);
}

// ============================================================
//  搭主界面：顶部一行（标题 + 当前用户 + 退出），下面是两个标签页
// ============================================================
QWidget *MainWindow::buildHomePage()
{
    QWidget *page = new QWidget(this);

    // ---- 顶部 ----
    QLabel *title = new QLabel("医院诊疗系统 · 患者信息管理", page);
    QFont f = title->font();
    f.setPointSize(15);
    f.setBold(true);
    title->setFont(f);

    m_labelCurrentUser = new QLabel("当前用户：", page);
    m_labelCurrentUser->setStyleSheet("color:#666666;");

    QPushButton *btnLogout = new QPushButton("退出登录", page);
    btnLogout->setFixedWidth(90);
    btnLogout->setMinimumHeight(28);
    connect(btnLogout, &QPushButton::clicked, this, &MainWindow::onLogout);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(title);
    topLayout->addSpacing(16);
    topLayout->addWidget(m_labelCurrentUser);
    topLayout->addStretch(1);
    topLayout->addWidget(btnLogout);

    // ---- 标签页 ----
    m_patientPage = new PatientPage(page);
    m_logPage     = new LogPage(page);

    m_tabs = new QTabWidget(page);
    m_tabs->addTab(m_patientPage, "患者信息");
    m_tabs->addTab(m_logPage,     "操作日志");
    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (index == 1)             // 切到日志页时自动刷新
            m_logPage->refresh();
    });

    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->addLayout(topLayout);
    layout->addWidget(m_tabs, 1);

    return page;
}
