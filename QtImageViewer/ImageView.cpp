#include "ImageView.h"

#include <QScrollbar>
#include <QTimeline>
#include <QTimer>
#include <QApplication>
#include <QMouseEvent>

ImageView::ImageView(QWidget* parent): QGraphicsView(parent)
{
    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    this->viewport()->setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAcceptDrops(false);

    m_dpr = this->devicePixelRatioF();
    m_hs = horizontalScrollBar();
    m_vs = verticalScrollBar();

    m_scroll_time_line_y = new QTimeLine();
    m_scroll_time_line_y->setEasingCurve(QEasingCurve::OutSine);
    m_scroll_time_line_y->setDuration(SCROLL_SPEED);
    m_scroll_time_line_y->setUpdateInterval(SCROLL_UPDATE_RATE);
    m_scroll_time_line_x = new QTimeLine();
    m_scroll_time_line_x->setEasingCurve(QEasingCurve::OutSine);
    m_scroll_time_line_x->setDuration(SCROLL_SPEED);
    m_scroll_time_line_x->setUpdateInterval(SCROLL_UPDATE_RATE);

    m_scale_timer = new QTimer(this);
    m_scale_timer->setSingleShot(true);
    m_scale_timer->setInterval(80);

    m_zoom_threshold = static_cast<int>(m_dpr * 4.);

    m_pixmap_item.setTransformationMode(Qt::SmoothTransformation);
    m_pixmap_item.setScale(1.0f);
    m_pixmap_item.setOffset(10000, 10000);
    m_pixmap_item.setTransformOriginPoint(10000, 10000);
    m_pixmap_item.setTransformationMode(Qt::SmoothTransformation);
    m_scaled_pixmap_item.setScale(1.0f);
    m_scaled_pixmap_item.setOffset(10000, 10000);
    m_scaled_pixmap_item.setTransformOriginPoint(10000, 10000);
    m_scaled_pixmap_item.setTransformationMode(Qt::SmoothTransformation);

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_scene = new QGraphicsScene();
    m_scene->setSceneRect(0, 0, 200000, 200000);
    m_scene->setBackgroundBrush(QColor(0, 0, 0));
    m_scene->addItem(&m_pixmap_item);
    m_scene->addItem(&m_scaled_pixmap_item);
    m_scaled_pixmap_item.hide();

    this->setFrameShape(QFrame::NoFrame);
    this->setScene(m_scene);

    connect(m_scroll_time_line_x, &QTimeLine::finished, this, &ImageView::onScrollTimelineFinished);
    connect(m_scroll_time_line_y, &QTimeLine::finished, this, &ImageView::onScrollTimelineFinished);
    connect(m_scroll_time_line_x, &QTimeLine::frameChanged, this, &ImageView::scrollToX);
    connect(m_scroll_time_line_y, &QTimeLine::frameChanged, this, &ImageView::scrollToY);
    connect(m_scale_timer, &QTimer::timeout, this, &ImageView::requestScaling);
}

void ImageView::setPixmap(QPixmap pixmap)
{
    m_pixmap = pixmap;
    m_pixmap.setDevicePixelRatio(m_dpr);
    m_pixmap_item.setPixmap(m_pixmap);
    m_pixmap_item.show();
    m_pixmap_item.update();
}

void ImageView::showPixmap(QPixmap pixmap)
{
    reset();

    if (!pixmap.isNull())
    {
        m_scaled_pixmap_item.hide();
        m_pixmap = pixmap;
        m_pixmap.setDevicePixelRatio(m_dpr);
        m_pixmap_item.setPixmap(m_pixmap);
        m_pixmap_item.show();
        updateMinScale();

        if (m_view_lock_mode == ViewLockMode::NONE)
            applyFitMode();
        else
        {
            m_image_fit_mode = ImageFitMode::FIT_FREE;
            fitFree(m_locked_scale);
            if (m_view_lock_mode == ViewLockMode::LOCK_ALL) applySavedViewportPos();
        }
        requestScaling();
        update();
    }
}

float ImageView::scale() const
{
    return m_pixmap_item.scale();
}

QSize ImageView::sourceSize() const
{
    return m_pixmap.size();
}

QSize ImageView::scaledSize() const
{
    if (m_pixmap.isNull()) return QSize(0, 0);
    QRectF pixmap_scene_rect = m_pixmap_item.mapRectToScene(m_pixmap_item.boundingRect());
    return sceneRoundRect(pixmap_scene_rect).size().toSize();
}

QRect ImageView::scaledRect() const
{
    QRectF pixmap_scene_rect = m_pixmap_item.mapRectToScene(m_pixmap_item.boundingRect());
    return QRect(mapFromScene(pixmap_scene_rect.topLeft()), mapFromScene(pixmap_scene_rect.bottomRight()));
}

void ImageView::reset()
{
    m_scaled_pixmap = QPixmap();
    m_scaled_pixmap_item.setPixmap(m_scaled_pixmap);
    m_pixmap = QPixmap();
    m_pixmap_item.setPixmap(m_pixmap);
    m_pixmap_item.setScale(1.0f);
    m_pixmap_item.setOffset(10000, 10000);
    centerOn(sceneRect().center());
    viewport()->update();
}

void ImageView::closeImage()
{
    reset();
}

void ImageView::setScaledPixmap(QPixmap pixmap)
{
    m_scaled_pixmap = pixmap;
    m_scaled_pixmap.setDevicePixelRatio(m_dpr);
    m_scaled_pixmap_item.setPixmap(m_scaled_pixmap);
    m_pixmap_item.hide();
    m_scaled_pixmap_item.show();
}

bool ImageView::isDisplaying() const
{
    return !m_pixmap.isNull();
}

void ImageView::scrollUp()
{
    scroll(0, -SCROLL_DISTANCE);
}

void ImageView::scrollDown()
{
    scroll(0, SCROLL_DISTANCE);
}

void ImageView::scrollLeft()
{
    scroll(-SCROLL_DISTANCE, 0);
}

void ImageView::scrollRight()
{
    scroll(SCROLL_DISTANCE, 0);
}

ImageFitMode ImageView::fitMode() const
{
    return m_image_fit_mode;
}

void ImageView::setFitMode(ImageFitMode mode)
{
    if (m_scale_timer->isActive()) m_scale_timer->stop();
    stopPosAnimation();
    m_image_fit_mode = mode;
    applyFitMode();
    requestScaling();
}

void ImageView::setFitOriginal()
{
    setFitMode(ImageFitMode::FIT_ORIGINAL);
}

void ImageView::setFitWidth()
{
    setFitMode(ImageFitMode::FIT_WIDTH);
    requestScaling();
}

void ImageView::setFitWindow()
{
    setFitMode(ImageFitMode::FIT_WINDOW);
    requestScaling();
}

void ImageView::stopPosAnimation()
{
    if (m_scroll_time_line_x->state() == QTimeLine::Running) m_scroll_time_line_x->stop();
    if (m_scroll_time_line_y->state() == QTimeLine::Running) m_scroll_time_line_y->stop();
}

void ImageView::applyFitMode()
{
    switch (m_image_fit_mode)
    {
    case ImageFitMode::FIT_WINDOW:
        fitWindow();
        break;
    case ImageFitMode::FIT_WIDTH:
        fitWidth();
        break;
    case ImageFitMode::FIT_ORIGINAL:
        fitNormal();
        break;
    default:
        break;
    }
}

void ImageView::requestScaling()
{
    if (m_pixmap.isNull() || m_pixmap_item.scale() == 1.f) return;
    if (m_scale_timer->isActive()) m_scale_timer->stop();
    if (scale() < 1.f) emit scalingRequested(scaledSize() * m_dpr);
}

bool ImageView::pixmapFit() const
{
    if (m_pixmap.isNull()) return true;
    return (m_pixmap.width() <= (viewport()->width() * devicePixelRatioF()) &&
            m_pixmap.height() <= (viewport()->height() * devicePixelRatioF()));
}

bool ImageView::scaledPixmapFit() const
{
    if (m_pixmap.isNull()) return true;
    QSize sz = scaledSize();
    return (sz.width() <= viewport()->width() && sz.height() <= viewport()->height());
}

void ImageView::scroll(int dx, int dy)
{
    stopPosAnimation();
    m_hs->setValue(m_hs->value() + dx);
    m_vs->setValue(m_vs->value() + dy);
    centerIfNecessary();
    snapToEdges();
    saveViewportPos();
}

void ImageView::scrollToX(int x)
{
    m_hs->setValue(x);
    centerIfNecessary();
    snapToEdges();
    update();
    qApp->processEvents();
}

void ImageView::scrollToY(int y)
{
    m_vs->setValue(y);
    centerIfNecessary();
    snapToEdges();
    update();
    qApp->processEvents();
}

void ImageView::onScrollTimelineFinished()
{
    saveViewportPos();
}

void ImageView::setExpandImage(bool flag)
{
    m_expand_image = flag;
    updateMinScale();
    applyFitMode();
    requestScaling();
}

void ImageView::show()
{
    setMouseTracking(false);
    QGraphicsView::show();
    setMouseTracking(true);
}

void ImageView::hide()
{
    setMouseTracking(false);
    QWidget::hide();
}

void ImageView::mousePressEvent(QMouseEvent* event)
{
    if (m_pixmap.isNull())
    {
        QWidget::mousePressEvent(event);
        return;
    }
    m_mouse_move_start_pos = event->pos();
    m_mouse_press_pos = m_mouse_move_start_pos;
    if (event->button() & Qt::RightButton)
        setZoomAnchor(event->pos());
    else
        QGraphicsView::mousePressEvent(event);
}

void ImageView::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    if (m_pixmap.isNull() || m_mouse_state == MouseState::DRAG || m_mouse_state == MouseState::WHEEL_ZOOM) return;

    if (event->buttons() & Qt::LeftButton)
    {
        // DRAG / PAN
        if (m_mouse_state == MouseState::NONE)
        {
            if (scaledPixmapFit())
                m_mouse_state = MouseState::DRAG_BEGIN;
            else
            {
                m_mouse_state = MouseState::PAN;
                if (cursor().shape() != Qt::ClosedHandCursor) setCursor(Qt::ClosedHandCursor);
            }
        }
        // dragging
        if (m_mouse_state == MouseState::DRAG_BEGIN)
        {
            if ((abs(m_mouse_press_pos.x() - event->pos().x()) > m_drag_threshold) ||
                abs(m_mouse_press_pos.y() - event->pos().y()) > m_drag_threshold)
            {
                m_mouse_state = MouseState::NONE;
                emit draggedOut();
            }
        }
        // panning
        if (m_mouse_state == MouseState::PAN) mousePan(event);
        return;
    }
    else if (event->buttons() & Qt::RightButton)
    {
        // ZOOM
        if (m_mouse_state == MouseState::ZOOM ||
            abs(m_mouse_press_pos.y() - event->pos().y()) > m_zoom_threshold / m_dpr)
        {
            if (cursor().shape() != Qt::SizeVerCursor) setCursor(Qt::SizeVerCursor);
            m_mouse_state = MouseState::ZOOM;
            mouseMoveZoom(event);
        }
        return;
    }
    else
        event->ignore();
}

void ImageView::mouseReleaseEvent(QMouseEvent* event)
{
    unsetCursor();
    if (m_pixmap.isNull() || m_mouse_state == MouseState::NONE)
    {
        QGraphicsView::mouseReleaseEvent(event);
        event->ignore();
    }
    m_mouse_state = MouseState::NONE;
}

// for some reason in qgraphicsview wheelEvent is followed by moveEvent (wtf?)
void ImageView::wheelEvent(QWheelEvent* event)
{
    qDebug() << event->modifiers() << event->pixelDelta() << event->angleDelta();

    if (event->buttons() & Qt::RightButton)
    {
        event->accept();
        m_mouse_state = MouseState::WHEEL_ZOOM;
        int angleDelta = event->angleDelta().ry();
        if (angleDelta > 0)
            zoomInCursor();
        else if (angleDelta < 0)
            zoomOutCursor();
    }
    else if (event->modifiers() == Qt::NoModifier)
    {
        QPoint pixelDelta = event->pixelDelta();
        QPoint angleDelta = event->angleDelta();
        // scroll by interval
        QRect imgRect = scaledRect();
        // shift by 2px in case of img edge misalignment
        if ((event->angleDelta().y() < 0 && imgRect.bottom() > height() + 2) ||
            (event->angleDelta().y() > 0 && imgRect.top() < -2))
        {
            event->accept();
            scroll(0, -angleDelta.y());
        }
        else
            event->ignore();
        saveViewportPos();
    }
    else
    {
        event->ignore();
        QWidget::wheelEvent(event);
    }
}

void ImageView::showEvent(QShowEvent* event)
{
    QGraphicsView::showEvent(event);
    qApp->processEvents();
    if (m_image_fit_mode == ImageFitMode::FIT_ORIGINAL) applyFitMode();
}

void ImageView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    m_mouse_press_pos = mapFromGlobal(cursor().pos());
    if (parentWidget()->isVisible())
    {
        stopPosAnimation();
        updateMinScale();
        if (m_image_fit_mode == ImageFitMode::FIT_FREE || m_image_fit_mode == ImageFitMode::FIT_ORIGINAL)
        {
            centerIfNecessary();
            snapToEdges();
        }
        else
            applyFitMode();
        update();
        if (m_scale_timer->isActive()) m_scale_timer->stop();
        m_scale_timer->start();
        saveViewportPos();
    }
}

void ImageView::mousePan(QMouseEvent* event)
{
    if (scaledPixmapFit()) return;
    m_mouse_move_start_pos -= event->pos();
    scroll(m_mouse_move_start_pos.x(), m_mouse_move_start_pos.y());
    m_mouse_move_start_pos = event->pos();
    saveViewportPos();
}

void ImageView::mouseMoveZoom(QMouseEvent* event)
{
    float step_multiplier = 0.003f; // this one feels ok
    int current_pos = event->pos().y();
    int move_distance = m_mouse_move_start_pos.y() - current_pos;
    float new_scale = scale() * (1.0f + step_multiplier * move_distance * m_dpr);
    m_mouse_move_start_pos = event->pos();
    m_image_fit_mode = ImageFitMode::FIT_FREE;

    zoomAnchored(new_scale);
    centerIfNecessary();
    snapToEdges();
    if (m_pixmap_item.scale() == m_fit_window_scale) m_image_fit_mode = ImageFitMode::FIT_WINDOW;
}

void ImageView::updateFitWindowScale()
{
    m_fit_window_scale = std::min((float)viewport()->width() * devicePixelRatioF() / m_pixmap.width(),
                                  (float)viewport()->height() * devicePixelRatioF() / m_pixmap.height());
}

void ImageView::updateMinScale()
{
    if (m_pixmap.isNull()) return;
    updateFitWindowScale();
    m_min_scale = qMax(10. / m_pixmap.width(), 10. / m_pixmap.height());
    if (m_view_lock_mode != ViewLockMode::NONE && m_locked_scale < m_min_scale) m_min_scale = m_locked_scale;
}

void ImageView::centerOnPixmap()
{
    auto rect = m_pixmap_item.sceneBoundingRect();
    auto port = mapToScene(viewport()->geometry()).boundingRect();
    m_hs->setValue(m_pixmap_item.offset().x() - (int)(port.width() - rect.width()) / 2);
    m_vs->setValue(m_pixmap_item.offset().y() - (int)(port.height() - rect.height()) / 2);
}

void ImageView::fitWidth()
{
    if (m_pixmap.isNull()) return;
    float scale_x = viewport()->width() * m_dpr / m_pixmap.width();
    if (!m_expand_image && scale_x > 1.0f) scale_x = 1.0f;
    if (scale() != scale_x)
    {
        swapToOriginalPixmap();
        doZoom(scale_x);
    }
    centerIfNecessary();
    if (scaledSize().height() > viewport()->height())
    {
        QPointF center = mapToScene(viewport()->rect()).boundingRect().center();
        center.setY(0);
        centerOn(center);
    }
    snapToEdges();
}

void ImageView::fitWindow()
{
    if (m_pixmap.isNull()) return;
    if (pixmapFit() && !m_expand_image)
        fitNormal();
    else
    {
        if (scale() != m_fit_window_scale)
        {
            swapToOriginalPixmap();
            doZoom(m_fit_window_scale);
        }
        centerOnPixmap();
    }
}

void ImageView::fitNormal()
{
    fitFree(1.0f);
}

void ImageView::fitFree(float scale)
{
    if (m_pixmap.isNull()) return;
    setZoomAnchor(viewport()->rect().center());
    zoomAnchored(scale);
    centerIfNecessary();
    snapToEdges();
}

void ImageView::swapToOriginalPixmap()
{
    if (m_pixmap.isNull() || !m_pixmap_item.isVisible()) return;
    m_scaled_pixmap_item.hide();
    m_scaled_pixmap = QPixmap();
    m_scaled_pixmap_item.setPixmap(m_scaled_pixmap);
    m_pixmap_item.show();
}

void ImageView::setZoomAnchor(QPoint viewport_pos)
{
    m_zoom_anchor = QPair<QPointF, QPoint>(m_pixmap_item.mapFromScene(mapToScene(viewport_pos)), viewport_pos);
}

void ImageView::zoomAnchored(float scale)
{
    if (this->scale() != scale)
    {
        QPointF vportCenter = mapToScene(viewport()->geometry()).boundingRect().center();
        doZoom(scale);
        QPointF diff = m_zoom_anchor.second - mapFromScene(m_pixmap_item.mapToScene(m_zoom_anchor.first));
        centerOn(vportCenter - diff);
        requestScaling();
    }
}

void ImageView::zoomIn()
{
    doZoom(false, true);
}

void ImageView::zoomInCursor()
{
    doZoom(true, true);
}

void ImageView::zoomOut()
{
    doZoom(false, false);
}

void ImageView::zoomOutCursor()
{
    doZoom(true, false);
}

void ImageView::doZoom(float scale)
{
    if (m_pixmap.isNull()) return;
    scale = qBound(m_min_scale, scale, 500.0f);
    auto tl = m_pixmap_item.sceneBoundingRect().topLeft().toPoint();
    m_pixmap_item.setOffset(tl);
    m_pixmap_item.setScale(scale);

    swapToOriginalPixmap();
    emit scaleChanged(scale);
}

void ImageView::doZoom(bool at_cursor, bool zoom_in)
{
    if (at_cursor && underMouse())
        setZoomAnchor(mapFromGlobal(cursor().pos()));
    else
        setZoomAnchor(viewport()->rect().center());
    zoomAnchored(scale() * zoom_in ? (1.0f + m_zoom_step) : (1.0f - m_zoom_step));
    centerIfNecessary();
    snapToEdges();
    m_image_fit_mode = ImageFitMode::FIT_FREE;
    if (m_pixmap_item.scale() == m_fit_window_scale) m_image_fit_mode = ImageFitMode::FIT_WINDOW;
}

void ImageView::toggleLockZoom()
{
    if (!isDisplaying()) return;
    if (m_view_lock_mode != ViewLockMode::LOCK_ZOOM)
    {
        m_view_lock_mode = ViewLockMode::LOCK_ZOOM;
        lockZoom();
    }
    else
        m_view_lock_mode = ViewLockMode::NONE;
}

bool ImageView::lockZoomEnabled() const
{
    return (m_view_lock_mode == ViewLockMode::LOCK_ZOOM);
}

void ImageView::lockZoom()
{
    m_locked_scale = m_pixmap_item.scale();
    m_image_fit_mode = ImageFitMode::FIT_FREE;
    saveViewportPos();
}

void ImageView::toggleLockView()
{
    if (!isDisplaying()) return;
    if (m_view_lock_mode != ViewLockMode::LOCK_ALL)
    {
        m_view_lock_mode = ViewLockMode::LOCK_ALL;
        lockZoom();
        saveViewportPos();
    }
    else
        m_view_lock_mode = ViewLockMode::NONE;
}

bool ImageView::lockViewEnabled() const
{
    return (m_view_lock_mode == ViewLockMode::LOCK_ALL);
}

// savedViewportPos is [0...1][0...1]
// values are where viewport center is on the image
void ImageView::saveViewportPos()
{
    if (m_view_lock_mode != ViewLockMode::LOCK_ALL) return;
    QPointF scene_senter = mapToScene(viewport()->rect().center()) + QPointF(1, 1);
    auto item_rect = m_pixmap_item.sceneBoundingRect();
    m_saved_viewport_pos.setX(qBound(qreal(0), (scene_senter.x() - item_rect.left()) / item_rect.width(), qreal(1)));
    m_saved_viewport_pos.setY(qBound(qreal(0), (scene_senter.y() - item_rect.top()) / item_rect.height(), qreal(1)));
}

void ImageView::applySavedViewportPos()
{
    auto item_rect = m_pixmap_item.sceneBoundingRect();
    QPointF new_scene_pos{};
    new_scene_pos.setX(item_rect.left() + item_rect.width() * m_saved_viewport_pos.x());
    new_scene_pos.setY(item_rect.top() + item_rect.height() * m_saved_viewport_pos.y());
    centerOn(new_scene_pos);
    centerIfNecessary();
    snapToEdges();
}

void ImageView::centerIfNecessary()
{
    if (m_pixmap.isNull()) return;
    QSize sz = scaledSize();
    auto rect = m_pixmap_item.sceneBoundingRect();
    auto port = mapToScene(viewport()->geometry()).boundingRect();
    if (sz.width() <= viewport()->width())
        m_hs->setValue(m_pixmap_item.offset().x() - (int)(port.width() - rect.width()) / 2);
    if (sz.height() <= viewport()->height())
        m_vs->setValue(m_pixmap_item.offset().y() - (int)(port.height() - rect.height()) / 2);
}

void ImageView::snapToEdges()
{
    QRect rect = scaledRect();
    // current vport center
    QPointF centerTarget = mapToScene(viewport()->rect()).boundingRect().center();
    qreal shift_x = 0;
    qreal shift_y = 0;
    if (rect.width() > width())
    {
        if (rect.left() > 0)
            shift_x = rect.left();
        else if (rect.right() < width())
            shift_x = rect.right() - width();
    }
    if (rect.height() > height())
    {
        if (rect.top() > 0)
            shift_y = rect.top();
        else if (rect.bottom() < height())
            shift_y = rect.bottom() - height();
    }
    centerOn(centerTarget + QPointF(shift_x, shift_y));
}

QPointF ImageView::sceneRoundPos(QPointF scenePoint) const
{
    return mapToScene(mapFromScene(scenePoint));
}

QRectF ImageView::sceneRoundRect(QRectF sceneRect) const
{
    return QRectF(sceneRoundPos(sceneRect.topLeft()), sceneRect.size());
}
