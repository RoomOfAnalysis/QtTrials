#include "MagnificationGlass.h"

#include <QPainter>
#include <QPainterPath>

MagnificationGlass::MagnificationGlass(QWidget* parent): QLabel(parent)
{
    this->hide();

    return;
}

void MagnificationGlass::magnify(const QImage& fromImage, const QPoint& fromPosition, QSize scopeSize,
                                 double magnification, bool smooth)
{
    if (!m_active) return;

    QSize glassSize = QSize((scopeSize.width() * magnification), (scopeSize.height() * magnification));
    QPoint regionTopLeft =
        QPoint((fromPosition.x() - (scopeSize.width() / 2)), (fromPosition.y() - (scopeSize.height() / 2)));
    regionTopLeft.setX(std::max(0, std::min(regionTopLeft.x(), (fromImage.width() - scopeSize.width()))));
    regionTopLeft.setY(std::max(0, std::min(regionTopLeft.y(), (fromImage.height() - scopeSize.height()))));

    auto startIdx = (regionTopLeft.x() + regionTopLeft.y() * fromImage.width()) * (fromImage.depth() / 8);
    QImage magnified((fromImage.bits() + startIdx), scopeSize.width(), scopeSize.height(), fromImage.bytesPerLine(),
                     fromImage.format());
    auto transform_flag = smooth ? Qt::SmoothTransformation : Qt::FastTransformation;
    QPixmap pMap = QPixmap::fromImage(
        magnified.scaled(glassSize.width(), glassSize.height(), Qt::IgnoreAspectRatio, transform_flag));

    // TODO: expose style setter?
    QPixmap map(pMap.size());
    map.fill(Qt::transparent);
    QPainter painter(&map);
    QPainterPath path{};
    path.addEllipse(pMap.rect());
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, pMap);
    QPen pen{};
    pen.setStyle(Qt::SolidLine);
    pen.setColor(Qt::blue);
    pen.setWidth(5);
    painter.setPen(pen);
    painter.drawEllipse(pMap.rect());

    this->setPixmap(map);

    this->resize(glassSize);
    this->move(std::min((this->parentWidget()->width() - glassSize.width() + 3),
                        std::max(0, fromPosition.x() - (glassSize.width() / 2))),
               std::max(-3, std::min((this->parentWidget()->height() - glassSize.height()),
                                     fromPosition.y() - (glassSize.height() / 2))));
    this->show();
}
