#include "QtOverlay.h"

#include <QPainter>
#include <QEvent>

QtOverlayFactoryFilter::QtOverlay::QtOverlay(QWidget* parent): QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void QtOverlayFactoryFilter::QtOverlay::setPaintFunc(std::function<void(QPainter*)> func)
{
    m_paint_func = func;
}

void QtOverlayFactoryFilter::QtOverlay::paintEvent(QPaintEvent*)
{
    auto painter = QPainter(this);
    if (m_paint_func) m_paint_func(&painter);
}

QtOverlayFactoryFilter::QtOverlayFactoryFilter(std::function<void(QPainter*)> paint_func, QObject* parent)
    : QObject(parent), m_paint_func(paint_func)
{
}

bool QtOverlayFactoryFilter::eventFilter(QObject* obj, QEvent* ev)
{
    if (!obj->isWidgetType()) return false;
    if (auto w = static_cast<QWidget*>(obj); ev->type() == QEvent::MouseButtonPress)
    {
        if (!m_overlay) m_overlay = new QtOverlay(w);
        m_overlay->setPaintFunc(m_paint_func);
        m_overlay->resize(w->size());
        m_overlay->show();
        qDebug() << "painted";
    }
    else if (ev->type() == QEvent::Resize && m_overlay && m_overlay->parentWidget() == w)
        m_overlay->resize(w->size());
    return false;
}