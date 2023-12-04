#pragma once

#include <memory>
#include <atomic>
#include <functional>

#include <QVideoSink>
#include <QPointer>

class QtFrameHandler: public QVideoSink
{
    Q_OBJECT

public:
    explicit QtFrameHandler(QObject* parent = nullptr);
    ~QtFrameHandler() = default;

    void setVideoSink(QVideoSink* sink);
    [[nodiscard]] QVideoSink* videoSink() const;

    void setFrameProcessor(std::function<void(QImage*)> processor);

private slots:
    void frame_handle(QVideoFrame const& frame);

private:
    Q_DISABLE_COPY(QtFrameHandler)

    QPointer<QVideoSink> m_video_sink = nullptr;

    std::atomic<int> m_fps = 0;
    QTimer* m_fps_timer = nullptr;

    std::function<void(QImage*)> m_frame_processor = nullptr;
};