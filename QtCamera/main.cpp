#include <QCamera>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QVideoWidget>
#include <QAudioDevice>
#include <QApplication>
#include <QDebug>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    //const auto audios = QMediaDevices::audioOutputs();
    //for (const QAudioDevice& audioDevice : audios)
    //    qDebug() << audioDevice.description();

    QCamera* camera = nullptr;
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.count() > 0)
    {
        // choose first available camera
        qDebug() << cameras[0].description();
        camera = new QCamera(cameras[0]);
    }

    if (camera)
    {
        QMediaCaptureSession* captureSession = new QMediaCaptureSession();
        captureSession->setCamera(camera);
        auto* viewfinder = new QVideoWidget;
        captureSession->setVideoOutput(viewfinder);
        viewfinder->show();

        camera->start();

        qDebug() << "error: "
                 << camera->error()
                 //<< "\n state:" << camera->state()  // unavailable in Qt 6.5
                 //<< "\n status: " << camera->status()
                 << "\n errorstring: "
                 << camera->errorString()
                 //<< "\n camptureMode: " << camera-captureMode()
                 //<< "\n camera.lockStatus: " << camera->lockStatus()
                 //<< "\n availableMetaData: " << camera->availableMetaData()
                 << "\n camera.isAvailable: " << camera->isAvailable() << "\n camera.isActive: " << camera->isActive()
                 << "\n viewfinder.isEnabled: " << viewfinder->isEnabled()
                 << "\n viewfinder.isVisible: " << viewfinder->isVisible();
    }

    return app.exec();
}