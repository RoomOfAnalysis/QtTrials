#include "ui_Viewer.h"

#include "Viewer.h"
#include "Capturer.h"
#include "FrameReader.h"
#include "FrameWriter.h"

#include <QVideoWidget>
#include <QApplication>
#include <QMediaCaptureSession>

Viewer::Viewer(QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_video1 = new QVideoWidget();
    ui->video1->addWidget(m_video1);
    m_video2 = new QVideoWidget();
    ui->video2->addWidget(m_video2);

    m_capturer = new Capturer();
    m_capturer->set_screen(QApplication::screens()[0]);

    m_frame_writer = new FrameWriter();
    m_capturer->set_video_sink(m_frame_writer);
    m_frame_writer->set_video_sink(m_video1->videoSink());

    m_frame_reader = new FrameReader();
    m_frame_reader->set_video_sink(m_video2->videoSink());

    setup_connections();
}

Viewer::~Viewer()
{
    stop();
    m_frame_reader->stop();

    delete m_frame_reader;
    delete m_video1;
    delete m_video2;
    delete m_capturer;
    delete ui;
}

void Viewer::setup_connections()
{
    connect(ui->startBtn, &QPushButton::clicked, m_frame_reader, &FrameReader::start);
    connect(ui->stopBtn, &QPushButton::clicked, m_frame_reader, &FrameReader::stop);
}

void Viewer::start()
{
    m_capturer->start();
    m_frame_reader->start();
}

void Viewer::stop()
{
    m_capturer->stop();
}