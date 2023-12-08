#pragma once

#include <QWidget>
#include <QPen>
#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>

class Board: public QWidget
{
    Q_OBJECT
public:
    enum class Shape
    {
        RECT,
        CIRCLE,
        ELLIPSE
    };

private:
    class DrawShape: public QUndoCommand
    {
    public:
        DrawShape(QGraphicsScene* scene, Shape shape, QPoint start_pt, QPoint end_pt, QPen* pen = nullptr,
                  QBrush* brush = nullptr);
        void undo() override;
        void redo() override;

    private:
        QGraphicsScene* m_scene = nullptr;
        QGraphicsItem* m_item = nullptr;
    };

public:
    explicit Board(QWidget* parent = nullptr);
    ~Board() = default;

    Shape& shape() { return m_shape; }
    QPen* pen() { return &m_pen; }
    QBrush* brush() { return &m_brush; }

    void saveImg();
    void openImg();

    void undo();
    void redo();

    void reset();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QGraphicsScene* m_scene = nullptr;
    QUndoStack* m_undo_stack = nullptr;

    Shape m_shape{Shape::CIRCLE};
    QPen m_pen{Qt::red, 5, Qt::SolidLine};
    QBrush m_brush{Qt::transparent};

    QPoint m_start_pt{}, m_end_pt{};
};