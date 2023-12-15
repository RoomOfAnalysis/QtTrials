#include "Board2.h"

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
    // incorrect position due to it is under [item coordinates](https://doc.qt.io/qt-6/qgraphicsscene.html#addEllipse)
    // need convert it
    auto view = m_scene->views()[0];
    auto s_pt = view->mapToScene(start_pt);
    auto e_pt = view->mapToScene(end_pt);

    if (pen) m_pen = *pen;
    if (brush) m_brush = *brush;

    switch (shape)
    {
    case Shape::RECT:
        {
            auto tmp_item = new QGraphicsRectItem(QRectF(s_pt, e_pt));
            tmp_item->setPen(m_pen);
            tmp_item->setBrush(m_brush);
            m_item = tmp_item;
            break;
        }
    case Shape::CIRCLE:
        {
            // center + radius
            auto radius = std::sqrt(QPointF::dotProduct(e_pt - s_pt, e_pt - s_pt));
            //s_pt.setX(s_pt.x() - radius);
            //s_pt.setY(s_pt.y() - radius);
            //auto tmp_item = new QGraphicsEllipseItem(s_pt.x(), s_pt.y(), radius * 2, radius * 2);

            // fix circle deform at first drawing...
            // FIXME: it looks like a circle but radius is incorrect...
            auto t = view->transform();
            t /= t.determinant();
            qDebug() << t;
            auto radius_x = radius / t.m11();
            auto radius_y = radius / t.m22();
            s_pt.setX(s_pt.x() - radius_x);
            s_pt.setY(s_pt.y() - radius_y);
            auto tmp_item = new QGraphicsEllipseItem(s_pt.x(), s_pt.y(), radius_x * 2, radius_y * 2);

            tmp_item->setPen(m_pen);
            tmp_item->setBrush(m_brush);
            m_item = tmp_item;
            break;
        }
    case Shape::ELLIPSE:
        {
            auto tmp_item = new QGraphicsEllipseItem(QRectF(s_pt, e_pt));
            tmp_item->setPen(m_pen);
            tmp_item->setBrush(m_brush);
            m_item = tmp_item;
            break;
        }
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

Board::BoardGraphicsView::BoardGraphicsView(QWidget* parent): QGraphicsView(parent)
{
    m_scene = new QGraphicsScene();
    setSceneRect(rect());
    setScene(m_scene);
    m_undo_stack = new QUndoStack(this);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setStyleSheet("background-color: transparent;");
    setMouseTracking(true);
}

void Board::BoardGraphicsView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) m_start_pt = event->pos();
}

void Board::BoardGraphicsView::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_end_pt = event->pos();
        m_undo_stack->push(new DrawShape(m_scene, m_shape, m_start_pt, m_end_pt, &m_pen, &m_brush));
    }
}

void Board::BoardGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);

    auto sz = sceneRect().size();
    auto n_sz = event->size();
    qreal scale_x = n_sz.width() / (qreal)sz.width();   // m11
    qreal scale_y = n_sz.height() / (qreal)sz.height(); // m22
    //auto t = transform();
    //setTransform(QTransform(scale_x, t.m12(), t.m13(), t.m21(), scale_y, t.m23(), t.m31(), t.m32(), t.m33()));
    setTransform(QTransform::fromScale(scale_x, scale_y));
}

void Board::BoardGraphicsView::undo()
{
    m_undo_stack->undo();
}

void Board::BoardGraphicsView::redo()
{
    m_undo_stack->redo();
}

void Board::BoardGraphicsView::reset()
{
    m_scene->clear();
    m_undo_stack->clear();

    m_scene->update();
}

Board::Board(QWidget* parent): QWidget(parent)
{
    m_view = new BoardGraphicsView();
    m_scene = m_view->scene();
    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addWidget(m_view);

    auto* undo_view = new QUndoView(m_view->undoStack());
    QHBoxLayout* pHLayout = new QHBoxLayout();
    pHLayout->addLayout(pLayout);
    pHLayout->addWidget(undo_view);
    pHLayout->setStretch(0, 1);
    pHLayout->setStretch(1, 1);

    this->setLayout(pHLayout);
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
        m_view->setStyleSheet(QString("border-image: url(%1) 0 0 0 0 stretch stretch;").arg(filename));
}
