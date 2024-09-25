#include "QtOverlayWidgetContainer.h"
#include "QtOverlayWidget.h"

QtOverlayWidgetContainer::QtOverlayWidgetContainer(QWidget* parent): QWidget(parent)
{
    // make it can receive focusEvent
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void QtOverlayWidgetContainer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    emit resized(size());
}

// give focus to first visible overlay
void QtOverlayWidgetContainer::focusEvent(QFocusEvent* event)
{
    QWidget::focusInEvent(event);
    for (auto child : children())
        if (auto* overlay = qobject_cast<QtOverlayWidget*>(child); overlay && overlay->isVisible())
        {
            overlay->setFocus();
            break;
        }
}