#pragma once

#include <QMainWindow>
#include "MagnificationGlass.h"

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
    
protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    void setupConnections();

private slots:
    void on_file_open();
    void on_show();
    void on_hide();

private:
    Ui::MainWindow* ui;
    MagnificationGlass* m_glass;
};