#pragma once

#include <QMainWindow>
#include <QMap>
#include <QPixmap>

class QListWidget;
class QMdiArea;

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void openImage();

private:
    QListWidget* m_list = nullptr;
    QMdiArea* m_mdi = nullptr;
    QMap<QString, QPixmap> m_imgs{};
};