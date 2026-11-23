#include "logpage.h"
#include "dbhelper.h"

#include <QSqlTableModel>
#include <QSqlError>
#include <QTableView>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAbstractItemView>

LogPage::LogPage(QWidget *parent)
    : QWidget(parent)
    , m_model(nullptr)
    , m_table(nullptr)
    , m_labelCount(nullptr)
{
    m_labelCount = new QLabel(this);
    m_labelCount->setStyleSheet("color:#666666;");

    QPushButton *btnRefresh = new QPushButton("刷新日志", this);
    btnRefresh->setMinimumHeight(30);
    btnRefresh->setMinimumWidth(90);
    connect(btnRefresh, &QPushButton::clicked, this, &LogPage::refresh);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(new QLabel("记录每一次增、删、改操作，用于事后追溯：", this));
    topLayout->addStretch(1);
    topLayout->addWidget(m_labelCount);
    topLayout->addWidget(btnRefresh);

    m_table = new QTableView(this);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 日志只读
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setStyleSheet(
        "QTableView{gridline-color:#e3e8ee;}"
        "QHeaderView::section{background:#eef2f7;color:#33475b;border:none;"
        "border-bottom:1px solid #d0d8e0;padding:6px;font-weight:bold;}");

    // 同一个 QSqlTableModel，换个表名就是另一份数据 —— 这就是 Model 的好处
    m_model = new QSqlTableModel(this, DbHelper::instance().db());
    m_model->setTable("logs");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->setHeaderData(0, Qt::Horizontal, "序号");
    m_model->setHeaderData(1, Qt::Horizontal, "操作类型");
    m_model->setHeaderData(2, Qt::Horizontal, "操作说明");
    m_model->setHeaderData(3, Qt::Horizontal, "操作时间");
    m_model->setSort(0, Qt::DescendingOrder);      // 最新的排最前面
    m_model->select();
    m_table->setModel(m_model);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(10, 10, 10, 10);
    root->addLayout(topLayout);
    root->addWidget(m_table, 1);

    refresh();
}

void LogPage::refresh()
{
    if (!m_model)
        return;

    m_model->select();
    m_labelCount->setText(QString("共 %1 条操作记录").arg(m_model->rowCount()));
}
