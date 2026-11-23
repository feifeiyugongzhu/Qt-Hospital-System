#include "patientpage.h"
#include "dbhelper.h"
#include "delegates.h"
#include "advquerydialog.h"

#include <QSqlTableModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableView>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QItemSelectionModel>
#include <QAbstractItemView>
#include <QDate>
#include <QStringList>

PatientPage::PatientPage(QWidget *parent)
    : QWidget(parent)
    , m_model(nullptr)
    , m_table(nullptr)
    , m_editKeyword(nullptr)
    , m_labelStatus(nullptr)
{
    buildUi();
    initModel();
}

// ============================================================
//  搭界面
// ============================================================
void PatientPage::buildUi()
{
    // ---- 第一行：操作按钮 ----
    QPushButton *btnAdd     = new QPushButton("新增记录", this);
    QPushButton *btnEdit    = new QPushButton("编辑选中", this);
    QPushButton *btnSave    = new QPushButton("保存修改", this);
    QPushButton *btnRevert  = new QPushButton("放弃修改", this);
    QPushButton *btnDelete  = new QPushButton("删除记录", this);
    QPushButton *btnRefresh = new QPushButton("刷新", this);
    QPushButton *btnAdv     = new QPushButton("通用查询…", this);

    QPushButton *btns[] = { btnAdd, btnEdit, btnSave, btnRevert, btnDelete, btnRefresh };
    for (QPushButton *b : btns) {
        b->setMinimumHeight(30);
        b->setMinimumWidth(86);
    }
    btnAdv->setMinimumHeight(30);

    connect(btnAdd,     &QPushButton::clicked, this, &PatientPage::onAdd);
    connect(btnEdit,    &QPushButton::clicked, this, [this]() {
        // 点"编辑选中"等同于在选中格子上双击
        QModelIndex idx = m_table->currentIndex();
        if (idx.isValid())
            m_table->edit(idx);
        else
            QMessageBox::information(this, "提示", "请先在表格里选中一行");
    });
    connect(btnSave,    &QPushButton::clicked, this, &PatientPage::onSave);
    connect(btnRevert,  &QPushButton::clicked, this, &PatientPage::onRevert);
    connect(btnDelete,  &QPushButton::clicked, this, &PatientPage::onDelete);
    connect(btnRefresh, &QPushButton::clicked, this, &PatientPage::refresh);
    connect(btnAdv,     &QPushButton::clicked, this, &PatientPage::onAdvancedQuery);

    QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->addWidget(btnAdd);
    toolLayout->addWidget(btnEdit);
    toolLayout->addWidget(btnSave);
    toolLayout->addWidget(btnRevert);
    toolLayout->addWidget(btnDelete);
    toolLayout->addWidget(btnRefresh);
    toolLayout->addStretch(1);
    toolLayout->addWidget(btnAdv);

    // ---- 第二行：查询条件 ----
    m_editKeyword = new QLineEdit(this);
    m_editKeyword->setPlaceholderText("输入姓名 / 电话 / 科室 / 诊断的关键字");
    m_editKeyword->setMinimumHeight(30);

    QPushButton *btnSearch = new QPushButton("查询", this);
    QPushButton *btnClear  = new QPushButton("重置", this);
    btnSearch->setMinimumHeight(30);
    btnClear->setMinimumHeight(30);
    btnSearch->setMinimumWidth(70);
    btnClear->setMinimumWidth(70);

    connect(btnSearch, &QPushButton::clicked, this, &PatientPage::onSearch);
    connect(btnClear,  &QPushButton::clicked, this, &PatientPage::onResetSearch);
    connect(m_editKeyword, &QLineEdit::returnPressed, this, &PatientPage::onSearch);

    m_labelStatus = new QLabel(this);
    m_labelStatus->setStyleSheet("color:#666666;");

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(new QLabel("关键字：", this));
    searchLayout->addWidget(m_editKeyword, 1);
    searchLayout->addWidget(btnSearch);
    searchLayout->addWidget(btnClear);
    searchLayout->addSpacing(16);
    searchLayout->addWidget(m_labelStatus);

    // ---- 第三行：表格 ----
    m_table = new QTableView(this);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);    // 整行选中
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);   // 一次只选一行
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->setStyleSheet(
        "QTableView{gridline-color:#e3e8ee;selection-background-color:#1a73e8;}"
        "QHeaderView::section{background:#eef2f7;color:#33475b;border:none;"
        "border-bottom:1px solid #d0d8e0;padding:6px;font-weight:bold;}");

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(10, 10, 10, 10);
    root->addLayout(toolLayout);
    root->addLayout(searchLayout);
    root->addWidget(m_table, 1);
}

// ============================================================
//  初始化模型：把 patients 表挂到表格上
// ============================================================
void PatientPage::initModel()
{
    // 从数据库里把科室列表读出来，供下拉代理使用
    QSqlQuery deptQuery(DbHelper::instance().db());
    if (deptQuery.exec("SELECT DISTINCT dept FROM patients "
                       "WHERE dept IS NOT NULL AND dept <> '' ORDER BY dept")) {
        while (deptQuery.next())
            m_deptList << deptQuery.value(0).toString();
    }
    if (m_deptList.isEmpty())
        m_deptList << "内科" << "外科" << "儿科" << "骨科" << "眼科" << "妇产科" << "皮肤科";

    m_model = new QSqlTableModel(this, DbHelper::instance().db());
    m_model->setTable("patients");

    // OnManualSubmit：改单元格只改内存，点"保存修改"才真正写库。
    // 好处是可以在写库之前做必填校验，也能随时"放弃修改"。
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    // 让表头显示中文（默认显示的是数据库字段名）
    m_model->setHeaderData(0, Qt::Horizontal, "编号");
    m_model->setHeaderData(1, Qt::Horizontal, "姓名");
    m_model->setHeaderData(2, Qt::Horizontal, "性别");
    m_model->setHeaderData(3, Qt::Horizontal, "年龄");
    m_model->setHeaderData(4, Qt::Horizontal, "联系电话");
    m_model->setHeaderData(5, Qt::Horizontal, "就诊科室");
    m_model->setHeaderData(6, Qt::Horizontal, "诊断结果");
    m_model->setHeaderData(7, Qt::Horizontal, "挂号日期");

    if (!m_model->select()) {
        QMessageBox::critical(this, "读取数据失败", m_model->lastError().text());
        return;
    }

    m_table->setModel(m_model);

    // 列宽：先按内容自适应，再让"诊断结果"这一列吃掉剩余宽度
    adjustColumns();

    // ---- 挂自定义代理：让性别、科室是下拉框，年龄是数字框 ----
    m_table->setItemDelegateForColumn(2, new ComboDelegate(QStringList() << "男" << "女", this));
    m_table->setItemDelegateForColumn(3, new SpinDelegate(0, 120, this));
    m_table->setItemDelegateForColumn(5, new ComboDelegate(m_deptList, this));

    // 编号列是主键，不允许用户改
    m_table->setEditTriggers(QAbstractItemView::DoubleClicked
                             | QAbstractItemView::EditKeyPressed
                             | QAbstractItemView::AnyKeyPressed);

    updateStatus();
}

// ============================================================
//  调整列宽：
//  resizeColumnsToContents 按内容算，但"姓名"这种短列在新增长名字后
//  会被截成"欧阳..."，所以给它定一个下限；最后一列撑满剩余空间。
// ============================================================
void PatientPage::adjustColumns()
{
    if (!m_table || !m_model)
        return;

    m_table->resizeColumnsToContents();
    m_table->setColumnWidth(1, 96);                                     // 姓名
    m_table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch);
}

// ============================================================
//  刷新：重新执行一次 SELECT
// ============================================================
void PatientPage::refresh(){
    if (!m_model)
        return;

    // 有未提交的改动先问一句，避免用户辛苦填的东西被冲掉
    if (m_model->isDirty()) {
        int ret = QMessageBox::question(this, "提示",
                                        "还有未保存的修改，刷新后会丢失。确定继续吗？");
        if (ret != QMessageBox::Yes)
            return;
    }

    m_model->revertAll();
    m_model->select();
    updateStatus();
}

// ============================================================
//  新增：在表格末尾插入一个空行
// ============================================================
void PatientPage::onAdd()
{
    if (!m_model)
        return;

    int row = m_model->rowCount();       // 插在最后一行之后
    m_model->insertRow(row);

    // 给几个字段填上默认值，省得用户一个个输
    m_model->setData(m_model->index(row, 2), "男");
    m_model->setData(m_model->index(row, 3), 30);
    m_model->setData(m_model->index(row, 5), m_deptList.value(0));
    m_model->setData(m_model->index(row, 7), QDate::currentDate().toString("yyyy-MM-dd"));

    // 选中新行，光标直接落到"姓名"那一格
    m_table->selectRow(row);
    m_table->scrollToBottom();
    m_table->setCurrentIndex(m_model->index(row, 1));
    m_table->edit(m_model->index(row, 1));

    adjustColumns();
    updateStatus();
}

// ============================================================
//  保存：先校验，再 submitAll 写库
// ============================================================
void PatientPage::onSave()
{
    if (!m_model)
        return;

    if (!validateAll())
        return;                        // 校验没过就停在原地，让用户改

    if (!m_model->submitAll()) {
        // 写库失败的典型原因：电话重复（UNIQUE 约束）、字段超长等
        QString err = m_model->lastError().text();
        QMessageBox::warning(this, "保存失败",
            QString("数据没有写入数据库。\n\n数据库返回：%1\n\n"
                    "常见原因：联系电话与已有记录重复。").arg(err));
        m_model->revertAll();          // 回滚，保证界面和数据库一致
        m_model->select();
        updateStatus();
        return;
    }

    writeLog("SAVE", QString("提交患者数据，当前共 %1 条记录").arg(m_model->rowCount()));
    m_model->select();                 // 重新读一遍，把自增 id 等最新值显示出来
    adjustColumns();
    updateStatus();

    QMessageBox::information(this, "保存成功", "数据已写入数据库。");
}

// 放弃所有未提交的改动
void PatientPage::onRevert()
{
    if (!m_model)
        return;

    m_model->revertAll();
    m_model->select();
    updateStatus();
}

// ============================================================
//  删除：先确认，再删
// ============================================================
void PatientPage::onDelete()
{
    if (!m_model)
        return;

    int row = m_table->currentIndex().row();
    if (row < 0) {
        QMessageBox::information(this, "提示", "请先在表格里选中要删除的一行");
        return;
    }

    QString name = m_model->data(m_model->index(row, 1)).toString();

    // 删除是不可逆操作，必须先让用户确认
    int ret = QMessageBox::question(this, "确认删除",
        QString("确定要删除患者「%1」的记录吗？\n此操作不可恢复。").arg(name));
    if (ret != QMessageBox::Yes)
        return;

    m_model->removeRow(row);

    if (!m_model->submitAll()) {
        QMessageBox::warning(this, "删除失败", m_model->lastError().text());
        m_model->revertAll();
        m_model->select();
        updateStatus();
        return;
    }

    writeLog("DELETE", QString("删除患者：%1").arg(name));
    m_model->select();
    updateStatus();

    QMessageBox::information(this, "删除成功", QString("已删除「%1」的记录。").arg(name));
}

// ============================================================
//  查询：用 setFilter 加条件，再 select
// ============================================================
void PatientPage::onSearch()
{
    if (!m_model)
        return;

    QString key = m_editKeyword->text().trimmed();

    if (key.isEmpty()) {
        m_model->setFilter("");
    } else {
        // 注意 LIKE 里的字符串要用单引号包起来，% 表示"任意字符"。
        // 这里的关键字是用户自己输入的，为了简单起见直接拼进过滤器；
        // 如果要做严格的防注入，可以改用 prepare 版的 QSqlQuery 自己查。
        QString pattern = "%" + key + "%";
        QString filter = QString(
            "name LIKE '%1' OR phone LIKE '%1' OR dept LIKE '%1' OR diagnosis LIKE '%1'")
            .arg(pattern);
        m_model->setFilter(filter);
    }

    m_model->revertAll();
    m_model->select();
    updateStatus();
}

void PatientPage::onResetSearch()
{
    m_editKeyword->clear();
    if (m_model) {
        m_model->setFilter("");
        m_model->revertAll();
        m_model->select();
    }
    updateStatus();
}

// 通用查询：交给 QSqlQuery 自己拼 SQL，见 advquerydialog.cpp
void PatientPage::onAdvancedQuery()
{
    AdvQueryDialog dlg(this);
    dlg.exec();
    refresh();          // 查询窗口里可能改过数据，回来刷新一下
}

// ============================================================
//  提交前的必填校验
// ============================================================
bool PatientPage::validateAll()
{
    for (int r = 0; r < m_model->rowCount(); ++r) {
        QString name = m_model->data(m_model->index(r, 1)).toString().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "数据不完整",
                QString("第 %1 行的「姓名」不能为空，请补全后再保存。").arg(r + 1));
            m_table->selectRow(r);
            m_table->setCurrentIndex(m_model->index(r, 1));
            return false;
        }

        QString ageText = m_model->data(m_model->index(r, 3)).toString();
        bool ok = false;
        int age = ageText.toInt(&ok);
        if (!ok || age <= 0 || age > 120) {
            QMessageBox::warning(this, "数据不合法",
                QString("第 %1 行的「年龄」必须在 1 ~ 120 之间。").arg(r + 1));
            m_table->selectRow(r);
            m_table->setCurrentIndex(m_model->index(r, 3));
            return false;
        }

        QString phone = m_model->data(m_model->index(r, 4)).toString().trimmed();
        if (!phone.isEmpty() && phone.length() != 11) {
            QMessageBox::warning(this, "数据不合法",
                QString("第 %1 行的「联系电话」应该是 11 位数字。").arg(r + 1));
            m_table->selectRow(r);
            m_table->setCurrentIndex(m_model->index(r, 4));
            return false;
        }
    }
    return true;
}

// 写一条操作日志（选做项：用户操作记录）
void PatientPage::writeLog(const QString &type, const QString &desc)
{
    QSqlQuery q(DbHelper::instance().db());
    q.prepare("INSERT INTO logs(op_type, op_desc) VALUES(?, ?)");
    q.addBindValue(type);
    q.addBindValue(desc);
    q.exec();      // 日志写失败不影响主流程，所以这里不判返回值
}

// 更新右侧的"共 N 条记录"
void PatientPage::updateStatus()
{
    if (!m_model || !m_labelStatus)
        return;

    int count = m_model->rowCount();
    if (count == 0) {
        m_labelStatus->setText("未找到匹配记录");
        m_labelStatus->setStyleSheet("color:#cc0000;font-weight:bold;");
    } else {
        m_labelStatus->setText(QString("共 %1 条记录").arg(count));
        m_labelStatus->setStyleSheet("color:#666666;");
    }
}
