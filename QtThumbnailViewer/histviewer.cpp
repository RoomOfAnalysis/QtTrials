#include "histviewer.hpp"

#include "zcview.hpp"

#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QGridLayout>
#include <QApplication>
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>

HistViewer::HistViewer(QWidget* parent): QWidget(parent), m_chart(new QChart)
{
    auto* chartView = new ZCView(m_chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto axisX = new QValueAxis;
    axisX->setRange(0, 255);
    axisX->setLabelFormat("%d");
    axisX->setTitleText("Intensity");
    auto axisY = new QValueAxis;
    axisY->setLabelFormat("%.1f");
    axisY->setRange(0, 255);
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

    auto* zCB = new QCheckBox();
    zCB->setChecked(false);
    auto* rCB = new QCheckBox();
    rCB->setChecked(true);
    auto* gCB = new QCheckBox();
    gCB->setChecked(true);
    auto* bCB = new QCheckBox();
    bCB->setChecked(true);

    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins({0, 0, 0, 0});
    mainLayout->addWidget(chartView, 8);

    auto cbLayout = new QHBoxLayout();
    cbLayout->addWidget(zCB);
    cbLayout->addWidget(new QLabel("RemoveLowHigh"), 1);
    cbLayout->addWidget(rCB);
    cbLayout->addWidget(new QLabel("Red"), 1);
    cbLayout->addWidget(gCB);
    cbLayout->addWidget(new QLabel("Green"), 1);
    cbLayout->addWidget(bCB);
    cbLayout->addWidget(new QLabel("Blue"), 1);

    auto sbLayout = new QHBoxLayout();
    auto* lsb = new QSpinBox();
    lsb->setPrefix("Low: ");
    lsb->setMinimum(0);
    lsb->setMaximum(255);
    lsb->setValue(0);
    sbLayout->addWidget(lsb, 1);
    auto* hsb = new QSpinBox();
    hsb->setPrefix("High: ");
    hsb->setMinimum(0);
    hsb->setMaximum(255);
    hsb->setValue(255);
    sbLayout->addWidget(hsb, 1);

    mainLayout->addLayout(cbLayout, 1);
    mainLayout->addLayout(sbLayout, 1);

    connect(zCB, &QCheckBox::stateChanged, [this](int state) {
        removeZeroPoint(state == Qt::Checked);
        updateSeries();
    });
    connect(rCB, &QCheckBox::stateChanged, [this](int state) {
        if (state == Qt::Checked)
            m_series_list[0]->show();
        else
            m_series_list[0]->hide();
    });
    connect(gCB, &QCheckBox::stateChanged, [this](int state) {
        if (state == Qt::Checked)
            m_series_list[1]->show();
        else
            m_series_list[1]->hide();
    });
    connect(bCB, &QCheckBox::stateChanged, [this](int state) {
        if (state == Qt::Checked)
            m_series_list[2]->show();
        else
            m_series_list[2]->hide();
    });

    connect(lsb, &QSpinBox::valueChanged, [this, hsb](int value) {
        m_low = value;
        hsb->setMinimum(value);
        if (m_remove_lh) updateSeries();
    });
    connect(hsb, &QSpinBox::valueChanged, [this, lsb](int value) {
        m_high = value;
        lsb->setMaximum(value);
        if (m_remove_lh) updateSeries();
    });
}

HistViewer::~HistViewer() = default;

void HistViewer::setImage(QImage const& img)
{
    m_hist = CalcHist(img);
    updateSeries();
}

void HistViewer::removeZeroPoint(bool flag)
{
    m_remove_lh = flag;
}

void HistViewer::setDarkTheme()
{
    m_chart->setTheme(QChart::ChartThemeDark);
    m_series_list[0]->setColor(Qt::red);
    m_series_list[1]->setColor(Qt::green);
    m_series_list[2]->setColor(Qt::blue);
}

void HistViewer::setLightTheme()
{
    m_chart->setTheme(QChart::ChartThemeLight);
    m_series_list[0]->setColor(Qt::red);
    m_series_list[1]->setColor(Qt::green);
    m_series_list[2]->setColor(Qt::blue);
}

void HistViewer::updateSeries()
{
    float ymax = 0;
    for (auto i = 0; i < 3; i++)
        m_series_list[i]->clear();
    auto s = m_remove_lh ? m_low : 0;
    auto e = m_remove_lh ? m_high : 256;
    for (auto i = s; i < e; i++)
    {
        ymax = std::max({ymax, m_hist.red[i], m_hist.green[i], m_hist.blue[i]});
        // no zero y
        if (m_hist.red[i]) m_series_list[0]->append(i, m_hist.red[i]);
        if (m_hist.green[i]) m_series_list[1]->append(i, m_hist.green[i]);
        if (m_hist.blue[i]) m_series_list[2]->append(i, m_hist.blue[i]);
    }
    m_chart->axes(Qt::Vertical)[0]->setRange(0, ymax);
    QApplication::processEvents();
}
