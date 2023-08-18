#pragma once

#include <QObject>
#include <QProcess>

class QTimer;

class QtWidgetRecorder: public QObject
{
    Q_OBJECT
public:
    explicit QtWidgetRecorder(QWidget* recorded_widget);
    ~QtWidgetRecorder();

    bool set_record_path(char const* recorded_file_path);

public slots:
    void start();
    void stop();

private slots:
    void capture_one_img();
    void on_process_exit(int exit_code, QProcess::ExitStatus exit_status);

private:
    QWidget* m_widget = nullptr;
    std::unique_ptr<QTimer> m_timer = nullptr;
    QString m_tmp_dir_path{};
    QString m_recorded_file_path{};
    int m_frame_cnt{0};
    std::unique_ptr<QProcess> m_process = nullptr;
};