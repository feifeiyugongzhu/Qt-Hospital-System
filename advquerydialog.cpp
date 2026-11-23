#include "advquerydialog.h"
#include "dbhelper.h"

#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlError>
#include <QHeaderView>
#include <QMessageBox>
#include <QAbstractItemView>

AdvQueryDialog::AdvQueryDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("通用查询 —— 用 QSqlQuery 自由组合条件");
    resize(780, 480);

    // 显示名和数据库列名一一对应，下标必须对齐
    m_fieldNames  << "姓名" << "性别" << "年龄" << "就诊科室" << "诊断结果" << "联系电话";
    m_columnNames << "name" << "gender" << "age"    << "dept"     << "diagnosis" << "phone";

    m_comboField = new QComboBox(this);
    m_comboField->addItems(m_fieldNames);

    m_comboOp = new QComboBox(this);
    m_comboOp->addItem("等于",        "=");
    m_comboOp->addItem("包含（模糊）", "LIKE");
    m_comboOp->addItem("大于",        ">");
    m_comboOp->addItem("小于",        "<");

    m_editValue = new QLineEdit(this);
    m_editValue->setPlaceholderText("输入要查的值，例如：内科 / 张 / 45");
    m_editValue->setMinimumHeight(30);

    QPushButton *btnExec = new QPushButton("执行查询", this);
    btnExec->setMinimumHeight(30);
    connect(btnExec, &QPushButton::clicked, this, &AdvQueryDialog::onExecute);
    connect(m_editValue, &QLineEdit::returnPressed, this, &AdvQueryDialog::onExecute);

    m_labelResult = new QLabel("尚未查询", this);
    m_labelResult->setStyleSheet("color:#666666;");

    QHBoxLayout *condLayout = new QHBoxLayout;
    condLayout->addWidget(new QLabel("查询条件：", this));
    condLayout->addWidget(m_comboField);
    condLayout->addWidget(m_comboOp);
    condLayout->addWidget(m_editValue, 1);
    condLayout->addWidget(btnExec);

    // ---- 结果表 ----
    m_qmodel = new QSqlQueryModel(this);

    m_table = new QTableView(this);
    m_table->setModel(m_qmodel);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 只读：查询结果不允许改
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setStyleSheet(
        "QTableView{gridline-color:#e3e8ee;}"
        "QHeaderView::section{background:#eef2f7;color:#33475b;border:none;"
        "border-bottom:1px solid #d0d8e0;padding:6px;font-weight:bold;}");

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(m_labelResult);
    bottomLayout->addStretch(1);
    QPushButton *btnClose = new QPushButton("关闭", this);
    btnClose->setMinimumHeight(30);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    bottomLayout->addWidget(btnClose);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->addLayout(condLayout);
    root->addWidget(m_table, 1);
    root->addLayout(bottomLayout);

    // 打开窗口时先查一次全部，别让表格空着
    m_comboField->setCurrentIndex(3);          // 默认按"就诊科室"
    m_editValue->setText("内科");
    onExecute();
}

void AdvQueryDialog::onExecute()
{
    int fieldIndex = m_comboField->currentIndex();
    QString column = m_columnNames.value(fieldIndex);
    QString op     = m_comboOp->currentData().toString();
    QString value  = m_editValue->text().trimmed();

    // ---- 拼 SQL：只有字段名和运算符来自下拉框（是程序自己给的固定字符串），----
    // ---- 用户输入的值一律走占位符绑定，所以拼不出注入 --------------------
    QString sql = QString(
        "SELECT id AS 编号, name AS 姓名, gender AS 性别, age AS 年龄, "
        "phone AS 联系电话, dept AS 就诊科室, diagnosis AS 诊断结果, reg_date AS 挂号日期 "
        "FROM patients WHERE %1 %2 ? ORDER BY id").arg(column, op);

    QSqlQuery query(DbHelper::instance().db());
    if (!query.prepare(sql)) {
        QMessageBox::warning(this, "SQL 准备失败", query.lastError().text());
        return;
    }

    // LIKE 的话要自己前后加 %，等于/大于/小于直接绑原值
    if (op == "LIKE")
        query.addBindValue("%" + value + "%");
    else
        query.addBindValue(value);

    if (!query.exec()) {
        // 比如"年龄 大于 三十"这种：SQLite 会拿字符串去比数字，
        // 这里如果是 SQL 层面的错误就在这里提示出来
        QMessageBox::warning(this, "查询失败",
            QString("SQL 执行出错：\n%1\n\n刚才的语句：\n%2")
                .arg(query.lastError().text(), query.lastQuery()));
        return;
    }

    m_qmodel->setQuery(std::move(query));

    // QSqlQueryModel 是"要用到才去取"的，先把它全取完才能拿到准确行数
    while (m_qmodel->canFetchMore())
        m_qmodel->fetchMore();

    int count = m_qmodel->rowCount();
    if (count == 0) {
        m_labelResult->setText("未找到匹配记录");
        m_labelResult->setStyleSheet("color:#cc0000;font-weight:bold;");
    } else {
        m_labelResult->setText(QString("共查到 %1 条记录").arg(count));
        m_labelResult->setStyleSheet("color:#2e7d32;font-weight:bold;");
    }
}
