#pragma once

#include <QMainWindow>
#include <QMutex>
#include <QWaitCondition>

#include "CameraCapturerTrackSource.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow: public QMainWindow, public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // VideoSinkInterface
    void OnFrame(const webrtc::VideoFrame& frame) override;

Q_SIGNALS:
    void recvFrame();

private Q_SLOTS:
    void on_startBtn_clicked();

    void on_stopBtn_clicked();

    void OpenVideoCaptureDevice();
    void CloseVideoCaptureDevice();

    void on_updateDeviceBtn_clicked();

    void onRecvFrame();

private:
    Ui::MainWindow* ui;

    rtc::scoped_refptr<CameraCapturerTrackSource> m_video_capturer;

    QMutex m_mutex;
    rtc::scoped_refptr<webrtc::I420BufferInterface> m_i420_buffer;
};