#include "uistyle.h"

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QColor>

void applyLightTheme()
{
    // Fusion 是 Qt 自带的跨平台风格，颜色完全由 palette 控制，
    // 不像 windowsvista 风格那样跟着系统主题跑。
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QPalette p;
    p.setColor(QPalette::Window,          QColor(0xf4, 0xf6, 0xf9));   // 窗口底色
    p.setColor(QPalette::WindowText,      QColor(0x1f, 0x23, 0x28));   // 普通文字
    p.setColor(QPalette::Base,            QColor(0xff, 0xff, 0xff));   // 输入框/表格底
    p.setColor(QPalette::AlternateBase,   QColor(0xf6, 0xf9, 0xfc));   // 表格隔行色
    p.setColor(QPalette::Text,            QColor(0x1f, 0x23, 0x28));
    p.setColor(QPalette::Button,          QColor(0xeb, 0xef, 0xf4));
    p.setColor(QPalette::ButtonText,      QColor(0x1f, 0x23, 0x28));
    p.setColor(QPalette::Highlight,       QColor(0x1a, 0x73, 0xe8));   // 选中行
    p.setColor(QPalette::HighlightedText, QColor(0xff, 0xff, 0xff));
    p.setColor(QPalette::ToolTipBase,     QColor(0xff, 0xff, 0xff));
    p.setColor(QPalette::ToolTipText,     QColor(0x1f, 0x23, 0x28));
    p.setColor(QPalette::PlaceholderText, QColor(0x99, 0xa0, 0xa8));
    p.setColor(QPalette::Disabled, QPalette::Text,       QColor(0xaa, 0xb0, 0xb8));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0xaa, 0xb0, 0xb8));

    QApplication::setPalette(p);
}
