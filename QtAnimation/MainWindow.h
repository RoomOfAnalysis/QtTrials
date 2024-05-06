#pragma once

#include <QMainWindow>

namespace Ui
{
    class MainWindow;
}

class MainWindow: public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setup_connections();

private:
    Ui::MainWindow* ui;
};