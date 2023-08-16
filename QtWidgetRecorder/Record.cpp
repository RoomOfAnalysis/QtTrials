#include "ui_Recorder.h"

#include "Record.h"
#include "QtWidgetRecorder.h"

#include <QFileDialog>
#include <QMovie>
#include <QStandardPaths>

Recorder::Recorder(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_recorder = new QtWidgetRecorder(ui->label);
    setup_connections();
}

Recorder::~Recorder()
{
    delete m_gif;
    delete m_recorder;
    delete ui;
}

void Recorder::setup_connections()
{
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(file_open_slot()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(exit_slot()));
    connect(ui->start_btn, SIGNAL(clicked()), this, SLOT(start_recorder()));
    connect(ui->stop_btn, SIGNAL(clicked()), this, SLOT(stop_recorder()));
}

void Recorder::file_open_slot()
{
    if (auto filename = QFileDialog::getOpenFileName(nullptr, tr("Open Gif File"), "", tr("Gif Files (*.gif)"));
        !filename.isEmpty())
    {
        m_gif = new QMovie(filename);
        ui->label->setMovie(m_gif);
        m_gif->start();

        m_recorder->set_record_path(
            QDir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).filePath("test.mp4").toStdString().c_str());
    }
}

void Recorder::exit_slot()
{
    this->close();
}

void Recorder::start_recorder()
{
    m_recorder->start();
}

void Recorder::stop_recorder()
{
    m_recorder->stop();
}
