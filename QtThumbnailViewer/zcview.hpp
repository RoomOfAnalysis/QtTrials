#pragma once

#include <QChartView>

class ZCView: public QChartView
{
    Q_OBJECT
public:
    enum class ZoomMode
    {
        Horizontal,
        Vertical,
        Rectangle,
        Pan
    };

    ZCView(QWidget* parent = nullptr);
    ZCView(QChart* chart, QWidget* parent = nullptr);
    ~ZCView();

    ZoomMode zoomMode() const;
    void setZoomMode(ZoomMode zoomMode);

    void zoomX(qreal factor, qreal xcenter);
    void zoomX(qreal factor);

    void zoomY(qreal factor, qreal ycenter);
    void zoomY(qreal factor);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    static bool isAxisTypeZoomableWithMouse(const QAbstractAxis::AxisType type);
    QPointF chartCoord2SeriesCoord(const QPointF& mousePos, QAbstractSeries* series) const;
    QPointF seriesCoord2ChartCoord(const QPointF& seriesPos, QAbstractSeries* series) const;

private:
    QPointF m_last_mouse_pos;
    ZoomMode m_zoomMode = ZoomMode::Pan;
};