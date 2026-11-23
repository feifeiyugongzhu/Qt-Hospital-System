#ifndef LOGPAGE_H
#define LOGPAGE_H

#include <QWidget>

class QSqlTableModel;
class QTableView;
class QLabel;

// 操作日志页（选做项）：把 logs 表显示出来，只读
class LogPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogPage(QWidget *parent = nullptr);
    void refresh();

private:
    QSqlTableModel *m_model;
    QTableView *m_table;
    QLabel *m_labelCount;
};

#endif // LOGPAGE_H
