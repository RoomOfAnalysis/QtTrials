#include "Image.h"

#include <QImageReader>

#include "ImageOps.h"

Image::Image(QString const& file_path): m_is_edited{false}, m_meta(new MetaData(file_path))
{
    load();
}

bool Image::setEditedImage(QImage edited_img)
{
    if (edited_img.isNull() && edited_img.width() != 0)
    {
        discardEditedImage();
        m_edited_image = std::move(edited_img);
        m_is_edited = true;
        return true;
    }
    return false;
}

bool Image::discardEditedImage()
{
    if (m_edited_image.isNull()) return false;
    m_edited_image = QImage();
    m_is_edited = false;
    return true;
}

QPixmap const Image::pixmap() const
{
    return QPixmap::fromImage(isEdited() ? m_edited_image : m_origin_image);
}

QImage const Image::origin_image() const
{
    return m_origin_image;
}

QImage const Image::image() const
{
    return isEdited() ? m_edited_image : m_origin_image;
}

QSize Image::size() const
{
    return image().size();
}

int Image::height() const
{
    return image().height();
}

int Image::width() const
{
    return image().width();
}

bool Image::isEdited() const
{
    return m_is_edited;
}

void Image::load()
{
    QImageReader reader(m_meta->fileInfo().filePath());
    if (!reader.canRead()) return;

    m_origin_image = exifOriented(reader.read(), m_meta->orientation());
}

bool Image::save()
{
    return save(m_meta->fileInfo().filePath(), true);
}

bool Image::save(QString const& path, bool overwriten)
{
    if (QFile::exists(path) && !overwriten) return false;

    bool success = false;
    if (isEdited())
    {
        success = m_edited_image.save(path);
        m_origin_image.swap(m_edited_image);
        discardEditedImage();
    }
    else
        success = m_origin_image.save(path);
    if (success)
        if (path == m_meta->fileInfo().filePath())
            m_meta->refresh();
        else
            m_meta.reset(new MetaData(path));
    return success;
}