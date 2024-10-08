#pragma once

#include "QtOverlayWidget.h"

class QRubberBand;

class QtCropRubberBandOverlay: public QtOverlayWidget
{
    Q_OBJECT
public:
    QtCropRubberBandOverlay(QtOverlayWidgetContainer* parent = nullptr);
    ~QtCropRubberBandOverlay();

signals:
    void selectionChanged(QRect selection);

public slots:
    void show();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QRubberBand* m_rubberband = nullptr;
    QPoint m_last_mouse_pos{};
};