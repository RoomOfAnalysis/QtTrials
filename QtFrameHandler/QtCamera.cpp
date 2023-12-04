#include "QtCamera.h"

#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>
#include <QMediaDevices>

QtCamera::QtCamera(QObject* parent)
{
    m_cam = new QCamera(this);
    m_cap = new QMediaCaptureSession(this);
}

QtCamera::~QtCamera()
{
    stop();
    disconnect(m_cam, nullptr, nullptr, nullptr);
    m_cam->setParent(nullptr);
    delete m_cam;
    m_cam = nullptr;
}

void QtCamera::setCameraDevice(QCameraDevice const& camera_device)
{
    // try to set camera frame rate to 30
    auto const& fs = camera_device.videoFormats();
    auto f = fs[0];
    for (auto const ff : fs)
        if (qRound((ff.minFrameRate() + ff.maxFrameRate()) / 2.0) == 30) f = ff;
    qDebug() << "camera frame rate:" << f.minFrameRate() << f.maxFrameRate() << "\npixel format: " << f.pixelFormat()
             << "\nresolution:" << f.resolution();

    m_cam->setCameraDevice(camera_device);
    // need set camera format, otherwise it will be very laggy and consume large amount of memory
    m_cam->setCameraFormat(f);
    m_cap->setCamera(m_cam);
}

void QtCamera::setVideoSink(QVideoSink* video_sink)
{
    if (m_video_sink == video_sink) return;

    m_video_sink = video_sink;
    m_cap->setVideoSink(m_video_sink);
    emit video_sink_changed();
}

QVideoSink* QtCamera::getVideoSink() const
{
    return m_video_sink.get();
}

void QtCamera::start()
{
    m_cam->start();
}

void QtCamera::stop()
{
    m_cam->stop();
}

QList<QCameraDevice> const QtCamera::getAvailableCameras()
{
    return QMediaDevices::videoInputs();
}