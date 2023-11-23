#pragma once

#include <memory>

#include <QObject>

class QScreenCapture;
class QMediaCaptureSession;
class QScreen;
class QVideoSink;

class Capturer: public QObject
{
    Q_OBJECT
public:
    Capturer();
    ~Capturer() = default;

    void set_screen(QScreen* screen);
    void set_video_sink(QVideoSink* sink);

public slots:
    void start();
    void stop();

private:
    std::unique_ptr<QScreenCapture> m_capturer = nullptr;
    std::unique_ptr<QMediaCaptureSession> m_session = nullptr;
};