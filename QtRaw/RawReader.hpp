#pragma once

#include <QIODevice>
#include <QImage>

class RawReader
{
public:
    RawReader();
    ~RawReader();

    bool setDevice(QIODevice* device);
    QImage read(QSize sz = {}, int quality = JPEG_DEFAULT_QUALITY);

    QSize thumbnailSize() const;
    QSize fullSize() const;

private:
    static constexpr int JPEG_HIGH_QUALITY_THRESHOLD = 50;
    static constexpr int JPEG_DEFAULT_QUALITY = 75;

    struct impl;
    std::unique_ptr<impl> d;
};
