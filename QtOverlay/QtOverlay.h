#pragma once

#include <functional>

#include <QWidget>
#include <QPointer>

class QtOverlayFactoryFilter: public QObject
{
    Q_OBJECT

    class QtOverlay: public QWidget
    {
    public:
        explicit QtOverlay(QWidget* parent = nullptr);

        void setPaintFunc(std::function<void(QPainter*)> func);

    protected:
        void paintEvent(QPaintEvent*) override;

    private:
        std::function<void(QPainter*)> m_paint_func = nullptr;
    };

public:
    explicit QtOverlayFactoryFilter(std::function<void(QPainter*)> paint_func, QObject* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    QPointer<QtOverlay> m_overlay;
    std::function<void(QPainter*)> m_paint_func = nullptr;
};