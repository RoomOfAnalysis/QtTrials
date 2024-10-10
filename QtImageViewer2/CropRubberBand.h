#pragma once

#include <QWidget>

class QRubberBand;

class CropRubberBand: public QWidget
{
    Q_OBJECT
public:
    CropRubberBand(QWidget* parent = nullptr);
    ~CropRubberBand();

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