#pragma once

#include <QWidget>

class QtOverlayWidgetContainer: public QWidget
{
    Q_OBJECT
public:
    explicit QtOverlayWidgetContainer(QWidget* parent = nullptr);

signals:
    void resized(QSize);

protected:
    void resizeEvent(QResizeEvent*) override;
    void focusEvent(QFocusEvent*);
};