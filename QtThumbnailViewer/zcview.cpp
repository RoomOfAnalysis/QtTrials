#include "zcview.hpp"

#include <QGuiApplication>
#include <QValueAxis>

ZCView::ZCView(QWidget* parent): QChartView(parent)
{
    setRubberBand(QChartView::RubberBand::NoRubberBand);
    setDragMode(QChartView::ScrollHandDrag);
}

ZCView::ZCView(QChart* chart, QWidget* parent): QChartView(chart, parent)
{
    setRubberBand(QChartView::RubberBand::NoRubberBand);
    setDragMode(QChartView::ScrollHandDrag);
}

ZCView::~ZCView() = default;

ZCView::ZoomMode ZCView::zoomMode() const
{
    return m_zoomMode;
}

void ZCView::setZoomMode(ZoomMode zoomMode)
{
    if (m_zoomMode != zoomMode)
    {
        m_zoomMode = zoomMode;
        switch (zoomMode)
        {
        case ZoomMode::Horizontal:
            setRubberBand(QChartView::HorizontalRubberBand);
            setDragMode(QChartView::NoDrag);
            break;
        case ZoomMode::Vertical:
            setRubberBand(QChartView::VerticalRubberBand);
            setDragMode(QChartView::NoDrag);
            break;
        case ZoomMode::Rectangle:
            setRubberBand(QChartView::RectangleRubberBand);
            setDragMode(QChartView::NoDrag);
            break;
        case ZoomMode::Pan:
            setRubberBand(QChartView::NoRubberBand);
            setDragMode(QChartView::ScrollHandDrag);
            break;
        default:
            break;
        }
    }
}

void ZCView::zoomX(qreal factor, qreal xcenter)
{
    auto rect = chart()->plotArea();
    rect.setWidth(rect.width() / factor);
    rect.moveLeft(rect.x() + xcenter * (1. - 1. / factor));
    chart()->zoomIn(rect);
}

void ZCView::zoomX(qreal factor)
{
    auto rect = chart()->plotArea();
    auto c = rect.center();
    rect.setWidth(rect.width() / factor);
    rect.moveCenter(c);
    chart()->zoomIn(rect);
}

void ZCView::zoomY(qreal factor, qreal ycenter)
{
    auto rect = chart()->plotArea();
    rect.setHeight(rect.height() / factor);
    rect.moveTop(rect.x() + ycenter * (1. - 1. / factor));
    chart()->zoomIn(rect);
}

void ZCView::zoomY(qreal factor)
{
    auto rect = chart()->plotArea();
    auto c = rect.center();
    rect.setHeight(rect.height() / factor);
    rect.moveCenter(c);
    chart()->zoomIn(rect);
}

void ZCView::mousePressEvent(QMouseEvent* event)
{
    m_last_mouse_pos = event->position();
    QChartView::mousePressEvent(event);
}

void ZCView::mouseMoveEvent(QMouseEvent* event)
{
    if (dragMode() == ScrollHandDrag)
    {
        if (event->buttons() & Qt::LeftButton)
        {
            bool move_horizon = false;
            for (const auto* axis : chart()->axes())
            {
                if (axis->orientation() == Qt::Horizontal && isAxisTypeZoomableWithMouse(axis->type()))
                {
                    move_horizon = true;
                    break;
                }
            }

            if (QGuiApplication::keyboardModifiers() & Qt::KeyboardModifier::ControlModifier)
                move_horizon = !move_horizon;

            if (move_horizon)
            {
                auto dx = -(event->position().x() - m_last_mouse_pos.x());
                chart()->scroll(dx, 0);
            }
            else
            {
                auto dy = event->pos().y() - m_last_mouse_pos.y();
                chart()->scroll(0, dy);
            }
        }
        m_last_mouse_pos = event->pos();
    }

    QChartView::mouseMoveEvent(event);
}

void ZCView::wheelEvent(QWheelEvent* event)
{
    bool zoom_horizon = false;
    for (auto axis : chart()->axes())
    {
        if (axis->orientation() == Qt::Horizontal && isAxisTypeZoomableWithMouse(axis->type()))
        {
            zoom_horizon = true;
            break;
        }
    }

    if (QGuiApplication::keyboardModifiers() & Qt::KeyboardModifier::ControlModifier) zoom_horizon = !zoom_horizon;

    if (zoom_horizon)
    {
        if (event->angleDelta().y() > 0)
            zoomX(2, event->position().x() - chart()->plotArea().x());
        else if (event->angleDelta().y() < 0)
            zoomX(0.5, event->position().x() - chart()->plotArea().x());
    }
    else
    {
        if (event->angleDelta().y() > 0)
            zoomY(2, event->position().y() - chart()->plotArea().y());
        else if (event->angleDelta().y() < 0)
            zoomY(0.5, event->position().y() - chart()->plotArea().y());
    }
}

bool ZCView::isAxisTypeZoomableWithMouse(const QAbstractAxis::AxisType type)
{
    return (type & QAbstractAxis::AxisTypeValue) | (type & QAbstractAxis::AxisTypeLogValue) |
           (type & QAbstractAxis::AxisTypeDateTime);
}

QPointF ZCView::chartCoord2SeriesCoord(const QPointF& mousePos, QAbstractSeries* series) const
{
    return chart()->mapToValue(chart()->mapFromScene(mousePos), series);
}

QPointF ZCView::seriesCoord2ChartCoord(const QPointF& seriesPos, QAbstractSeries* series) const
{
    return chart()->mapFromScene(chart()->mapToPosition(seriesPos, series));
}