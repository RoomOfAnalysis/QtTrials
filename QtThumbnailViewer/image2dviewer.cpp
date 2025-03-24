#include "image2dviewer.hpp"

#include <QPainter>
#include <QWheelEvent>
#include <QScrollArea>
#include <QLayout>
#include <QTimer>
#include <QScrollBar>
#include <QApplication>
#include <QClipBoard>
#include <QFileDialog>
#include <QMenu>

class Image2DViewer::ImageWidget: public QWidget
{
public:
    explicit ImageWidget(QWidget* parent = nullptr): QWidget(parent) {}
    ~ImageWidget() = default;
    bool empty() const { return m_image.isNull(); }
    QImage image() const { return m_image; }
    void setImage(const QImage& i)
    {
        m_image = i;
        update();
    }
    QPoint mapToImage(QPoint p)
    {
        QPoint upperLeft;
        QPoint center(width() / 2, height() / 2);
        if (width() > m_image.width()) upperLeft.setX(center.x() - m_image.width() / 2);
        if (height() > m_image.height()) upperLeft.setY(center.y() - m_image.height() / 2);
        return QPoint(p.x() - upperLeft.x(), p.y() - upperLeft.y());
    }
    QSize imageSize() const { return m_image.size(); }

protected:
    QSize sizeHint() const override { return m_image.size(); }
    void paintEvent(QPaintEvent* ev) override
    {
        float scale = qMax(float(width()) / m_image.width(), float(height()) / m_image.height());

        QPainter painter(this);

        const float sx = qMax(-x() / scale, 0.f);
        const float sy = qMax(-y() / scale, 0.f);
        const float sw = qMin<float>(width() / scale, m_image.width());
        const float sh = qMin<float>(height() / scale, m_image.height());

        QRectF sourceRect(sx, sy, sw, sh);
        QRectF targetRect = rect();
        targetRect &= ev->rect();
        sourceRect &= QRectF(ev->rect().x() / scale, ev->rect().y() / scale, ev->rect().width() / scale,
                             ev->rect().height() / scale);
        painter.drawImage(targetRect, m_image, sourceRect);
        return;
    }

private:
    QImage m_image{};
};

class MyScrollArea: public QScrollArea
{
protected:
    void wheelEvent(QWheelEvent* event) override
    {
        event->ignore();
        return;
    }
};

Image2DViewer::Image2DViewer(QWidget* parent): QWidget(parent)
{
    m_cursor_is_hidden = false;
    m_move_image_locked = false;
    m_image_widget = new ImageWidget;

    m_scroll_area = new MyScrollArea;
    m_scroll_area->setContentsMargins(0, 0, 0, 0);
    m_scroll_area->setAlignment(Qt::AlignCenter);
    m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scroll_area->setFrameStyle(0);
    m_scroll_area->setWidget(m_image_widget);
    m_scroll_area->setWidgetResizable(false);

    QVBoxLayout* scrollLayout = new QVBoxLayout;
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(0);
    scrollLayout->addWidget(m_scroll_area);
    this->setLayout(scrollLayout);

    m_mouse_movement_timer = new QTimer(this);
    connect(m_mouse_movement_timer, SIGNAL(timeout()), this, SLOT(monitorCursorState()));
}

Image2DViewer::~Image2DViewer() = default;

void Image2DViewer::loadImage(QImage image)
{
    m_temp_disable_resize = false;
    m_image_zoom_factor = 1.f;
    m_orig_image = image;

    QApplication::processEvents();
    reload();
    imageChanged(image);
}

void Image2DViewer::resizeImage()
{
    static bool busy = false;
    if (busy) return;
    QSize imageSize;
    if (m_image_widget)
        imageSize = m_image_widget->imageSize();
    else
        return;
    if (imageSize.isEmpty()) return;

    busy = true;

    int imageViewWidth = this->size().width();
    int imageViewHeight = this->size().height();

    float positionY =
        m_scroll_area->verticalScrollBar()->value() > 0 ?
            m_scroll_area->verticalScrollBar()->value() / float(m_scroll_area->verticalScrollBar()->maximum()) :
            0;
    float positionX =
        m_scroll_area->horizontalScrollBar()->value() > 0 ?
            m_scroll_area->horizontalScrollBar()->value() / float(m_scroll_area->horizontalScrollBar()->maximum()) :
            0;

    auto calcZoom = [this](int size) { return size * m_image_zoom_factor; };

    if (m_temp_disable_resize)
        imageSize.scale(imageSize.width(), imageSize.height(), Qt::KeepAspectRatio);
    else
        imageSize.scale(calcZoom(imageViewWidth), calcZoom(imageViewHeight), Qt::KeepAspectRatio);

    QPointF newPosition = m_scroll_area->widget()->pos();
    m_scroll_area->widget()->setFixedSize(imageSize);
    m_scroll_area->widget()->adjustSize();
    if (newPosition.isNull() || imageSize.width() < width() + 100 || imageSize.height() < height() + 100)
        centerImage(imageSize);
    else
    {
        m_scroll_area->horizontalScrollBar()->setValue(m_scroll_area->horizontalScrollBar()->maximum() * positionX);
        m_scroll_area->verticalScrollBar()->setValue(m_scroll_area->verticalScrollBar()->maximum() * positionY);
    }
    busy = false;
}

void Image2DViewer::setCursorHiding(bool hide)
{
    if (hide)
        m_mouse_movement_timer->start(500);
    else
    {
        m_mouse_movement_timer->stop();
        if (m_cursor_is_hidden)
        {
            QApplication::restoreOverrideCursor();
            m_cursor_is_hidden = false;
        }
    }
}

void Image2DViewer::refresh()
{
    if (!m_image_widget) return;

    m_viewer_image = m_orig_image;

    transform();

    setImage(m_viewer_image);
    resizeImage();
}

void Image2DViewer::reload()
{
    m_viewer_image = m_orig_image;

    setImage(m_viewer_image);
    resizeImage();
}

void Image2DViewer::flipH()
{
    m_flipH = true;
    transform();
    setImage(m_viewer_image);
    resizeImage();
}

void Image2DViewer::flipV()
{
    m_flipV = true;
    transform();
    setImage(m_viewer_image);
    resizeImage();
}

QImage Image2DViewer::getOriginImage() const
{
    return m_orig_image;
}

QImage Image2DViewer::getViewerImage() const
{
    return m_viewer_image;
}

void Image2DViewer::saveImage()
{
    setCursorHiding(false);

    QString path = QFileDialog::getSaveFileName(this, tr("Save image as"), QDir::homePath(),
                                                tr("Images") + " (*.jpg *.jpeg *.png *.bmp *.tif *.webp)");
    if (!path.isEmpty() && m_viewer_image.save(path)) emit imageSaved(path);
}

void Image2DViewer::copyImage()
{
    QApplication::clipboard()->setImage(m_viewer_image);
}

void Image2DViewer::pasteImage()
{
    if (!m_image_widget) return;

    if (!QApplication::clipboard()->image().isNull())
    {
        m_orig_image = QApplication::clipboard()->image();
        refresh();
    }
}

void Image2DViewer::monitorCursorState()
{
    static QPoint lastPos;

    if (QCursor::pos() != lastPos)
    {
        lastPos = QCursor::pos();
        if (m_cursor_is_hidden)
        {
            QApplication::restoreOverrideCursor();
            m_cursor_is_hidden = false;
        }
    }
    else if (!m_cursor_is_hidden)
    {
        QApplication::setOverrideCursor(Qt::BlankCursor);
        m_cursor_is_hidden = true;
    }
}

void Image2DViewer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    resizeImage();
}

void Image2DViewer::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    resizeImage();
}

void Image2DViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_image_widget) return;
    // only accept left button drag move
    if (!(event->buttons() & Qt::LeftButton)) return;

    if (m_move_image_locked)
    {
        int newX = m_layoutX + (event->pos().x() - m_mouseX);
        int newY = m_layoutY + (event->pos().y() - m_mouseY);
        bool needToMove = false;

        if (m_image_widget->size().width() > size().width())
        {
            if (newX > 0)
                newX = 0;
            else if (newX < (size().width() - m_image_widget->size().width()))
                newX = (size().width() - m_image_widget->size().width());
            needToMove = true;
        }
        else
        {
            newX = m_layoutX;
        }

        if (m_image_widget->size().height() > size().height())
        {
            if (newY > 0)
                newY = 0;
            else if (newY < (size().height() - m_image_widget->size().height()))
                newY = (size().height() - m_image_widget->size().height());
            needToMove = true;
        }
        else
        {
            newY = m_layoutY;
        }

        if (needToMove)
        {
            m_scroll_area->horizontalScrollBar()->setValue(-newX);
            m_scroll_area->verticalScrollBar()->setValue(-newY);
        }
    }
}

void Image2DViewer::contextMenuEvent(QContextMenuEvent* event)
{
    while (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
    if (!m_context_menu)
    {
        m_context_menu = new QMenu(tr("Context Menu"), this);
        auto action = new QAction(tr("Save Image As"), this);
        action->setIcon(QIcon(":/ImageTool/UI_Icons/save_as.png"));
        connect(action, &QAction::triggered, this, &Image2DViewer::saveImage);
        m_context_menu->addAction(action);
    }
    m_context_menu->exec(QCursor::pos());
}

void Image2DViewer::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);
    while (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
}

void Image2DViewer::mousePressEvent(QMouseEvent* event)
{
    if (!m_image_widget) return;

    if (event->button() == Qt::LeftButton)
    {
        auto pos = event->position();
        setMouseMoveData(true, qRound(pos.x()), qRound(pos.y()));
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    QWidget::mousePressEvent(event);
}

void Image2DViewer::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        setMouseMoveData(false, 0, 0);
        while (QApplication::overrideCursor())
            QApplication::restoreOverrideCursor();
    }
    QWidget::mouseReleaseEvent(event);
}

void Image2DViewer::wheelEvent(QWheelEvent* event)
{
    if (!m_temp_disable_resize)
    {
        int angleDelta = event->angleDelta().ry();
        if (angleDelta > 0)
            m_image_zoom_factor += 0.1f;
        else if (angleDelta < 0)
            m_image_zoom_factor -= 0.1f;
        resizeImage();
        event->accept();
    }
}

void Image2DViewer::setMouseMoveData(bool lockMove, int lMouseX, int lMouseY)
{
    if (!m_image_widget) return;
    m_move_image_locked = lockMove;
    m_mouseX = lMouseX;
    m_mouseY = lMouseY;
    m_layoutX = m_image_widget->pos().x();
    m_layoutY = m_image_widget->pos().y();
}

void Image2DViewer::centerImage(QSize& imgSize)
{
    m_scroll_area->ensureVisible(imgSize.width() / 2, imgSize.height() / 2, width() / 2, height() / 2);
}

void Image2DViewer::transform()
{
    if (m_flipH || m_flipV) m_viewer_image = m_viewer_image.mirrored(m_flipH, m_flipV);
}

void Image2DViewer::setImage(const QImage& image)
{
    m_image_widget->setImage(image);
}
