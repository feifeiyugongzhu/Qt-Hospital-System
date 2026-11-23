#ifndef DELEGATES_H
#define DELEGATES_H

#include <QStyledItemDelegate>
#include <QStringList>

// ============================================================
//  自定义代理（Delegate）
//
//  代理是 View 和 Model 之间的"中间人"：当你在表格里双击某个单元格，
//  是代理负责弹出一个编辑器让你改。
//  默认代理只会给一个 QLineEdit，改什么都能输进去。
//  想让"性别"只能选男/女、"年龄"只能输数字，就得自己写代理。
//
//  Qt 的数据流是这样走的：
//    用户双击 → View 找代理 → createEditor 造出编辑器
//             → setEditorData 把单元格的旧值填进编辑器
//    用户改完 → setModelData 把编辑器里的新值写回 Model
// ============================================================

// 下拉框代理：用在下拉选择类字段（性别、科室）
class ComboDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ComboDelegate(const QStringList &options, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
    QStringList m_options;   // 下拉框里的候选值
};

// 数字调节框代理：用在年龄这种只能是数字的字段
class SpinDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SpinDelegate(int minValue, int maxValue, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
    int m_min;
    int m_max;
};

#endif // DELEGATES_H
