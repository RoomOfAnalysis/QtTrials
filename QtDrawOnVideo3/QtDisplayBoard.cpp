#include "QtDisplayBoard.h"

#include <QGraphicsView>
#include <QGridLayout>

QtDisplayBoard::QtDisplayBoard(QWidget* parent): QWidget(parent)
{
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setStyleSheet("background: transparent");
    m_pixmap_item = m_scene->addPixmap(QPixmap());

    auto* layout = new QGridLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins({0, 0, 0, 0});
    layout->setSpacing(0);
    layout->addWidget(m_view);
}

QtDisplayBoard::~QtDisplayBoard() {}

QSize QtDisplayBoard::sizeHint() const
{
    return QSize(1920, 1080);
}

void QtDisplayBoard::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_view->fitInView(m_pixmap_item, Qt::KeepAspectRatio);
    m_view->centerOn(m_pixmap_item);
}

QGraphicsPixmapItem* QtDisplayBoard::getPixmapItem() const
{
    return m_pixmap_item;
}

QGraphicsScene* QtDisplayBoard::getScene() const
{
    return m_scene;
}