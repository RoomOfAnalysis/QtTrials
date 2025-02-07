#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDir>
#include <QFutureWatcher>
#include <QCollator>
#include <QtConcurrent>

#include "vol.h"

MainWindow::MainWindow(QWidget* parent): QWidget(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_graph = new Q3DScatterWidgetItem();
    m_quickWidget = new QQuickWidget();
    m_graph->setWidget(m_quickWidget);
    m_graph->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_graph->widget()->setFocusPolicy(Qt::StrongFocus);

    m_vol = new Vol(m_graph);
    m_vol->setFpsLabel(ui->fpsLabel);
    m_vol->setSliceSliders(ui->sliceXSlider, ui->sliceYSlider, ui->sliceZSlider);
    m_vol->setSliceLabels(ui->sliceImageXLabel, ui->sliceImageYLabel, ui->sliceImageZLabel);

    new QGridLayout(ui->display);
    ui->display->layout()->addWidget(m_quickWidget);

    setup_connections();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_graph;
    delete m_quickWidget;
}

void MainWindow::setup_connections()
{
    connect(ui->loadImagesBtn, &QPushButton::pressed, [this]() {
        auto paths =
            QFileDialog::getOpenFileNames(this, tr("Open Images"), QDir::homePath(), tr("*jpg;;*jpeg;;*.png;;*.bmp"));
        if (paths.isEmpty()) return;

        QCollator collator(locale());
        collator.setNumericMode(true);
        collator.setCaseSensitivity(Qt::CaseInsensitive);
        std::sort(paths.begin(), paths.end(), collator);

        auto* watcher = new QFutureWatcher<QList<QImage*>>();
        connect(watcher, &QFutureWatcher<QList<QImage*>>::finished, [this, watcher, paths]() {
            m_vol->loadVolumeData(watcher->result());
            watcher->deleteLater();
        });
        watcher->setFuture(QtConcurrent::run([this, paths]() {
            QList<QImage*> images;
            images.reserve(paths.size());
            for (qsizetype i = 0; i < paths.size(); i++)
            {
                auto* img = new QImage(paths[i]);
                *img = img->scaled(1024, 1024, Qt::KeepAspectRatio);
                images.push_back(img);
            }
            return images;
        }));
    });

    connect(ui->sliceXSlider, &QSlider::valueChanged, m_vol, &Vol::sliceX);
    connect(ui->sliceYSlider, &QSlider::valueChanged, m_vol, &Vol::sliceY);
    connect(ui->sliceZSlider, &QSlider::valueChanged, m_vol, &Vol::sliceZ);
}