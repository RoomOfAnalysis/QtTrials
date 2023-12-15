#pragma once

#include <QLabel>
#include <QPen>
#include <QUndoCommand>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsView>

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
        QPen m_pen{};
        QBrush m_brush{};
    };

    class BoardGraphicsView: public QGraphicsView
    {
    public:
        explicit BoardGraphicsView(QWidget* parent = nullptr);

        void undo();
        void redo();
        void reset();

        QUndoStack* undoStack() const { return m_undo_stack; }

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    public:
        Board::Shape m_shape{Board::Shape::CIRCLE};
        QPen m_pen{Qt::red, 5, Qt::SolidLine};
        QBrush m_brush{Qt::transparent};

    private:
        QGraphicsScene* m_scene = nullptr;
        QUndoStack* m_undo_stack = nullptr;
        QPoint m_start_pt{}, m_end_pt{};
    };

public:
    explicit Board(QWidget* parent = nullptr);
    ~Board() = default;

    Shape& shape() { return m_view->m_shape; }
    QPen* pen() { return &m_view->m_pen; }
    QBrush* brush() { return &m_view->m_brush; }

    void saveImg();
    void openImg();

    void undo() { m_view->undo(); }
    void redo() { m_view->redo(); }
    void reset() { m_view->reset(); }

private:
    QGraphicsScene* m_scene = nullptr;
    BoardGraphicsView* m_view = nullptr;
};