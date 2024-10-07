#pragma once

#include "QtOverlayWidget.h"
#include "Histogram.h"

class QChart;
class QLineSeries;

class QtImageHistOverlay: public QtOverlayWidget
{
    Q_OBJECT
public:
    explicit QtImageHistOverlay(QtOverlayWidgetContainer* parent = nullptr);
    ~QtImageHistOverlay();

    void setImage(QImage const& img);

public slots:
    void show();

private:
    QChart* m_chart = nullptr;
    QLineSeries* m_series_list[3]{};
    Histogram m_hist{};
};
