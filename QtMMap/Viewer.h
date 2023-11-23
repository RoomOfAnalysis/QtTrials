#pragma once

#include <QMainWindow>

class Capturer;
class FrameReader;
class FrameWriter;
class QVideoWidget;

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

    void start();
    void stop();

private:
    void setup_connections();

private:
    Ui::MainWindow* ui;
    Capturer* m_capturer = nullptr;
    FrameReader* m_frame_reader = nullptr;
    FrameWriter* m_frame_writer = nullptr;
    QVideoWidget* m_video1 = nullptr;
    QVideoWidget* m_video2 = nullptr;
};
