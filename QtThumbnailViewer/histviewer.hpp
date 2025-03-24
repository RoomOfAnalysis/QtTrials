#pragma once

#include <QWidget>
#include "histogram.hpp"

class QChart;
class QLineSeries;

class HistViewer: public QWidget
{
    Q_OBJECT
public:
    explicit HistViewer(QWidget* parent = nullptr);
    ~HistViewer();

    void setImage(QImage const& img);
    void removeZeroPoint(bool flag);
    void setDarkTheme();
    void setLightTheme();

private:
    void updateSeries();

private:
    QChart* m_chart = nullptr;
    QLineSeries* m_series_list[3]{};
    Histogram m_hist{};
    bool m_remove_lh = false;
    int m_low = 0;
    int m_high = 255;
};