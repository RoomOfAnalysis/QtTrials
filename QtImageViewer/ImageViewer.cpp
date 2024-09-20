#include "ImageViewer.h"
#include "ui_ImageViewer.h"

#include "ImageView.h"
#include "Image.h"

#include <QFileDialog>

ImageViewer::ImageViewer(QWidget* parent): QMainWindow(parent), ui(new Ui::ImageViewer)
{
    ui->setupUi(this);

    setup_connections();
}

ImageViewer::~ImageViewer()
{
    delete ui;
}

void ImageViewer::setup_connections()
{
    connect(ui->actionOpen, &QAction::triggered, this, &ImageViewer::on_file_open);
}

void ImageViewer::on_file_open()
{
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("*;;*.bmp;; *.png;; *.jpg;; *.tif;; *.GIF"));
    if (filename.isEmpty())
        return;
    else
    {
        m_image.reset(new Image(filename));
        qDebug() << m_image->metadata()->fileInfo();
        qDebug() << m_image->metadata()->mimeType() << m_image->metadata()->orientation();
        qDebug() << m_image->metadata()->getExifTags();
        qDebug() << m_image->pixmap().isNull();
        ui->image_view->showPixmap(m_image->pixmap());
    }
}