#include "ImageViewer.h"
#include "CropRubberBand.h"

#include <QScrollArea>
#include <QWheelEvent>
#include <QLayout>
#include <QTimer>
#include <QApplication>
#include <QScrollBar>
#include <QMessageBox>
#include <QClipboard>

class MyScrollArea: public QScrollArea
{
protected:
    void wheelEvent(QWheelEvent* event) override
    {
        event->ignore();
        return;
    }
};

static unsigned int getHeightByWidth(int imgWidth, int imgHeight, int newWidth)
{
    float aspect;
    aspect = (float)imgWidth / (float)newWidth;
    return (imgHeight / aspect);
}

static unsigned int getWidthByHeight(int imgHeight, int imgWidth, int newHeight)
{
    float aspect;
    aspect = (float)imgHeight / (float)newHeight;
    return (imgWidth / aspect);
}

ImageViewer::ImageViewer(QWidget* parent): QWidget(parent)
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

ImageViewer::~ImageViewer() = default;

void ImageViewer::loadImage(QImage image)
{
    m_temp_disable_resize = false;
    m_image_zoom_factor = 1.f;
    m_orig_image = image;

    QApplication::processEvents();
    reload();
}

void ImageViewer::resizeImage()
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
    {
        switch (m_zoom_in_flags)
        {
        case ZoomMethods::Disable:
            if (imageSize.width() <= imageViewWidth && imageSize.height() <= imageViewHeight)
                imageSize.scale(calcZoom(imageSize.width()), calcZoom(imageSize.height()), Qt::KeepAspectRatio);
            break;

        case ZoomMethods::WidthAndHeight:
            if (imageSize.width() <= imageViewWidth && imageSize.height() <= imageViewHeight)
                imageSize.scale(calcZoom(imageViewWidth), calcZoom(imageViewHeight), Qt::KeepAspectRatio);
            break;

        case ZoomMethods::Width:
            if (imageSize.width() <= imageViewWidth)
            {
                imageSize.scale(calcZoom(imageViewWidth),
                                calcZoom(getHeightByWidth(imageSize.width(), imageSize.height(), imageViewWidth)),
                                Qt::KeepAspectRatio);
            }
            break;

        case ZoomMethods::Height:
            if (imageSize.height() <= imageViewHeight)
            {
                imageSize.scale(calcZoom(getWidthByHeight(imageSize.height(), imageSize.width(), imageViewHeight)),
                                calcZoom(imageViewHeight), Qt::KeepAspectRatio);
            }
            break;

        case ZoomMethods::Disprop:
            int newWidth = imageSize.width(), newHeight = imageSize.height();
            if (newWidth <= imageViewWidth) newWidth = imageViewWidth;
            if (newHeight <= imageViewHeight) newHeight = imageViewHeight;
            imageSize.scale(calcZoom(newWidth), calcZoom(newHeight), Qt::IgnoreAspectRatio);
            break;
        }

        switch (m_zoom_out_flags)
        {
        case ZoomMethods::Disable:
            if (imageSize.width() >= imageViewWidth || imageSize.height() >= imageViewHeight)
                imageSize.scale(calcZoom(imageSize.width()), calcZoom(imageSize.height()), Qt::KeepAspectRatio);
            break;

        case ZoomMethods::WidthAndHeight:
            if (imageSize.width() >= imageViewWidth || imageSize.height() >= imageViewHeight)
                imageSize.scale(calcZoom(imageViewWidth), calcZoom(imageViewHeight), Qt::KeepAspectRatio);
            break;

        case ZoomMethods::Width:
            if (imageSize.width() >= imageViewWidth)
            {
                imageSize.scale(calcZoom(imageViewWidth),
                                calcZoom(getHeightByWidth(imageSize.width(), imageSize.height(), imageViewWidth)),
                                Qt::KeepAspectRatio);
            }
            break;

        case ZoomMethods::Height:
            if (imageSize.height() >= imageViewHeight)
            {
                imageSize.scale(calcZoom(getWidthByHeight(imageSize.height(), imageSize.width(), imageViewHeight)),
                                calcZoom(imageViewHeight), Qt::KeepAspectRatio);
            }
            break;

        case ZoomMethods::Disprop:
            int newWidth = imageSize.width(), newHeight = imageSize.height();
            if (newWidth >= imageViewWidth) newWidth = imageViewWidth;
            if (newHeight >= imageViewHeight) newHeight = imageViewHeight;
            imageSize.scale(calcZoom(newWidth), calcZoom(newHeight), Qt::IgnoreAspectRatio);
            break;
        }
    }

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

void ImageViewer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    resizeImage();
}

void ImageViewer::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    resizeImage();
}

void ImageViewer::centerImage(QSize& imgSize)
{
    m_scroll_area->ensureVisible(imgSize.width() / 2, imgSize.height() / 2, width() / 2, height() / 2);
}

void ImageViewer::transform()
{
    if (!qFuzzyCompare(m_rotation, 0))
    {
        QTransform trans;
        trans.rotate(m_rotation);
        m_viewer_image = m_viewer_image.transformed(trans, Qt::SmoothTransformation);
    }

    if (m_flipH || m_flipV) m_viewer_image = m_viewer_image.mirrored(m_flipH, m_flipV);

    if (m_crop_left || m_crop_top || m_crop_width || m_crop_height)
        m_viewer_image = m_viewer_image.copy(m_crop_left, m_crop_top, m_crop_width, m_crop_height);
}

void ImageViewer::refresh()
{
    if (!m_image_widget) return;

    if (m_scaled_width)
        m_viewer_image =
            m_orig_image.scaled(m_scaled_width, m_scaled_height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    else
        m_viewer_image = m_orig_image;

    transform();

    m_image_widget->setImage(m_viewer_image);
    resizeImage();
}

void ImageViewer::setImage(const QImage& image)
{
    m_image_widget->setImage(image);
}

void ImageViewer::reload()
{
    m_scaled_width = m_scaled_height = 0;
    m_viewer_image = m_orig_image;

    setImage(m_viewer_image);
    resizeImage();
    if (m_crop_left || m_crop_top || m_crop_width || m_crop_height) m_crop_rubber_band->show();
    m_image_widget->setRotation(m_rotation);
}

void ImageViewer::monitorCursorState()
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

void ImageViewer::setCursorHiding(bool hide)
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

void ImageViewer::mouseDoubleClickEvent(QMouseEvent* event)
{
    QWidget::mouseDoubleClickEvent(event);
    while (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
}

void ImageViewer::mousePressEvent(QMouseEvent* event)
{
    if (!m_image_widget) return;
    if (event->button() == Qt::LeftButton)
    {
        if (event->modifiers() == Qt::ControlModifier)
        {
            m_crop_origin = event->pos();
            if (!m_crop_rubber_band) m_crop_rubber_band = new CropRubberBand(this);
            m_crop_rubber_band->show();
            m_crop_rubber_band->setGeometry(QRect(m_crop_origin, event->pos()).normalized());
        }
        else
        {
            if (m_crop_rubber_band) m_crop_rubber_band->hide();
        }
        m_initial_rotation = m_image_widget->rotation();
        auto pos = event->position();
        setMouseMoveData(true, qRound(pos.x()), qRound(pos.y()));
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    // rotation
    // TODO: roate aroung center?
    // m_mouse_rotate_enabled = event->button() == Qt::RightButton;
    // crop
    if (event->button() == Qt::MiddleButton) applyCropAndRotation();
    QWidget::mousePressEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        setMouseMoveData(false, 0, 0);
        while (QApplication::overrideCursor())
            QApplication::restoreOverrideCursor();
    }

    QWidget::mouseReleaseEvent(event);
}

void ImageViewer::wheelEvent(QWheelEvent* event)
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

void ImageViewer::applyCropAndRotation()
{
    if (!m_image_widget) return;

    bool didSomething = false;
    if (m_crop_rubber_band && m_crop_rubber_band->isVisible())
    {
        QPoint bandTopLeft = m_image_widget->mapFromGlobal(mapToGlobal(m_crop_rubber_band->geometry().topLeft()));
        QPoint bandBottomRight =
            m_image_widget->mapFromGlobal(mapToGlobal(m_crop_rubber_band->geometry().bottomRight()));

        double scaledX = m_image_widget->width();
        double scaledY = m_image_widget->height();
        scaledX = m_viewer_image.width() / scaledX;
        scaledY = m_viewer_image.height() / scaledY;

        bandTopLeft.setX(int(bandTopLeft.x() * scaledX));
        bandTopLeft.setY(int(bandTopLeft.y() * scaledY));
        bandBottomRight.setX(int(bandBottomRight.x() * scaledX));
        bandBottomRight.setY(int(bandBottomRight.y() * scaledY));

        m_crop_left = bandTopLeft.x();
        m_crop_top = bandTopLeft.y();
        m_crop_width = bandBottomRight.x() - bandTopLeft.x();
        m_crop_height = bandBottomRight.y() - bandTopLeft.y();
        m_rotation = m_image_widget->rotation();

        m_crop_rubber_band->hide();

        transform();
        m_image_widget->setImage(m_viewer_image);
        resizeImage();
        didSomething = true;
    }
    if (!qFuzzyCompare(m_image_widget->rotation(), 0))
    {
        refresh();
        m_image_widget->setRotation(0);
        didSomething = true;
    }
    if (!didSomething)
        QMessageBox::warning(
            this, tr("No selection for cropping, and no rotation"),
            tr("To make a selection, hold down the Ctrl key and select a region using the mouse. "
               "To rotate, hold down the Ctrl and Shift keys and drag the mouse near the right edge."));
}

void ImageViewer::setMouseMoveData(bool lockMove, int lMouseX, int lMouseY)
{
    if (!m_image_widget) return;
    m_move_image_locked = lockMove;
    m_mouseX = lMouseX;
    m_mouseY = lMouseY;
    m_layoutX = m_image_widget->pos().x();
    m_layoutY = m_image_widget->pos().y();
}

void ImageViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_image_widget) return;

    if (m_mouse_rotate_enabled)
    {
        QPointF fulcrum(QPointF(m_image_widget->pos()) +
                        QPointF(m_image_widget->width() / 2.0, m_image_widget->height() / 2.0));
        if (event->pos().x() > (width() * 3) / 4)
            fulcrum.setY(m_mouseY); // if the user pressed near the right edge, start with initial rotation of 0
        QLineF vector(fulcrum, event->position());
        m_image_widget->setRotation(m_initial_rotation - vector.angle());
    }
    else if (event->modifiers() & Qt::ControlModifier)
    {
        if (!m_crop_rubber_band || !m_crop_rubber_band->isVisible()) return;
        QRect newRect;
        newRect = QRect(m_crop_origin, event->pos());

        // Force square
        if (event->modifiers() & Qt::ShiftModifier)
        {
            const int deltaX = m_crop_origin.x() - event->pos().x();
            const int deltaY = m_crop_origin.y() - event->pos().y();
            newRect.setSize(QSize(-deltaX, deltaY < 0 ? qAbs(deltaX) : -qAbs(deltaX)));
        }

        m_crop_rubber_band->setGeometry(newRect.normalized());
    }
    else if (m_move_image_locked)
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

void ImageViewer::contextMenuEvent(QContextMenuEvent*)
{
    while (QApplication::overrideCursor())
        QApplication::restoreOverrideCursor();
    m_context_menu_position = QCursor::pos();
    //ImagePopUpMenu->exec(m_context_menu_position);
}

int ImageViewer::getImageWidthPreCropped()
{
    return m_orig_image.width();
}

int ImageViewer::getImageHeightPreCropped()
{
    return m_orig_image.height();
}

void ImageViewer::copyImage()
{
    QApplication::clipboard()->setImage(m_viewer_image);
}

void ImageViewer::pasteImage()
{
    if (!m_image_widget) return;

    if (!QApplication::clipboard()->image().isNull())
    {
        m_orig_image = QApplication::clipboard()->image();
        refresh();
    }
}

QPoint ImageViewer::getContextMenuPosition()
{
    return m_context_menu_position;
}
