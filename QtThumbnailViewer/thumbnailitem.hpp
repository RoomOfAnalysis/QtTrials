#pragma once

#include <QWidget>

namespace Ui
{
    class ThumbnailItem;
}

class ThumbnailItem: public QWidget
{
    Q_OBJECT
public:
    explicit ThumbnailItem(QWidget* parent = nullptr);
    ~ThumbnailItem();

    void setImage(QImage);
    void setImage(QImage img, QString const& path);
    void setImage(QString const& path);

    QString getImagePath() const;
    bool isSelected() const;

private:
    Ui::ThumbnailItem* ui;
    QString img_path;
};