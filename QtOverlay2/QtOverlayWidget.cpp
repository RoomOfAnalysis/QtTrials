#include "QtOverlayWidget.h"

#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>

QtOverlayWidget::QtOverlayWidget(QtOverlayWidgetContainer* parent): QWidget(parent)
{
    connect(parent, &QtOverlayWidgetContainer::resized, this, &QtOverlayWidget::onContainerResized);

    m_opacity_effect = new QGraphicsOpacityEffect(this);
    m_opacity_effect->setOpacity(1.0);
    setGraphicsEffect(m_opacity_effect);

    m_fade_animation = new QPropertyAnimation(this, "opacity");
    m_fade_animation->setDuration(120);
    m_fade_animation->setStartValue(1.0f);
    m_fade_animation->setEndValue(0.0f);
    m_fade_animation->setEasingCurve(QEasingCurve::OutQuad);
    connect(m_fade_animation, &QPropertyAnimation::finished, [this]() { QWidget::hide(); });
}

QtOverlayWidget::~QtOverlayWidget()
{
    delete m_opacity_effect;
    delete m_fade_animation;
}

QSize QtOverlayWidget::containerSize() const
{
    return m_container_size;
}

void QtOverlayWidget::setContainerSize(QSize container_size)
{
    m_container_size = container_size;
    recalculateGeometry();
}

void QtOverlayWidget::onContainerResized(QSize container_size)
{
    setContainerSize(container_size);
}

qreal QtOverlayWidget::opacity() const
{
    return m_opacity_effect->opacity();
}

void QtOverlayWidget::setOpacity(qreal opacity)
{
    m_opacity_effect->setOpacity(opacity);
    update();
}

int QtOverlayWidget::horizontalMargin() const
{
    return m_horizontal_margin;
}

int QtOverlayWidget::verticalMargin() const
{
    return m_vertical_margin;
}

void QtOverlayWidget::setHorizontalMargin(int margin)
{
    m_horizontal_margin = margin;
    recalculateGeometry();
}

void QtOverlayWidget::setVerticalMargin(int margin)
{
    m_vertical_margin = margin;
    recalculateGeometry();
}

void QtOverlayWidget::setPosition(QtOverlayWidgetPosition pos)
{
    m_position = pos;
    recalculateGeometry();
}

void QtOverlayWidget::setFadeDuration(int duration)
{
    m_fade_animation->setDuration(duration);
}

void QtOverlayWidget::setFadeEnabled(bool flag)
{
    m_fade_enabled = flag;
}

void QtOverlayWidget::show()
{
    m_fade_animation->stop();
    m_opacity_effect->setOpacity(1.0);
    QWidget::show();
}

void QtOverlayWidget::hideAnimated()
{
    if (m_fade_enabled && !isHidden())
    {
        m_fade_animation->stop();
        m_fade_animation->start(QPropertyAnimation::KeepWhenStopped);
    }
    else
    {
        QWidget::hide();
        if (hasFocus() || isAncestorOf(qApp->focusWidget())) parentWidget()->setFocus();
    }
}

void QtOverlayWidget::hide()
{
    m_fade_animation->stop();

    QWidget::hide();
    if (hasFocus() || isAncestorOf(qApp->focusWidget())) parentWidget()->setFocus();
}

void QtOverlayWidget::paintEvent(QPaintEvent*)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void QtOverlayWidget::mousePressEvent(QMouseEvent* event)
{
    event->accept();
}

void QtOverlayWidget::mouseReleaseEvent(QMouseEvent* event)
{
    event->accept();
}

void QtOverlayWidget::wheelEvent(QWheelEvent* event)
{
    event->accept();
}

void QtOverlayWidget::recalculateGeometry()
{
    QRect rect = QRect(QPoint(0, 0), sizeHint());
    QPoint pos(0, 0);
    switch (m_position)
    {
    case QtOverlayWidgetPosition::TOPLEFT:
        pos.setX(m_horizontal_margin);
        pos.setY(m_vertical_margin);
        break;
    case QtOverlayWidgetPosition::TOP:
        pos.setX((containerSize().width() - rect.width()) / 2);
        pos.setY(m_vertical_margin);
        break;
    case QtOverlayWidgetPosition::TOPRIGHT:
        pos.setX(containerSize().width() - rect.width() - m_horizontal_margin);
        pos.setY(m_vertical_margin);
        break;
    case QtOverlayWidgetPosition::RIGHT:
        pos.setX(containerSize().width() - rect.width() - m_horizontal_margin);
        pos.setY((containerSize().height() - rect.height()) / 2);
        break;
    case QtOverlayWidgetPosition::BOTTOMRIGHT:
        pos.setX(containerSize().width() - rect.width() - m_horizontal_margin);
        pos.setY(containerSize().height() - rect.height() - m_vertical_margin);
        break;
    case QtOverlayWidgetPosition::BOTTOM:
        pos.setX((containerSize().width() - rect.width()) / 2);
        pos.setY(containerSize().height() - rect.height() - m_vertical_margin);
        break;
    case QtOverlayWidgetPosition::BOTTOMLEFT:
        pos.setX(m_horizontal_margin);
        pos.setY(containerSize().height() - rect.height() - m_vertical_margin);
        break;
    case QtOverlayWidgetPosition::LEFT:
        pos.setX(m_horizontal_margin);
        pos.setY((containerSize().height() - rect.height()) / 2);
        break;
    case QtOverlayWidgetPosition::CENTER:
        pos.setX((containerSize().width() - rect.width()) / 2);
        pos.setY((containerSize().height() - rect.height()) / 2);
    }
    rect.moveTopLeft(pos);
    setGeometry(rect);
}