#pragma once

#include "CameraCapturer.h"

class CameraCapturerTrackSource: public webrtc::VideoTrackSource
{
public:
    static rtc::scoped_refptr<CameraCapturerTrackSource> Create(int width, int height, int fps)
    {
        const size_t kWidth = width;
        const size_t kHeight = height;
        const size_t kFps = fps;
        std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo());
        if (!info) return nullptr;
        int num_devices = info->NumberOfDevices();
        for (int i = 1; i < num_devices; ++i)
            if (auto capturer = absl::WrapUnique(CameraCapturer::Create(kWidth, kHeight, kFps, i)); capturer)
                return rtc::scoped_refptr<rtc::RefCountedObject<CameraCapturerTrackSource>>{
                    new rtc::RefCountedObject<CameraCapturerTrackSource>(std::move(capturer))};

        return nullptr;
    }

    static rtc::scoped_refptr<CameraCapturerTrackSource> Create(int width, int height, int fps, int deviceId)
    {
        const size_t kWidth = width;
        const size_t kHeight = height;
        const size_t kFps = fps;
        if (auto capturer = absl::WrapUnique(CameraCapturer::Create(kWidth, kHeight, kFps, deviceId)); capturer)
            return rtc::scoped_refptr<rtc::RefCountedObject<CameraCapturerTrackSource>>{
                new rtc::RefCountedObject<CameraCapturerTrackSource>(std::move(capturer))};

        return nullptr;
    }

    void Destroy()
    {
        if (m_capturer)
        {
            m_capturer->Destroy();
            m_capturer.reset();
        }
    }

protected:
    CameraCapturerTrackSource(std::unique_ptr<CameraCapturer> capturer)
        : VideoTrackSource(/*remote=*/false), m_capturer(std::move(capturer))
    {
    }

private:
    rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override { return m_capturer.get(); }
    std::unique_ptr<CameraCapturer> m_capturer;
};