#include "Capturer.h"

#include <QScreenCapture>
#include <QMediaCaptureSession>
#include <QScreen>
#include <QWindow>
#include <QDebug>

Capturer::Capturer()
{
    m_capturer = std::make_unique<QScreenCapture>();
    m_session = std::make_unique<QMediaCaptureSession>();

    m_session->setScreenCapture(m_capturer.get());

    connect(m_capturer.get(), &QScreenCapture::errorOccurred, this,
            [](QScreenCapture::Error error, QString const& errorString) {
                qFatal() << "QScreenCapture: Error occurred" << errorString;
            });
}

void Capturer::set_screen(QScreen* screen)
{
    m_capturer->setScreen(screen);
    qDebug() << m_capturer->error();
    qDebug() << screen << m_session->screenCapture()->screen();
}

void Capturer::set_video_sink(QVideoSink* sink)
{
    m_session->setVideoSink(sink);
}

void Capturer::start()
{
    m_capturer->start();
    qDebug() << "Capture started" << m_capturer->isActive();
}

void Capturer::stop()
{
    m_capturer->stop();
    qDebug() << "Capture stopped" << !m_capturer->isActive();
}
