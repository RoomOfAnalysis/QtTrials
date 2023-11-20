#pragma once

#include "webrtc_headers.h"

class VideoCapturer: public rtc::VideoSourceInterface<webrtc::VideoFrame>
{
public:
    class FramePreprocessor
    {
    public:
        virtual ~FramePreprocessor() = default;

        virtual webrtc::VideoFrame Preprocess(webrtc::VideoFrame const& frame) = 0;
    };

    ~VideoCapturer() override;

    void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink, rtc::VideoSinkWants const& wants) override;
    void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;
    void SetFramePreprocessor(std::unique_ptr<FramePreprocessor> preprocessor)
    {
        webrtc::MutexLock lock(&m_lock);
        m_preprocessor = std::move(preprocessor);
    }

protected:
    void OnFrame(webrtc::VideoFrame const& frame);
    rtc::VideoSinkWants GetSinkWants();

private:
    void UpdateVideoAdapter();
    webrtc::VideoFrame MaybePreprocess(webrtc::VideoFrame const& frame);

    webrtc::Mutex m_lock;
    std::unique_ptr<FramePreprocessor> m_preprocessor RTC_GUARDED_BY(m_lock);
    rtc::VideoBroadcaster m_broadcaster;
    cricket::VideoAdapter m_video_adapter;
};
