#include "viewer.hpp"
#include "ui_viewer.h"

#include "thumbnailitem.hpp"

#include <QListWidgetItem>
#include <QFileInfo>
#include <QDir>
#include <QCollator>
#include <QCloseEvent>
#include <QtConcurrent>
#include <QElapsedTimer>

//#define SEQ

Viewer::Viewer(QWidget* parent): QWidget(parent), ui(new Ui::Viewer)
{
    ui->setupUi(this);

    connect(ui->thumbnailsListWidget, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        if (auto* t_item = qobject_cast<ThumbnailItem*>(ui->thumbnailsListWidget->itemWidget(item)); t_item)
            if (auto path = t_item->getImagePath(); !path.isEmpty()) ui->cmpViewer->setOriginImage(QImage(path));
    });

    connect(ui->renderBtn, &QPushButton::pressed, [this]() { render(); });
}

Viewer::~Viewer()
{
    delete ui;
}

void Viewer::loadImage(QImage img)
{
    ui->cmpViewer->setOriginImage(img);
    ui->cmpViewer->hideGeneratedViewer();
}

QImage Viewer::getImage() const
{
    return ui->cmpViewer->getOriginImage();
}

void Viewer::setImages(QList<QImage> const& images, QStringList const& paths)
{
    ui->imgNumLabel->setText(tr("Source Images (%1):").arg(images.size()));
    ui->thumbnailsListWidget->clear();

    img_paths = paths;
    for (qsizetype i = 0; i < images.size(); i++)
    {
        auto* t_item = new ThumbnailItem();
        t_item->setImage(images[i], img_paths[i]);
        auto* item = new QListWidgetItem();
        item->setSizeHint(QSize(300, 100));
        ui->thumbnailsListWidget->addItem(item);
        ui->thumbnailsListWidget->setItemWidget(item, t_item);
    }
}

QList<QImage> Viewer::loadImages(QStringList const& paths)
{
    QElapsedTimer timer;
    timer.start();

#ifdef SEQ
    QList<QImage> images;
    images.reserve(paths.size());
    for (qsizetype i = 0; i < paths.size(); i++)
        images.push_back(QImage(paths[i]).scaled(100, 100, Qt::KeepAspectRatio, Qt::FastTransformation));
#else
    auto images = QtConcurrent::blockingMappedReduced<QList<QImage>>(
        QThreadPool::globalInstance(), paths.cbegin(), paths.cend(),
        [](QString const& p) { return QImage(p).scaled(100, 100, Qt::KeepAspectRatio, Qt::FastTransformation); },
        [](QList<QImage>& imgs, QImage img) { imgs.append(img); });
#endif
    qDebug() << timer.elapsed() << "ms";

    return images;
}

void Viewer::render()
{
    // QStringList selected_paths;
    // for (auto i = 0; i < ui->thumbnailsListWidget->count(); i++)
    //     if (auto* t_item =
    //             qobject_cast<ThumbnailItem*>(ui->thumbnailsListWidget->itemWidget(ui->thumbnailsListWidget->item(i)));
    //         t_item)
    //         if (t_item->isSelected()) selected_paths.append(t_item->getImagePath());

    // if (selected_paths.isEmpty()) return;

    // auto dir_path = QFileInfo(selected_paths[0]).absoluteDir().absolutePath();

    // ui->cmpViewer->hideGeneratedViewer();

    // QCollator collator(locale());
    // collator.setNumericMode(true);
    // collator.setCaseSensitivity(Qt::CaseInsensitive);

    // QStringList selected_image_names(selected_paths.size());
    // std::transform(selected_paths.cbegin(), selected_paths.cend(), selected_image_names.begin(),
    //                [](QString path) { return QFileInfo(path).fileName(); });
    // std::sort(selected_image_names.begin(), selected_image_names.end(), collator);

    // do something with selected images
    ui->cmpViewer->setGeneratedImage(getImage());
}

void Viewer::closeEvent(QCloseEvent* event)
{
    if (auto p = parentWidget(); p)
    {
        p->hide();
        event->ignore();
        return;
    }
    QWidget::closeEvent(event);
}
