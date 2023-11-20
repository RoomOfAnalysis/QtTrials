#pragma once

#include "VideoCapturer.h"

class CameraCapturer: public VideoCapturer, public rtc::VideoSinkInterface<webrtc::VideoFrame>
{
public:
    static CameraCapturer* Create(size_t width, size_t height, size_t target_fps, size_t capture_device_index);
    virtual ~CameraCapturer();

    void OnFrame(webrtc::VideoFrame const& frame) override;

public:
    void Destroy();

private:
    CameraCapturer();
    bool Init(size_t width, size_t height, size_t target_fps, size_t capture_device_index);

    rtc::scoped_refptr<webrtc::VideoCaptureModule> m_vcm;
    webrtc::VideoCaptureCapability m_capability;
    std::unique_ptr<rtc::Thread> m_vcm_capturer;
};
