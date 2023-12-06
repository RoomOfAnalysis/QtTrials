#include "QtDisplayBoard.h"

#include <QGraphicsView>
#include <QGridLayout>

QtDisplayBoard::QtDisplayBoard(QWidget* parent): QWidget(parent)
{
    m_video_item = new QGraphicsVideoItem();
    m_video_item->setFlag(QGraphicsItem::ItemStacksBehindParent);
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setStyleSheet("background: transparent");
    m_scene->addItem(m_video_item);

    auto* layout = new QGridLayout(this);
    layout->addWidget(m_view);
}

QtDisplayBoard::~QtDisplayBoard() {}

QSize QtDisplayBoard::sizeHint() const
{
    return m_video_item->size().toSize();
}

void QtDisplayBoard::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    m_view->fitInView(m_video_item, Qt::KeepAspectRatio);
}

QGraphicsVideoItem* QtDisplayBoard::getVideoItem() const
{
    return m_video_item;
}

QGraphicsScene* QtDisplayBoard::getScene() const
{
    return m_scene;
}