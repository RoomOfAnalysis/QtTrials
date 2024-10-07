#include "Histogram.h"

#include <QImageReader>

Histogram CalcHist(QImage const& img)
{
    Histogram hist;
    if (img.isNull())
    {
        qWarning() << "Invalid file";
        return hist;
    }
    const QImage image = img.scaled(256, 256).convertToFormat(QImage::Format_RGB888);
    for (auto y = 0; y < image.height(); y++)
    {
        auto const* line = image.scanLine(y);
        for (auto x = 0; x < image.width(); x++)
        {
            auto const index = x * 3;
            hist.red[line[index + 0]] += 1.f;
            hist.green[line[index + 1]] += 1.f;
            hist.blue[line[index + 2]] += 1.f;
        }
    }
    return hist;
}

Histogram CalcHist(const QString& filePath)
{
    QImageReader reader(filePath);
    reader.setScaledSize(QSize(256, 256));
    reader.setAutoTransform(false);
    QImage image = reader.read();
    if (image.isNull())
    {
        qWarning() << "Invalid file" << filePath << reader.errorString();
        return {};
    }
    return CalcHist(image);
}