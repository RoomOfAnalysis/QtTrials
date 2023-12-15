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
    void setupConnections();

private slots:
    void on_file_open();
    void on_file_save();
    void on_file_reset();
    void on_redo();
    void on_undo();
    void on_reset();

private:
    Ui::MainWindow* ui;
};