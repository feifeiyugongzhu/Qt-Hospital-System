#ifndef UISTYLE_H
#define UISTYLE_H

// 把整个程序统一成浅色界面。
// Qt 6 默认会跟随 Windows 的深色模式，自动变成黑底。
// 但实验报告要打印、要截图，浅色更清楚，所以程序启动时主动指定一套浅色配色。
void applyLightTheme();

#endif // UISTYLE_H
