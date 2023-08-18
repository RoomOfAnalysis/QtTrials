#pragma once

#include <QtWidgets/QMainWindow>

class Capturer;
class ScreenListModel;

class QVideoWidget;
class QItemSelection;

namespace Ui
{
    class MainWindow;
}

class Viewer: public QMainWindow
{
    Q_OBJECT

public:
    Viewer(QWidget* parent = nullptr);
    ~Viewer();

private:
    void setup_connections();

    void on_screen_selection_changed(QItemSelection selection);
private slots:
    void start_capturer();
    void stop_capturer();

private:
    Ui::MainWindow* ui;
    ScreenListModel* m_screen_list_model;
    Capturer* m_capturer;
    QVideoWidget* m_video_widget;
};
