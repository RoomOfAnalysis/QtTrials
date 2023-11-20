#include "CameraCapturer.h"

CameraCapturer::CameraCapturer(): m_vcm(nullptr), m_vcm_capturer(rtc::Thread::Create())
{
    m_vcm_capturer->SetName("VCM Capturer", nullptr);
    m_vcm_capturer->Start();
    m_vcm_capturer->AllowInvokesToThread(m_vcm_capturer.get());
}

bool CameraCapturer::Init(size_t width, size_t height, size_t target_fps, size_t capture_device_index)
{
    bool ret = m_vcm_capturer->BlockingCall([this, capture_device_index, width, height, target_fps]() {
        std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> device_info(
            webrtc::VideoCaptureFactory::CreateDeviceInfo());

        char device_name[256];
        char unique_name[256];
        if (device_info->GetDeviceName(static_cast<uint32_t>(capture_device_index), device_name, sizeof(device_name),
                                       unique_name, sizeof(unique_name)) != 0)
        {
            Destroy();
            return false;
        }

        m_vcm = webrtc::VideoCaptureFactory::Create(unique_name);
        if (!m_vcm) return false;
        m_vcm->RegisterCaptureDataCallback(this);

        device_info->GetCapability(m_vcm->CurrentDeviceName(), 0, m_capability);

        m_capability.width = static_cast<int32_t>(width);
        m_capability.height = static_cast<int32_t>(height);
        m_capability.maxFPS = static_cast<int32_t>(target_fps);
        m_capability.videoType = webrtc::VideoType::kI420;

        if (m_vcm->StartCapture(m_capability) != 0)
        {
            Destroy();
            return false;
        }
        RTC_CHECK(m_vcm->CaptureStarted());

        return true;
    });
    return ret;
}

CameraCapturer* CameraCapturer::Create(size_t width, size_t height, size_t target_fps, size_t capture_device_index)
{
    std::unique_ptr<CameraCapturer> vcm_capturer(new CameraCapturer());
    if (!vcm_capturer->Init(width, height, target_fps, capture_device_index))
    {
        RTC_LOG(LS_WARNING) << "Failed to create CameraCapture(w = " << width << ", h = " << height
                            << ", fps = " << target_fps << ")";
        return nullptr;
    }
    return vcm_capturer.release();
}

void CameraCapturer::Destroy()
{
    if (!m_vcm) return;

    m_vcm_capturer->BlockingCall([this]() {
        m_vcm->StopCapture();
        m_vcm->DeRegisterCaptureDataCallback();
        // Release reference to VCM.
        m_vcm = nullptr;
    });
}

CameraCapturer::~CameraCapturer()
{
    Destroy();
}

void CameraCapturer::OnFrame(const webrtc::VideoFrame& frame)
{
    VideoCapturer::OnFrame(frame);
}