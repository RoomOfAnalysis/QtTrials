#pragma once

#include <stack>

#include <QWidget>
#include <QPen>

class Board : public QWidget
{
	Q_OBJECT
public:
    enum class Shape
    {
        NONE,
        RECT,
        CIRCLE,
        ELLIPSE,
        POLYGON
    };

public:
    explicit Board(QWidget* parent = nullptr);
    ~Board() = default;

    QPen& pen() { return m_pen; }
    QBrush& brush() { return m_brush; }
    Shape& shape() { return m_shape; }

    void saveImg();
    void openImg();

    void undo();
    void redo();

    void reset();

protected:
    void paint(QImage* img);

    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QImage m_img{QImage(600, 600, QImage::Format_RGB32)};
    QImage m_tmp_img{};
    QPen m_pen{Qt::black, 1, Qt::SolidLine};
    QBrush m_brush{Qt::transparent};
    Shape m_shape{Shape::NONE};
    QPoint m_start_pt{}, m_end_pt{};
    bool m_drawing{false}, m_mouse_pressing{false};
    std::stack<QImage> m_undos{}, m_redos{};
};