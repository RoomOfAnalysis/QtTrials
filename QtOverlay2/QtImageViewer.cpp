#include "QtImageViewer.h"

#include <QVBoxLayout>
#include <QMouseEvent>

QtImageViewer::QtImageViewer(QWidget* parent): QtOverlayWidgetContainer(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    setMouseTracking(true);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    this->setLayout(layout);

    m_img_view.reset(new ImageView(this));
    layout->addWidget(m_img_view.get());

    m_img_view->show();
    setInteractionEnabled(true);

    connect(&m_cursor_timer, &QTimer::timeout, this, &QtImageViewer::hideCursor);
}

QRect QtImageViewer::imageRect()
{
    if (m_img_view)
        return m_img_view->scaledRect();
    else
        return QRect(0, 0, 0, 0);
}

float QtImageViewer::currentScale()
{
    return m_img_view->scale();
}

QSize QtImageViewer::sourceSize()
{
    return m_img_view->sourceSize();
}

void QtImageViewer::setInteractionEnabled(bool mode)
{
    if (m_interaction_enabled == mode) return;
    m_interaction_enabled = mode;
    if (m_interaction_enabled)
    {
        connect(this, &QtImageViewer::toggleLockZoom, m_img_view.get(), &ImageView::toggleLockZoom);
        connect(this, &QtImageViewer::toggleLockView, m_img_view.get(), &ImageView::toggleLockView);
        connect(this, &QtImageViewer::zoomIn, m_img_view.get(), &ImageView::zoomIn);
        connect(this, &QtImageViewer::zoomOut, m_img_view.get(), &ImageView::zoomOut);
        connect(this, &QtImageViewer::zoomInCursor, m_img_view.get(), &ImageView::zoomInCursor);
        connect(this, &QtImageViewer::zoomOutCursor, m_img_view.get(), &ImageView::zoomOutCursor);
        connect(this, &QtImageViewer::scrollUp, m_img_view.get(), &ImageView::scrollUp);
        connect(this, &QtImageViewer::scrollDown, m_img_view.get(), &ImageView::scrollDown);
        connect(this, &QtImageViewer::scrollLeft, m_img_view.get(), &ImageView::scrollLeft);
        connect(this, &QtImageViewer::scrollRight, m_img_view.get(), &ImageView::scrollRight);
        connect(this, &QtImageViewer::fitWindow, m_img_view.get(), &ImageView::setFitWindow);
        connect(this, &QtImageViewer::fitWidth, m_img_view.get(), &ImageView::setFitWidth);
        connect(this, &QtImageViewer::fitOriginal, m_img_view.get(), &ImageView::setFitOriginal);
        connect(m_img_view.get(), &ImageView::draggedOut, this, &QtImageViewer::draggedOut);
        m_img_view->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
    else
    {
        disconnect(this, &QtImageViewer::zoomIn, m_img_view.get(), &ImageView::zoomIn);
        disconnect(this, &QtImageViewer::zoomOut, m_img_view.get(), &ImageView::zoomOut);
        disconnect(this, &QtImageViewer::zoomInCursor, m_img_view.get(), &ImageView::zoomInCursor);
        disconnect(this, &QtImageViewer::zoomOutCursor, m_img_view.get(), &ImageView::zoomOutCursor);
        disconnect(this, &QtImageViewer::scrollUp, m_img_view.get(), &ImageView::scrollUp);
        disconnect(this, &QtImageViewer::scrollDown, m_img_view.get(), &ImageView::scrollDown);
        disconnect(this, &QtImageViewer::scrollLeft, m_img_view.get(), &ImageView::scrollLeft);
        disconnect(this, &QtImageViewer::scrollRight, m_img_view.get(), &ImageView::scrollRight);
        disconnect(this, &QtImageViewer::fitWindow, m_img_view.get(), &ImageView::setFitWindow);
        disconnect(this, &QtImageViewer::fitWidth, m_img_view.get(), &ImageView::setFitWidth);
        disconnect(this, &QtImageViewer::fitOriginal, m_img_view.get(), &ImageView::setFitOriginal);
        disconnect(m_img_view.get(), &ImageView::draggedOut, this, &QtImageViewer::draggedOut);
        m_img_view->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        hideContextMenu();
    }
}

bool QtImageViewer::interactionEnabled() const
{
    return m_interaction_enabled;
}

bool QtImageViewer::showImage(QPixmap pixmap)
{
    if (!pixmap) return false;
    m_img_view->showPixmap(pixmap);
    hideCursorTimed(false);
    return true;
}

void QtImageViewer::setFitMode(ImageFitMode mode)
{
    if (mode == ImageFitMode::FIT_WINDOW)
        emit fitWindow();
    else if (mode == ImageFitMode::FIT_WIDTH)
        emit fitWidth();
    else if (mode == ImageFitMode::FIT_ORIGINAL)
        emit fitOriginal();
}

ImageFitMode QtImageViewer::fitMode() const
{
    return m_img_view->fitMode();
}

void QtImageViewer::onScalingFinished(QPixmap scaled)
{
    m_img_view->setScaledPixmap(scaled);
}

void QtImageViewer::closeImage()
{
    m_img_view->closeImage();
    showCursor();
}

bool QtImageViewer::isDisplaying() const
{
    return m_img_view->isDisplaying();
}

bool QtImageViewer::lockZoomEnabled() const
{
    return m_img_view->lockZoomEnabled();
}

bool QtImageViewer::lockViewEnabled() const
{
    return m_img_view->lockViewEnabled();
}

void QtImageViewer::mousePressEvent(QMouseEvent* event)
{
    hideContextMenu();
    event->ignore();
}

void QtImageViewer::mouseReleaseEvent(QMouseEvent* event)
{
    showCursor();
    hideCursorTimed(false);
    event->ignore();
}

void QtImageViewer::mouseMoveEvent(QMouseEvent* event)
{
    if (!(event->buttons() & Qt::LeftButton) && !(event->buttons() & Qt::RightButton))
    {
        showCursor();
        hideCursorTimed(true);
    }
    event->ignore();
}

void QtImageViewer::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) showContextMenu();
    event->ignore();
}

void QtImageViewer::hideCursorTimed(bool restartTimer)
{
    if (restartTimer || !m_cursor_timer.isActive()) m_cursor_timer.start(CURSOR_HIDE_TIMEOUT_MS);
}

void QtImageViewer::hideCursor()
{
    m_cursor_timer.stop();
    // ignore if we have something else open
    if (!isDisplaying() || !isActiveWindow()) return;
    // ignore when menu is up
    if (m_context_menu && m_context_menu->isVisible()) return;
}

void QtImageViewer::showCursor()
{
    m_cursor_timer.stop();
    if (cursor().shape() == Qt::BlankCursor) setCursor(QCursor(Qt::ArrowCursor));
}

void QtImageViewer::showContextMenu()
{
    QPoint pos = cursor().pos();
    showContextMenu(pos);
}

void QtImageViewer::showContextMenu(QPoint pos)
{
    if (isVisible())
    {
        if (!m_context_menu) m_context_menu.reset(new QMenu(this));
        if (!m_context_menu->isVisible())
        {
            QAction action_zoom_in_cursor("ZoomInCursor", m_context_menu.get());
            connect(&action_zoom_in_cursor, &QAction::triggered, [this]() { emit zoomInCursor(); });
            QAction action_zoom_out_cursor("ZoomOutCursor", m_context_menu.get());
            connect(&action_zoom_out_cursor, &QAction::triggered, [this]() { emit zoomOutCursor(); });
            QAction action_toggle_interation("ToggleInteraction", m_context_menu.get());
            connect(&action_toggle_interation, &QAction::triggered,
                    [this]() { setInteractionEnabled(!interactionEnabled()); });
            m_context_menu->addAction(&action_zoom_in_cursor);
            m_context_menu->addAction(&action_zoom_out_cursor);
            m_context_menu->addAction(&action_toggle_interation);
            m_context_menu->exec(pos);
        }
        else
            m_context_menu->hide();
    }
}

void QtImageViewer::hideContextMenu()
{
    if (m_context_menu) m_context_menu->hide();
}

void QtImageViewer::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    hideContextMenu();
}

// block native tab-switching so we can use it in shortcuts
bool QtImageViewer::focusNextPrevChild(bool next)
{
    return false;
}
