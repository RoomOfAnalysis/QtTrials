#pragma once

#include <QImage>

QImage flipH(QImage in);
QImage flipV(QImage in);
QImage rotated(QImage in, qreal degree);
QImage exifOriented(QImage in, int orientation);
QImage scaled(QImage in, QSize sz);
QImage cropped(QImage in, QRect rect);
