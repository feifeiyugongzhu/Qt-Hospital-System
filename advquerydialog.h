#ifndef ADVQUERYDIALOG_H
#define ADVQUERYDIALOG_H

#include <QDialog>
#include <QStringList>

class QComboBox;
class QLineEdit;
class QLabel;
class QTableView;
class QSqlQueryModel;

// ============================================================
//  AdvQueryDialog —— 通用查询窗口
//
//  它和主表格用的是两套不同的模型，正好用来对比：
//    主表格  用 QSqlTableModel —— 能改、能写回数据库
//    这个窗口用 QSqlQueryModel —— 只读，专门负责"显示一条 SQL 的查询结果"
//
//  SQL 由用户在下拉框里选字段和条件，程序用 prepare + 绑定值拼出来，
//  既实现了"通用"，又不会被注入。
// ============================================================
class AdvQueryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdvQueryDialog(QWidget *parent = nullptr);

private slots:
    void onExecute();

private:
    QComboBox *m_comboField;     // 查哪个字段
    QComboBox *m_comboOp;        // 用什么条件
    QLineEdit *m_editValue;      // 查什么值
    QLabel *m_labelResult;       // 查到几条
    QTableView *m_table;         // 结果表
    QSqlQueryModel *m_qmodel;    // 只读的结果模型

    QStringList m_fieldNames;    // 显示用的中文名
    QStringList m_columnNames;   // 对应的数据库列名
};

#endif // ADVQUERYDIALOG_H
