#include "delegates.h"

#include <QComboBox>
#include <QSpinBox>

// ============================================================
//  ComboDelegate —— 下拉框代理
// ============================================================

ComboDelegate::ComboDelegate(const QStringList &options, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_options(options)
{
}

// 造编辑器：这里返回一个 QComboBox
QWidget *ComboDelegate::createEditor(QWidget *parent,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QComboBox *editor = new QComboBox(parent);
    editor->addItems(m_options);
    return editor;
}

// 把单元格里原来的值填进下拉框（这样一打开就停在该行当前的值上）
void ComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox *>(editor);
    QString current = index.model()->data(index, Qt::EditRole).toString();

    int pos = combo->findText(current);
    if (pos >= 0)
        combo->setCurrentIndex(pos);
}

// 用户选完之后，把下拉框的值写回 Model
void ComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox *>(editor);
    model->setData(index, combo->currentText(), Qt::EditRole);
}

// 让编辑器和单元格严丝合缝地对齐
void ComboDelegate::updateEditorGeometry(QWidget *editor,
                                         const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

// ============================================================
//  SpinDelegate —— 数字框代理
// ============================================================

SpinDelegate::SpinDelegate(int minValue, int maxValue, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_min(minValue)
    , m_max(maxValue)
{
}

QWidget *SpinDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QSpinBox *editor = new QSpinBox(parent);
    editor->setRange(m_min, m_max);
    editor->setFrame(false);
    return editor;
}

void SpinDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QSpinBox *spin = static_cast<QSpinBox *>(editor);
    int value = index.model()->data(index, Qt::EditRole).toInt();
    spin->setValue(value);
}

void SpinDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    QSpinBox *spin = static_cast<QSpinBox *>(editor);
    spin->interpretText();                       // 确保手输的值也生效
    model->setData(index, spin->value(), Qt::EditRole);
}

void SpinDelegate::updateEditorGeometry(QWidget *editor,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
