#pragma once

#include <QtCore>

namespace QtUtils
{
    QString compilerInfo();
    QString systemInfo();
    bool isMainThread();
    void setUTF8Codec();
    void loadFonts(QStringList const& paths);
    void loadQSS(QStringList const& paths);

    QRect desktopRect();
    QPixmap grabDefaultScreen();
    QPixmap grabCombinedScreens();
    void moveToCenter(QWidget* w);
    void moveToCenter(QWidget* child, QWidget* parent);

    void quitApp();
    void rebootApp();
    qint64 getPidByProcessName(QString const& process_name);
    bool killProcess(qint64 pid);
} // namespace QtUtils