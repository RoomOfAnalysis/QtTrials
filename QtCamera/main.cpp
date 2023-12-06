#include <QCamera>
#include <QMediaDevices>
#include <QVideoWidget>
#include <QAudioDevice>
#include <QApplication>
#include <QDebug>
#include <QMediaRecorder>
#include <QUrl>
#include <QMediaFormat>
#include <QTimer>

#include "CameraSource.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    //const auto audios = QMediaDevices::audioOutputs();
    //for (const QAudioDevice& audioDevice : audios)
    //    qDebug() << audioDevice.description();

    CameraSource* cs = nullptr;
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.count() > 0)
    {
        // choose first available camera
        qDebug() << cameras[0].description();
        auto const& fs = cameras[0].videoFormats();
        qDebug() << "available video format: ";
        for (auto const& f : fs)
            qDebug() << "\n resolution: " << f.resolution() << "\n min frame rate: " << f.minFrameRate()
                     << "\n max frame rate: " << f.maxFrameRate() << "\n pixel format: " << f.pixelFormat();
        cs = new CameraSource(cameras[0]);
    }

    if (cs)
    {
        QObject::connect(cs, &CameraSource::fps_changed, [&]() { qDebug() << "fps: " << cs->fps(); });

        auto* vw = new QVideoWidget;
        cs->set_video_sink(vw->videoSink());
        cs->start_cam();
        vw->show();

        auto& cap = cs->get_cap_session();
        auto* recorder = new QMediaRecorder(&cap);
        cap.setRecorder(recorder);
        // https://doc.qt.io/qt-6/qmediarecorder.html#setVideoResolution
        //recorder->setVideoResolution(800, 450);
        recorder->setQuality(QMediaRecorder::HighQuality);
        recorder->setVideoFrameRate(60);
        recorder->setOutputLocation(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/../test.mp4"));
        QMediaFormat media_format(QMediaFormat::MPEG4);
        media_format.setAudioCodec(QMediaFormat::AudioCodec::AAC);
        media_format.setVideoCodec(QMediaFormat::VideoCodec::H264);
        recorder->setMediaFormat(media_format);
        QObject::connect(recorder, &QMediaRecorder::errorOccurred, [&]() { qDebug() << recorder->errorString(); });
        qDebug() << recorder->isAvailable();
        recorder->record();

        auto* timer = new QTimer();
        timer->setInterval(10 * 1000);
        QObject::connect(timer, &QTimer::timeout, [&]() {
            recorder->stop();
            qDebug() << recorder->duration() / 1000;
            app.quit();
        });
        timer->start();
    }

    return app.exec();
}