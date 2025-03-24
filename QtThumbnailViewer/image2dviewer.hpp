#pragma once

#include <QWidget>

class QScrollArea;

class Image2DViewer: public QWidget
{
    Q_OBJECT
public:
    Image2DViewer(QWidget* parent = nullptr);
    ~Image2DViewer();

    void loadImage(QImage image);
    void resizeImage();
    void setCursorHiding(bool hide);
    void refresh();
    void reload();
    void flipH();
    void flipV();

    QImage getOriginImage() const;
    QImage getViewerImage() const;

    void saveImage();

signals:
    void imageChanged(QImage);
    void imageSaved(QString);

public slots:
    void monitorCursorState();
    void copyImage();
    void pasteImage();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void setMouseMoveData(bool lockMove, int lMouseX, int lMouseY);
    void centerImage(QSize& imgSize);
    void transform();
    void setImage(const QImage& image);

private:
    class ImageWidget;

private:
    bool m_temp_disable_resize = false;
    QScrollArea* m_scroll_area = nullptr;
    ImageWidget* m_image_widget = nullptr;
    float m_image_zoom_factor = 1.f;
    QTimer* m_mouse_movement_timer = nullptr;
    bool m_cursor_is_hidden = false;
    bool m_move_image_locked = false;
    int m_mouseX = 0;
    int m_mouseY = 0;
    int m_layoutX = 0;
    int m_layoutY = 0;
    bool m_flipH = false;
    bool m_flipV = false;
    QImage m_orig_image{};
    QImage m_viewer_image{};
    QMenu* m_context_menu = nullptr;
};