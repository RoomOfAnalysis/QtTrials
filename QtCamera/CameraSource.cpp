#include "CameraSource.h"

#include <QTimer>
#include <QMediaDevices>

CameraSource::CameraSource(QCameraDevice const& device, QObject* parent): QVideoSink(parent)
{
    m_cam = new QCamera(device, this);

    // FIXME: seems no way to change the camera fps, even choose one from the available video formats still no effect?
    // very confused... without setting the fps is always 30, but it will change to 60 after setting the camera format with any value
    //auto const& fs = device.videoFormats();
    //auto f = fs[0];
    //qDebug() << f.minFrameRate() << f.maxFrameRate();
    //for (auto const ff : fs)
    //    if (qRound((ff.minFrameRate() + ff.maxFrameRate()) / 2.0) == 30) f = ff;
    //qDebug() << f.minFrameRate() << f.maxFrameRate();
    //m_cam->setCameraFormat(f);

    m_cap.setCamera(m_cam);
    m_cap.setVideoSink(this);

    m_fps_timer = new QTimer(this);
    m_fps_timer->setInterval(1000); // 1s

    connect(m_fps_timer, &QTimer::timeout, this, &CameraSource::timer_handle);
    connect(this, &QVideoSink::videoFrameChanged, this, &CameraSource::frame_handle);
}

CameraSource::~CameraSource()
{
    disconnect(m_cam, nullptr, nullptr, nullptr);
    m_cam->setParent(nullptr);
    delete m_cam;
    m_cam = nullptr;
}

void CameraSource::set_video_sink(QVideoSink* video_sink)
{
    if (m_video_sink == video_sink) return;

    m_video_sink = video_sink;
    emit video_sink_changed();
}

QVideoSink* CameraSource::video_sink() const
{
    return m_video_sink.get();
}

int CameraSource::fps() const
{
    return m_fps;
}

QMediaCaptureSession& CameraSource::get_cap_session()
{
    return m_cap;
}

void CameraSource::frame_handle(QVideoFrame const& frame)
{
    QVideoFrame f = frame; // shallow copy
    if (f.isValid() && f.map(QVideoFrame::ReadOnly))
    {
        // process every frame img
        // FIXME: memory usage is quite high here...
        // 19GB peak increase and release at 30fps, but only 5GB at 60fps??
        //auto img = f.toImage().convertToFormat(QImage::Format_RGB888);
        f.unmap();
    }
    m_cnt++;
    if (m_video_sink) m_video_sink->setVideoFrame(frame);
}

void CameraSource::timer_handle()
{
    m_fps = m_cnt;
    m_cnt = 0;
    emit fps_changed();
}

void CameraSource::start_cam()
{
    m_cam->start();
    m_fps_timer->start();
}
void CameraSource::stop_cam()
{
    m_cam->stop();
}