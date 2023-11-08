#pragma once

#include <QVideoSink>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QPointer>
#include <QCameraDevice>

class CameraSource: public QVideoSink
{
    Q_OBJECT

public:
    explicit CameraSource(QCameraDevice const& device = {}, QObject* parent = nullptr);
    ~CameraSource() override;

    void set_video_sink(QVideoSink* video_sink);

    [[nodiscard]] QVideoSink* video_sink() const;
    [[nodiscard]] int fps() const;

    void start_cam();
    void stop_cam();

signals:
    void video_sink_changed();
    void fps_changed();

private slots:
    void frame_handle(QVideoFrame const& frame);
    void timer_handle();

private:
    Q_DISABLE_COPY(CameraSource)

    QCamera* m_cam = nullptr;
    QMediaCaptureSession m_cap{};
    QPointer<QVideoSink> m_video_sink = nullptr;
    int m_fps = -1, m_cnt = 0;
    QTimer* m_fps_timer = nullptr;
};