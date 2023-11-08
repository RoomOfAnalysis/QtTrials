#include <QCamera>
#include <QMediaDevices>
#include <QVideoWidget>
#include <QAudioDevice>
#include <QApplication>
#include <QDebug>

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
    }

    return app.exec();
}