#include "QtAspectWidget.h"

#include <QResizeEvent>

QtAspectWidget::QtAspectWidget(QWidget* parent, double aspect_ratio): QWidget(parent), m_aspect_ratio(aspect_ratio) {}

QtAspectWidget::~QtAspectWidget() = default;

void QtAspectWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    auto [width, height] = event->size();
    auto width_n = std::min(static_cast<double>(width), height * m_aspect_ratio);
    auto height_n = std::min(static_cast<double>(height), width / m_aspect_ratio);
    auto hm = round(width - width_n) / 2;
    auto vm = round(height - height_n) / 2;
    setContentsMargins(hm, vm, hm, vm);
}