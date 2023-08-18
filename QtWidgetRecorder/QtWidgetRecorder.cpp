#include <QTimer>
#include <QUrl>
#include <QTemporaryDir>
#include <QString>
#include <Qwidget>
#include <QStringList>

#include "QtWidgetRecorder.h"

QtWidgetRecorder::QtWidgetRecorder(QWidget* recorded_widget): m_widget(recorded_widget)
{
    m_timer = std::make_unique<QTimer>();
    m_timer->setInterval(30);

    m_process = std::make_unique<QProcess>();
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_timer.get(), SIGNAL(timeout()), this, SLOT(capture_one_img()));
    connect(m_process.get(), SIGNAL(finished(int, QProcess::ExitStatus)), this,
            SLOT(on_process_exit(int, QProcess::ExitStatus)));
}

QtWidgetRecorder::~QtWidgetRecorder()
{
    if (auto tmp_dir = QDir(m_tmp_dir_path); tmp_dir.exists()) tmp_dir.removeRecursively();
}

bool QtWidgetRecorder::set_record_path(char const* recorded_file_path)
{
    QTemporaryDir tmp_dir;
    if (!tmp_dir.isValid())
    {
        qFatal() << "can not create temp folder";
        return false;
    }
    tmp_dir.setAutoRemove(false);
    m_tmp_dir_path = tmp_dir.path();
    qDebug() << m_tmp_dir_path;
    m_recorded_file_path = recorded_file_path;
    qDebug() << m_recorded_file_path;
    return true;
}

void QtWidgetRecorder::start()
{
    m_timer->start();

    qDebug() << "m_timer started";
}

void QtWidgetRecorder::stop()
{
    m_timer->stop();

    qDebug() << "m_timer stopped";

#ifdef _MSC_VER
    QString program("ffmpeg.exe");
#else
    QString program("ffmpeg");
#endif
    QStringList arguments;
    arguments << "-y" // yes for output video overwritten
              << "-framerate"
              << "30"
              << "-i" << QDir(m_tmp_dir_path).path() + "/frame_%d.png" << m_recorded_file_path;
    qDebug() << arguments;

    m_process->start(program, arguments);
}

void QtWidgetRecorder::capture_one_img()
{
    auto img = m_widget->grab().toImage();
    img.save(QDir(m_tmp_dir_path).filePath(QString::fromStdString("frame_" + std::to_string(m_frame_cnt++) + ".png")));
}

void QtWidgetRecorder::on_process_exit(int exit_code, QProcess::ExitStatus exit_status)
{
    if (exit_status != QProcess::NormalExit)
        qFatal() << m_process->errorString();
    else
        qDebug() << m_process->readAllStandardOutput();

    qDebug() << "tmp_dir removed" << QDir(m_tmp_dir_path).removeRecursively();
}