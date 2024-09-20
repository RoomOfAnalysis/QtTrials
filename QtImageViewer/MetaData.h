#pragma once

#include <exiv2/exiv2.hpp>

#include <QFileInfo>
#include <QMimeType>
#include <QString>
#include <QStringList>
#include <QMap>

class MetaData
{
public:
    MetaData(QString const& file_path);
    ~MetaData() = default;

    QFileInfo const& fileInfo() const;
    QMimeType const& mimeType() const;
    int orientation() const;

    void loadExifTags(bool force_reload = false);
    std::optional<QString> getExifTag(QString const& key) const;
    QMap<QString, QString> const& getExifTags() const;

    void refresh();

private:
    void load_mime();
    void load_exif_image();
    void load_orientation();

    static QString exiv2DataStrToQStr(std::string const& str);

private:
    QFileInfo m_file_info;
    QMimeType m_mime_type;
    std::unique_ptr<Exiv2::Image> m_exif_img;
    QMap<QString, QString> m_exif_tags;
    int m_orientation{-1};
};