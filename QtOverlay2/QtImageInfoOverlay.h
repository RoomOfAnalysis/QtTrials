#pragma once

#include "QtOverlayWidget.h"

#include <QLabel>

namespace Ui
{
    class QtImageInfoOverlay;
}
class KVItem: public QWidget
{
    Q_OBJECT
public:
    explicit KVItem(QWidget* parent = nullptr);
    void setKV(QString key, QString value);

protected:
    void paintEvent(QPaintEvent* event);

private:
    QLabel m_key_label, m_value_label;
};

class QtImageInfoOverlay: public QtOverlayWidget
{
    Q_OBJECT
public:
    explicit QtImageInfoOverlay(QtOverlayWidgetContainer* parent = nullptr);
    ~QtImageInfoOverlay();

    void setExifInfo(QMap<QString, QString> const&);

public slots:
    void show();

protected:
    void wheelEvent(QWheelEvent*) override;

private:
    Ui::QtImageInfoOverlay* ui;
    QList<KVItem*> m_items;
    QLabel m_item_stub;
};