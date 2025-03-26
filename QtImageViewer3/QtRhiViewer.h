// code from https://github.com/RealChuan/Qt-Graphics/blob/main/src/gpugraphics/rhiview.cc

#pragma once

#include <QRhiWidget>

class QtRhiViewer: public QRhiWidget
{
    Q_OBJECT
public:
    explicit QtRhiViewer(QWidget* parent = nullptr);
    ~QtRhiViewer();

public slots:
    void setImage(QImage image);
    void resetToOriginalSize();
    void fitToScreen();
    void rotateNinetieth();
    void anti_rotateNinetieth();

signals:
    void scaleFactorChanged(qreal scaleFactor);
    void imageSizeChanged(QSize const& imageSize);
    void imageChanged(QImage image);
    void rhiChanged(QString const& backendName);

protected:
    void initialize(QRhiCommandBuffer* cb) override;
    void render(QRhiCommandBuffer* cb) override;
    void releaseResources() override;

    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    // TODO: mousePressEvent + mouseMoveEvent to drag magnified image

private:
    class QtRhiViewerPrivate;
    QScopedPointer<QtRhiViewerPrivate> d_ptr;
};