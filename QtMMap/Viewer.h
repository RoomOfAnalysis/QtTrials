#pragma once

#include <QMainWindow>

class Capturer;
class FrameReader;
class FrameWriter;
class FrameReaderMmap;
class FrameWriterMmap;
class FrameReaderSocket;
class FrameWriterSocket;
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
#ifdef USE_MMAP
    FrameReaderMmap* m_frame_reader = nullptr;
    FrameWriterMmap* m_frame_writer = nullptr;
#elif defined(USE_SOCKET)
    FrameReaderSocket* m_frame_reader = nullptr;
    FrameWriterSocket* m_frame_writer = nullptr;
#else
    FrameReader* m_frame_reader = nullptr;
    FrameWriter* m_frame_writer = nullptr;
#endif
    QVideoWidget* m_video1 = nullptr;
    QVideoWidget* m_video2 = nullptr;
};
