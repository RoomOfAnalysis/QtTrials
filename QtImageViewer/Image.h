#pragma once

#include <QString>
#include <QPixmap>

#include "MetaData.h"

class Image
{
public:
    Image(QString const& file_path);
    ~Image() = default;

    bool setEditedImage(QImage edited_img);
    bool discardEditedImage();

    QPixmap const pixmap() const;
    QImage const origin_image() const;
    QImage const image() const;

    QSize size() const;
    int height() const;
    int width() const;

    bool isEdited() const;

    MetaData* metadata() const { return m_meta.get(); }

public slots:
    bool save();
    bool save(QString const& path, bool overwriten = true);

private:
    void load();

private:
    std::unique_ptr<MetaData> m_meta;
    QImage m_origin_image, m_edited_image;
    bool m_is_edited;
};