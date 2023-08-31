#include "MainWindow.h"

#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);
    m_glass = new MagnificationGlass(this);

    setupConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_glass;
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && ui->img_label->geometry().contains(event->pos()))
        m_glass->magnify(grab().toImage(), event->pos());
}

void MainWindow::setupConnections()
{
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::on_file_open);

    connect(ui->show_btn, &QPushButton::clicked, this, &MainWindow::on_show);
    connect(ui->hide_btn, &QPushButton::clicked, this, &MainWindow::on_hide);
}

void MainWindow::on_show()
{
    m_glass->set_active(true);
    m_glass->magnify(grab().toImage(), QPoint{500, 500});
}

void MainWindow::on_hide()
{
    m_glass->set_active(false);
    m_glass->hide();
}

void MainWindow::on_file_open()
{
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("*;;*.bmp;; *.png;; *.jpg;; *.tif;; *.GIF"));
    if (filename.isEmpty())
        return;
    else
    {
        QImage img{};
        img.load(filename);
        ui->img_label->setPixmap(QPixmap::fromImage(img));
    }
}
