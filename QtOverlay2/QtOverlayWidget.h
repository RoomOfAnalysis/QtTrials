#pragma once

#include "QtOverlayWidgetContainer.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

enum class QtOverlayWidgetPosition
{
    TOPLEFT,
    TOP,
    TOPRIGHT,
    RIGHT,
    BOTTOMRIGHT,
    BOTTOM,
    BOTTOMLEFT,
    LEFT,
    CENTER
};

class QtOverlayWidget: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
public:
    explicit QtOverlayWidget(QtOverlayWidgetContainer* parent);
    ~QtOverlayWidget();

    QSize containerSize() const;
    int horizontalMargin() const;
    int verticalMargin() const;

    void setHorizontalMargin(int);
    void setVerticalMargin(int);
    void setPosition(QtOverlayWidgetPosition pos);
    void setFadeDuration(int duration);
    void setFadeEnabled(bool flag);

public slots:
    void show();
    void hide();
    void hideAnimated();

protected:
    // called whenever container rectangle changes
    virtual void recalculateGeometry();
    void setContainerSize(QSize);

    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    void onContainerResized(QSize);

private slots:
    void setOpacity(qreal opacity);
    qreal opacity() const;

protected:
    QtOverlayWidgetPosition m_position = QtOverlayWidgetPosition::CENTER;

private:
    QSize m_container_size{};
    QGraphicsOpacityEffect* m_opacity_effect = nullptr;
    int m_horizontal_margin = 10, m_vertical_margin = 10;
    bool m_fade_enabled = false;
    QPropertyAnimation* m_fade_animation = nullptr;
};