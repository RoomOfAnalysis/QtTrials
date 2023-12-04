#pragma once

#include <QObject>
#include <QPointer>
#include <QCameraDevice>

class QCamera;
class QMediaCaptureSession;
class QVideoSink;

class QtCamera: public QObject
{
    Q_OBJECT
public:
    explicit QtCamera(QObject* parent = nullptr);
    ~QtCamera();

    void setCameraDevice(QCameraDevice const& camera_device);
    void setVideoSink(QVideoSink* video_sink);

    [[nodiscard]] QVideoSink* getVideoSink() const;

    void start();
    void stop();

    static QList<QCameraDevice> const getAvailableCameras();

signals:
    void video_sink_changed();

private:
    Q_DISABLE_COPY(QtCamera)

    QCamera* m_cam = nullptr;
    QMediaCaptureSession* m_cap = nullptr;
    QPointer<QVideoSink> m_video_sink = nullptr;
};