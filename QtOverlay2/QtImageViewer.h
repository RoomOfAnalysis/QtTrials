#pragma once

#include "QtOverlayWidgetContainer.h"
#include "../QtImageViewer/ImageView.h"

#include <QTimer>
#include <QMenu>

class QtImageViewer: public QtOverlayWidgetContainer
{
    Q_OBJECT
public:
    explicit QtImageViewer(QWidget* parent = nullptr);
    ~QtImageViewer() = default;

    QRect imageRect();
    float currentScale();
    QSize sourceSize();

    void setInteractionEnabled(bool mode);
    bool interactionEnabled() const;

    bool showImage(QPixmap pixmap);
    void onScalingFinished(QPixmap scaled);
    bool isDisplaying() const;
    bool lockZoomEnabled() const;
    bool lockViewEnabled() const;

signals:
    void scalingRequested(QSize);
    void zoomIn();
    void zoomOut();
    void zoomInCursor();
    void zoomOutCursor();
    void scrollUp();
    void scrollDown();
    void scrollLeft();
    void scrollRight();
    void fitWindow();
    void fitWidth();
    void fitOriginal();
    void draggedOut();
    void toggleLockZoom();
    void toggleLockView();

public slots:
    void setFitMode(ImageFitMode mode);
    ImageFitMode fitMode() const;
    void closeImage();
    void hideCursor();
    void showCursor();
    void hideCursorTimed(bool restartTimer);

    void showContextMenu();
    void hideContextMenu();
    void showContextMenu(QPoint pos);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void hideEvent(QHideEvent* event) override;

    bool focusNextPrevChild(bool next) override;

private:
    std::unique_ptr<ImageView> m_img_view = nullptr;
    std::unique_ptr<QMenu> m_context_menu = nullptr;

    bool m_interaction_enabled = false;
    QTimer m_cursor_timer;
    const int CURSOR_HIDE_TIMEOUT_MS = 1000;
};