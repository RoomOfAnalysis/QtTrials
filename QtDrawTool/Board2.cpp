#include "Board2.h"

#include <QGraphicsView>
#include <QFileDialog>
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>
#include <QVBoxLayout>
#include <QUndoView>

Board::DrawShape::DrawShape(QGraphicsScene* scene, Board::Shape shape, QPoint start_pt, QPoint end_pt, QPen* pen,
                            QBrush* brush)
    : m_scene(scene)
{
    auto width = end_pt.x() - start_pt.x();
    auto height = end_pt.y() - start_pt.y();

    QPen p;
    if (pen) p = *pen;
    QBrush b;
    if (brush) b = *brush;

    switch (shape)
    {
    case Shape::RECT:
        m_item = m_scene->addRect(start_pt.x(), start_pt.y(), width, height > 0 ? width : -width, p, b);
        break;
    case Shape::CIRCLE:
        // FIXME: incorrect position due to it is under [item coordinates](https://doc.qt.io/qt-6/qgraphicsscene.html#addEllipse)
        // use [this](https://doc.qt.io/qt-6/qgraphicsitem.html#mapFromScene)?
        m_item = m_scene->addEllipse(start_pt.x(), start_pt.y(), width, height > 0 ? width : -width, p, b);
        break;
    default:
        qDebug() << "Shape not supported";
        break;
    }

    setText(QString("Shape: %1, Start: %2, End: %3")
                .arg(static_cast<int>(shape))
                .arg(QString("(%1, %2)").arg(start_pt.x()).arg(start_pt.y()))
                .arg(QString("(%1, %2)").arg(end_pt.x()).arg(end_pt.y())));
}

void Board::DrawShape::undo()
{
    m_scene->removeItem(m_item);
    m_scene->update();
}

void Board::DrawShape::redo()
{
    m_scene->addItem(m_item);
    m_scene->update();
}

Board::Board(QWidget* parent): QWidget(parent)
{
    m_scene = new QGraphicsScene(this);
    m_undo_stack = new QUndoStack(this);

    auto* view = new QGraphicsView(m_scene);
    view->setAttribute(Qt::WA_TransparentForMouseEvents);
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addWidget(view);

    auto* undo_view = new QUndoView(m_undo_stack);
    QHBoxLayout* pHLayout = new QHBoxLayout();
    pHLayout->addLayout(pLayout);
    pHLayout->addWidget(undo_view);

    this->setLayout(pHLayout);

    setMouseTracking(true);
}

void Board::saveImg()
{
    QString filename =
        QFileDialog::getSaveFileName(this, tr("Save Image"), "", tr("*;;*.bmp;; *.png;; *.jpg;; *.tif;; *.GIF"));
    if (filename.isEmpty())
        return;
    else
        grab().toImage().save(filename);
}

void Board::openImg()
{
    QString filename =
        QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("*;;*.bmp;; *.png;; *.jpg;; *.tif;; *.GIF"));
    if (filename.isEmpty())
        return;
    else
    {
        auto* item = new QGraphicsPixmapItem();
        item->setPixmap(QPixmap(filename));
        m_scene->addItem(item);
    }
}

void Board::undo()
{
    m_undo_stack->undo();
}

void Board::redo()
{
    m_undo_stack->redo();
}

void Board::reset()
{
    m_undo_stack->clear();

    update();
}

void Board::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) m_start_pt = event->pos();
}

void Board::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_end_pt = event->pos();

        m_undo_stack->push(new DrawShape(m_scene, m_shape, m_start_pt, m_end_pt, &m_pen, &m_brush));
        qDebug() << "draw";
    }
}
