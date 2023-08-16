#pragma once

#include <QtWidgets/QMainWindow>

class QtWidgetRecorder;

namespace Ui
{
    class MainWindow;
}

class Recorder: public QMainWindow
{
    Q_OBJECT

public:
    Recorder(QWidget* parent = nullptr);
    ~Recorder();

private:
    void setup_connections();

private slots:
    void file_open_slot();
    void exit_slot();
    void start_recorder();
    void stop_recorder();

private:
    Ui::MainWindow* ui;
    QMovie* m_gif;
    QtWidgetRecorder* m_recorder;
};
