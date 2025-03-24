#include "thumbnailitem.hpp"
#include "ui_thumbnailitem.h"

#include <QFileInfo>

ThumbnailItem::ThumbnailItem(QWidget* parent): QWidget(parent), ui(new Ui::ThumbnailItem)
{
    ui->setupUi(this);
}

ThumbnailItem::~ThumbnailItem()
{
    delete ui;
}

void ThumbnailItem::setImage(QImage img)
{
    ui->thumbnailLabel->setPixmap(QPixmap::fromImage(img));
}

void ThumbnailItem::setImage(QImage img, QString const& path)
{
    QFileInfo fi(path);
    auto text = fi.fileName() + "\n" + fi.lastModified().toString();
    img_path = path;
    ui->txtLabel->setText(text);
    setImage(img);
}

void ThumbnailItem::setImage(QString const& path)
{
    QFileInfo fi(path);
    auto text = fi.fileName() + "\n" + fi.lastModified().toString();
    img_path = path;
    ui->thumbnailLabel->setPixmap(QPixmap::fromImage(QImage(path).scaled(100, 100, Qt::KeepAspectRatio)));
    ui->txtLabel->setText(text);
}

QString ThumbnailItem::getImagePath() const
{
    return img_path;
}

bool ThumbnailItem::isSelected() const
{
    return ui->checkBox->isChecked();
}
