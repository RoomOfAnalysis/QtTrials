#include "ImageOps.h"

QImage flipH(QImage in)
{
    return in.mirrored(true, false);
}

QImage flipV(QImage in)
{
    return in.mirrored(false, true);
}

QImage rotated(QImage in, qreal degree)
{
    QTransform transform;
    transform.rotate(degree);
    return in.transformed(transform, Qt::SmoothTransformation);
}

QImage exifOriented(QImage in, int orientation)
{
    //// Qt Image Transformation
    //switch (orientation)
    //{
    //case 1:
    //    return flipH(in);
    //case 2:
    //    return flipV(in);
    //case 3:
    //    return flipV(flipH(in));
    //case 4:
    //    return rotated(in, 90);
    //case 5:
    //    return rotated(flipH(in), 90);
    //case 6:
    //    return rotated(flipV(in), 90);
    //case 7:
    //    return rotated(in, -90);
    //default:
    //    return in;
    //}

    // exif orientation
    switch (orientation)
    {
    case 1:
        return in;
    case 2:
        return flipH(in);
    case 3:
        return rotated(in, 180);
    case 4:
        return flipV(in);
    case 5:
        return flipH(rotated(in, 90));
    case 6:
        return rotated(in, 90);
    case 7:
        return flipV(rotated(in, 90));
    case 8:
        return rotated(in, 270);
    default:
        return in;
    }
}

QImage scaled(QImage in, QSize sz)
{
    return in.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

QImage cropped(QImage in, QRect rect)
{
    return !in.isNull() && in.rect().contains(rect, false) ? in.copy(rect) : QImage();
}