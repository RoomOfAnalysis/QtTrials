#include "ui_ScreenCapturerViewer.h"

#include "Viewer.h"
#include "Capturer.h"
#include "Models.h"

#include <QVideoWidget>

Viewer::Viewer(QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_video_widget = new QVideoWidget(ui->widget);

    m_screen_list_model = new ScreenListModel(QGuiApplication::screens(), this);
    ui->ScreenListView->setModel(m_screen_list_model);

    m_capturer = new Capturer();
    m_capturer->set_video_output(m_video_widget);

    setup_connections();
}

Viewer::~Viewer()
{
    delete m_video_widget;
    delete m_screen_list_model;
    delete m_capturer;
    delete ui;
}


void Viewer::setup_connections()
{
    connect(ui->ScreenListView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &Viewer::on_screen_selection_changed);
    connect(ui->StartBtn, &QPushButton::clicked, this, &Viewer::start_capturer);
    connect(ui->StopBtn, &QPushButton::clicked, this, &Viewer::stop_capturer);
}

void Viewer::on_screen_selection_changed(QItemSelection selection)
{
    if (auto indexes = selection.indexes(); !indexes.empty())
        m_capturer->set_screen(m_screen_list_model->screen(indexes.front()));
}

void Viewer::start_capturer()
{
    m_capturer->start();
}

void Viewer::stop_capturer()
{
    m_capturer->stop();
}