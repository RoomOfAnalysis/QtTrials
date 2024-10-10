#pragma once

#include "ImageWidget.h"

class QScrollArea;
class CropRubberBand;

class ImageViewer: public QWidget
{
    Q_OBJECT
public:
    enum class ZoomMethods
    {
        Disable = 0,
        WidthAndHeight,
        Width,
        Height,
        Disprop
    };

    ImageViewer(QWidget* parent = nullptr);
    ~ImageViewer();

    void loadImage(QImage image);
    void resizeImage();
    void setCursorHiding(bool hide);
    void refresh();
    void reload();
    int getImageWidthPreCropped();
    int getImageHeightPreCropped();
    QPoint getContextMenuPosition();

public slots:
    void monitorCursorState();
    //void saveImage();
    //void saveImageAs();
    void copyImage();
    void pasteImage();
    void applyCropAndRotation();

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
    bool m_temp_disable_resize = false;
    QScrollArea* m_scroll_area;
    float m_image_zoom_factor = 1.f;
    ZoomMethods m_zoom_in_flags = ZoomMethods::WidthAndHeight;
    ZoomMethods m_zoom_out_flags = ZoomMethods::WidthAndHeight;
    bool m_flipH = false;
    bool m_flipV = false;
    qreal m_rotation = 0;
    int m_crop_left = 0, m_crop_top = 0, m_crop_width = 0, m_crop_height = 0;
    float m_scaled_width = 0, m_scaled_height = 0;
    bool m_mouse_rotate_enabled = false;

    ImageWidget* m_image_widget = nullptr;
    QImage m_orig_image;
    QImage m_viewer_image;
    QTimer* m_mouse_movement_timer = nullptr;

    bool m_cursor_is_hidden = false;
    bool m_move_image_locked = false;
    qreal m_initial_rotation = 0;
    int m_mouseX = 0;
    int m_mouseY = 0;
    int m_layoutX = 0;
    int m_layoutY = 0;

    QPoint m_crop_origin{};
    QPoint m_context_menu_position{};

    CropRubberBand* m_crop_rubber_band = nullptr;
};