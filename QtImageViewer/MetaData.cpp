#include "MetaData.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDebug>
#include <QImageReader>

MetaData::MetaData(QString const& file_path)
{
    m_file_info.setFile(file_path);
    if (!m_file_info.isFile())
    {
        qInfo() << "MetaData file path is invalid:" << file_path;
        return;
    }
    load_mime();
    load_exif_image();
    load_orientation();
}

void MetaData::load_mime()
{
    QMimeDatabase mime_db;
    m_mime_type = mime_db.mimeTypeForFile(m_file_info.filePath(), QMimeDatabase::MatchContent);
}

void MetaData::load_exif_image()
{
    try
    {
        m_exif_img = Exiv2::ImageFactory::open((m_file_info.isSymLink()) ? m_file_info.symLinkTarget().toStdString() :
                                                                           m_file_info.filePath().toStdString());
        if (!m_exif_img || !m_exif_img->good())
        {
            m_exif_img = nullptr;
            qInfo() << "[Exiv2] invalid exif data from file:" << m_file_info.filePath();
            return;
        }
    }
    catch (Exiv2::Error const& e)
    {
        qInfo() << "[Exiv2] failed to open file:" << m_file_info.filePath() << "due to:" << e.what();
    }

    try
    {
        m_exif_img->readMetadata();
    }
    catch (Exiv2::Error const& e)
    {
        qInfo() << "[Exiv2] failed to read metadata from file:" << m_file_info.filePath() << "due to:" << e.what();
        m_exif_img = nullptr;
    }
}

void MetaData::load_orientation()
{
    /*
    enum Transformation
    {
        TransformationNone = 0,
        TransformationMirror = 1,
        TransformationFlip = 2,
        TransformationRotate180 = TransformationMirror | TransformationFlip,
        TransformationRotate90 = 4,
        TransformationMirrorAndRotate90 = TransformationMirror | TransformationRotate90,
        TransformationFlipAndRotate90 = TransformationFlip | TransformationRotate90,
        TransformationRotate270 = TransformationRotate180 | TransformationRotate90
    };
    */
    m_orientation = -1;
    if (auto reader = QImageReader(m_file_info.filePath()); reader.canRead())
        m_orientation = static_cast<int>(reader.transformation());
    else
        qDebug() << reader.errorString();

    ///*
    //* exif orientation is different from Qt's
    //* https://exiftool.org/TagNames/EXIF.html
    //* http://sylvana.net/jpegcrop/exif_orientation.html
    //1 = Horizontal (normal)
    //2 = Mirror horizontal
    //3 = Rotate 180
    //4 = Mirror vertical
    //5 = Mirror horizontal and rotate 270 CW
    //6 = Rotate 90 CW
    //7 = Mirror horizontal and rotate 90 CW
    //8 = Rotate 270 CW
    //*/
    //try
    //{
    //    if (auto const& exif_data = m_exif_img->exifData(); !exif_data.empty())
    //        if (auto it = exif_data.findKey(Exiv2::ExifKey("Exif.Image.Orientation"));
    //            it != exif_data.end() && it->count() != 0)
    //        {
    //            qInfo() << "[Exiv2] orientation:" << it->toInt64();
    //        }
    //}
    //catch (Exiv2::Error const& e)
    //{
    //    qInfo() << "[Exiv2] failed to get orientation due to:" << e.what();
    //}
}

QFileInfo const& MetaData::fileInfo() const
{
    return m_file_info;
}

QMimeType const& MetaData::mimeType() const
{
    return m_mime_type;
}

int MetaData::orientation() const
{
    return m_orientation;
}

// https://exiv2.org/tags.html
void MetaData::loadExifTags(bool force_reload)
{
    if (!m_exif_tags.empty() && !force_reload) return;

    m_exif_tags.clear();
    try
    {
        auto const& exif_data = m_exif_img->exifData();
        if (exif_data.empty()) return;

        Exiv2::ExifKey description("Exif.Image.ImageDescription");
        Exiv2::ExifKey make("Exif.Image.Make");
        Exiv2::ExifKey model("Exif.Image.Model");
        Exiv2::ExifKey dateTime("Exif.Image.DateTime");
        Exiv2::ExifKey exposureTime("Exif.Photo.ExposureTime");
        Exiv2::ExifKey fnumber("Exif.Photo.FNumber");
        Exiv2::ExifKey isoSpeedRatings("Exif.Photo.ISOSpeedRatings");
        Exiv2::ExifKey flash("Exif.Photo.Flash");
        Exiv2::ExifKey focalLength("Exif.Photo.FocalLength");
        Exiv2::ExifKey userComment("Exif.Photo.UserComment");

        if (auto it = exif_data.findKey(description); it != exif_data.end() && it->count() > 0)
            m_exif_tags.insert(QObject::tr("Image Description"), exiv2DataStrToQStr(it->toString()));

        if (auto it = exif_data.findKey(make); it != exif_data.end() && it->count() > 0)
            m_exif_tags.insert(QObject::tr("Make"), exiv2DataStrToQStr(it->toString()));

        if (auto it = exif_data.findKey(model); it != exif_data.end() && it->count() > 0)
            m_exif_tags.insert(QObject::tr("Model"), exiv2DataStrToQStr(it->toString()));

        if (auto it = exif_data.findKey(dateTime); it != exif_data.end() && it->count() > 0)
            m_exif_tags.insert(QObject::tr("Date Time"), exiv2DataStrToQStr(it->toString()));

        if (auto it = exif_data.findKey(exposureTime); it != exif_data.end() && it->count() > 0)
        {
            Exiv2::Rational r = it->toRational();
            if (r.first < r.second)
            {
                qreal exp = round(static_cast<qreal>(r.second) / r.first);
                m_exif_tags.insert(QObject::tr("Exposure Time"), "1/" + QString::number(exp) + QObject::tr(" sec"));
            }
            else
            {
                qreal exp = round(static_cast<qreal>(r.first) / r.second);
                m_exif_tags.insert(QObject::tr("Exposure Time"), QString::number(exp) + QObject::tr(" sec"));
            }
        }

        if (auto it = exif_data.findKey(fnumber); it != exif_data.end() && it->count() > 0)
        {
            Exiv2::Rational r = it->toRational();
            qreal fn = static_cast<qreal>(r.first) / r.second;
            m_exif_tags.insert(QObject::tr("F Number"), "f/" + QString::number(fn, 'g', 3));
        }

        if (auto it = exif_data.findKey(isoSpeedRatings); it != exif_data.end() && it->count() > 0)
            m_exif_tags.insert(QObject::tr("ISO Speed Ratings"), exiv2DataStrToQStr(it->toString()));

        if (auto it = exif_data.findKey(flash); it != exif_data.end() && it->count() > 0)
            m_exif_tags.insert(QObject::tr("Flash"), exiv2DataStrToQStr(it->toString()));

        if (auto it = exif_data.findKey(focalLength); it != exif_data.end() && it->count() > 0)
        {
            Exiv2::Rational r = it->toRational();
            qreal fn = static_cast<qreal>(r.first) / r.second;
            m_exif_tags.insert(QObject::tr("Focal Length"), QString::number(fn, 'g', 3) + QObject::tr(" mm"));
        }

        if (auto it = exif_data.findKey(userComment); it != exif_data.end() && it->count() > 0)
            m_exif_tags.insert(QObject::tr("User Comment"), exiv2DataStrToQStr(it->toString()));
    }
    catch (Exiv2::Error const& e)
    {
        qInfo() << "[Exiv2] failed to load exif tags due to:" << e.what();
    }
}

std::optional<QString> MetaData::getExifTag(QString const& key) const
{
    if (auto it = m_exif_tags.find(key); it != m_exif_tags.end()) return it.value();
    return {};
}

QMap<QString, QString> const& MetaData::getExifTags() const
{
    return m_exif_tags;
}

QString MetaData::exiv2DataStrToQStr(std::string const& str)
{
    auto qstr = QString::fromStdString(str);
    if (qstr.startsWith("charset=", Qt::CaseInsensitive)) qstr.remove(0, qstr.indexOf(" ") + 1);
    return qstr;
}

void MetaData::refresh()
{
    m_file_info.refresh();
}