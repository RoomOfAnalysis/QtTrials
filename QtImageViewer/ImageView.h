#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

class QTimeLine;
class QTimer;
enum class ImageFitMode
{
    FIT_WINDOW,
    FIT_WIDTH,
    FIT_ORIGINAL,
    FIT_FREE
};

enum class MouseState
{
    NONE,
    DRAG_BEGIN,
    DRAG,
    PAN,
    ZOOM,
    WHEEL_ZOOM
};

enum class ViewLockMode
{
    NONE,
    LOCK_ZOOM,
    LOCK_ALL
};

class ImageView: public QGraphicsView
{
    Q_OBJECT
public:
    ImageView(QWidget* parent = nullptr);
    ~ImageView() = default;

    void setPixmap(QPixmap pixmap);
    void setScaledPixmap(QPixmap pixmap);
    void showPixmap(QPixmap pixmap);

    ImageFitMode fitMode() const;
    float scale() const;
    QSize sourceSize() const;
    QSize scaledSize() const;
    QRect scaledRect() const;

    bool isDisplaying() const;

    bool pixmapFit() const;
    bool scaledPixmapFit() const;

signals:
    void scalingRequested(QSize);
    void scaleChanged(qreal);
    void draggedOut();

public slots:
    void setFitMode(ImageFitMode mode);
    void setFitOriginal();
    void setFitWidth();
    void setFitWindow();
    void zoomIn();
    void zoomOut();
    void zoomInCursor();
    void zoomOutCursor();
    void scrollUp();
    void scrollDown();
    void scrollLeft();
    void scrollRight();
    void closeImage();
    void setExpandImage(bool flag);
    void show();
    void hide();

    void toggleLockZoom();
    bool lockZoomEnabled() const;
    void toggleLockView();
    bool lockViewEnabled() const;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void requestScaling();
    void scrollToX(int x);
    void scrollToY(int y);
    void centerOnPixmap();
    void onScrollTimelineFinished();

private:
    void zoomAnchored(float scale);
    void fitNormal();
    void fitWidth();
    void fitWindow();
    void fitFree(float scale);
    void swapToOriginalPixmap();

    void scroll(int dx, int dy);
    void mousePan(QMouseEvent* event);
    void mouseMoveZoom(QMouseEvent* event);
    void reset();
    void applyFitMode();

    void stopPosAnimation();
    void doZoom(float scale);
    void doZoom(bool at_cursor, bool zoom_in);
    void setZoomAnchor(QPoint viewport_pos);

    void centerIfNecessary();
    void snapToEdges();
    void updateFitWindowScale();
    void updateMinScale();
    void applySavedViewportPos();
    void saveViewportPos();
    void lockZoom();

    QPointF sceneRoundPos(QPointF scenePoint) const;
    QRectF sceneRoundRect(QRectF sceneRect) const;

private:
    int const SCROLL_SPEED = 150;
    int const SCROLL_UPDATE_RATE = 7;
    int const SCROLL_DISTANCE = 200;

    QGraphicsScene* m_scene = nullptr;
    QPixmap m_pixmap{}, m_scaled_pixmap{};
    QGraphicsPixmapItem m_pixmap_item{}, m_scaled_pixmap_item{};

    ImageFitMode m_image_fit_mode = ImageFitMode::FIT_WINDOW;
    bool m_expand_image = false;
    float m_fit_window_scale = 0.25f;
    float m_min_scale = 0.1f;
    ViewLockMode m_view_lock_mode = ViewLockMode::NONE;
    float m_locked_scale = 0.1f;

    QScrollBar* m_hs = nullptr;
    QScrollBar* m_vs = nullptr;
    qreal m_dpr{};
    QTimeLine *m_scroll_time_line_x = nullptr, *m_scroll_time_line_y = nullptr;

    MouseState m_mouse_state = MouseState::NONE;
    QPoint m_mouse_move_start_pos{}, m_mouse_press_pos{}, m_draw_pos{};
    int m_drag_threshold = 10;

    QTimer* m_scale_timer = nullptr;
    int m_zoom_threshold = 4;
    float m_zoom_step = 0.1f;
    QPair<QPointF, QPointF> m_zoom_anchor{}; // [pixmap coords, viewport coords]

    QPointF m_saved_viewport_pos{};
};