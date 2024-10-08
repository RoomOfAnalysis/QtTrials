#include "QtImageHistOverlay.h"

#include "QtZoomableChartView.h"

#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QGridLayout>
#include <QApplication>

QtImageHistOverlay::QtImageHistOverlay(QtOverlayWidgetContainer* parent): QtOverlayWidget(parent), m_chart(new QChart)
{
    this->setFixedSize(QSize(500, 500));

    auto* chartView = new QtZoomableChartView(m_chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background-color:white");
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto axisX = new QValueAxis;
    axisX->setRange(0, 255);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("Intensity");
    auto axisY = new QValueAxis;
    axisY->setLabelFormat("%f");
    axisY->setRange(0, 256 * 10);
    axisY->setTitleText("Frequency");
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_chart->legend()->hide();
    m_chart->setTitle("Histogram");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    for (auto i = 0; i < 3; i++)
    {
        m_series_list[i] = new QLineSeries();
        m_chart->addSeries(m_series_list[i]);
        m_series_list[i]->attachAxis(axisX);
        m_series_list[i]->attachAxis(axisY);
    }
    m_series_list[0]->setColor(Qt::red);
    m_series_list[1]->setColor(Qt::green);
    m_series_list[2]->setColor(Qt::blue);

    auto gridLayout = new QGridLayout(this);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins({0, 0, 0, 0});
    gridLayout->addWidget(chartView, 0, 0);

    this->setPosition(QtOverlayWidgetPosition::LEFT);
    if (parent) setContainerSize(parent->size());
}

QtImageHistOverlay::~QtImageHistOverlay() = default;

void QtImageHistOverlay::setImage(QImage const& img)
{
    m_hist = CalcHist(img);
    float ymax = 0;
    for (auto i = 0; i < 3; i++)
        m_series_list[i]->clear();
    for (auto i = 0; i < 256; i++)
    {
        ymax = std::max({ymax, m_hist.red[i], m_hist.green[i], m_hist.blue[i]});
        m_series_list[0]->append(i, m_hist.red[i]);
        m_series_list[1]->append(i, m_hist.green[i]);
        m_series_list[2]->append(i, m_hist.blue[i]);
    }
    m_chart->axes(Qt::Vertical)[0]->setRange(0, ymax);
    qApp->processEvents();
    recalculateGeometry();
}

void QtImageHistOverlay::show()
{
    QtOverlayWidget::show();
    adjustSize();
    recalculateGeometry();
}