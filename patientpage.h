#ifndef PATIENTPAGE_H
#define PATIENTPAGE_H

#include <QWidget>
#include <QStringList>

class QSqlTableModel;
class QTableView;
class QLineEdit;
class QLabel;
class QPushButton;

// ============================================================
//  PatientPage —— 患者信息管理页（本实验的主体）
//
//  Model  ：QSqlTableModel，直接和 patients 表绑定
//  View   ：QTableView，把 Model 里的数据画成表格
//  Control：本类里的各个按钮槽函数
//
//  关键点：QSqlTableModel 是"可写"的模型。在表格里改一个格子，
//  Model 里对应数据就变了，再 submitAll() 就写进数据库。
//  这就是 View-Model 结构省事的地方 —— 不用手工拼 UPDATE 语句。
// ============================================================
class PatientPage : public QWidget
{
    Q_OBJECT

public:
    explicit PatientPage(QWidget *parent = nullptr);

    void refresh();                       // 重新从数据库读一遍

private slots:
    void onAdd();                          // 新增一行
    void onSave();                         // 把改动提交到数据库
    void onRevert();                       // 放弃改动
    void onDelete();                       // 删除选中行
    void onSearch();                       // 按关键字查询
    void onResetSearch();                  // 清空查询条件
    void onAdvancedQuery();                // 通用查询（QSqlQuery）

private:
    void buildUi();
    void initModel();
    void adjustColumns();                  // 调整列宽
    bool validateAll();                    // 提交前的必填校验
    void writeLog(const QString &type, const QString &desc);
    void updateStatus();

    QSqlTableModel *m_model;     // 数据模型
    QTableView *m_table;         // 表格视图
    QLineEdit *m_editKeyword;    // 关键字输入框
    QLabel *m_labelStatus;       // 右侧状态：共几条 / 无结果
    QStringList m_deptList;      // 科室列表（给下拉代理用）
};

#endif // PATIENTPAGE_H
