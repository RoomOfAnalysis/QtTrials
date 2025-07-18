#include "Plot2D.h"

#include <qwt/QwtPlotPanner>
#include <qwt/QwtPlotLayout>
#include <qwt/QwtPlotCanvas>
#include <qwt/QwtScaleWidget>
#include <qwt/QwtPlotLegendItem>
#include <qwt/QwtLegend>
#include <qwt/QwtLegendLabel>
#include <qwt/QwtPlotMagnifier>

#include <QPen>

static void dataRange(const QwtPlot* chart, QwtInterval* yLeft, QwtInterval* yRight, QwtInterval* xBottom,
                      QwtInterval* xTop);

class LegendItem: public QwtPlotLegendItem
{
public:
    LegendItem()
    {
        setRenderHint(QwtPlotItem::RenderAntialiased);
        QColor color(Qt::white);
        setTextPen(color);
        setBorderPen(color);
        QColor c(Qt::gray);
        c.setAlpha(200);
        setBackgroundBrush(c);
    }
};

class Plot2DPrivate
{
    inline Plot2D* q_func() { return (static_cast<Plot2D*>(q_ptr)); }
    inline const Plot2D* q_func() const { return (static_cast<const Plot2D*>(q_ptr)); }
    friend class Plot2D;
    Plot2D* q_ptr;

public:
    QScopedPointer<QwtPlotZoomer> m_zoomer;
    QScopedPointer<QwtPlotZoomer> m_zoomerSecond;
    QwtPlotGrid* m_grid;
    QwtPlotPanner* m_panner;
    LegendItem* m_legend;
    QwtLegend* m_legendPanel;

    Plot2DPrivate(Plot2D* p): q_ptr(p), m_grid(nullptr), m_panner(nullptr), m_legend(nullptr), m_legendPanel(nullptr) {}
};

Plot2D::Plot2D(QString title, QWidget* parent): QwtPlot(title, parent), d_ptr(new Plot2DPrivate(this))
{
    setAutoReplot(false);
    setAutoFillBackground(true);

    QwtPlotLayout* pLayout = plotLayout();
    pLayout->setCanvasMargin(0);
    pLayout->setAlignCanvasToScales(true);

    auto* pCanvas = new QwtPlotCanvas();
    pCanvas->setFrameStyle(QFrame::Box | QFrame::Plain);
    pCanvas->setLineWidth(2);
    pCanvas->setBorderRadius(0);
    pCanvas->setCursor(Qt::ArrowCursor);
    pCanvas->setFocusPolicy(Qt::ClickFocus);
    pCanvas->setFocusProxy(this);
    pCanvas->setStyleSheet("background-color: transparent;");
    setCanvas(pCanvas);
    setFocusPolicy(Qt::ClickFocus);
    setFrameShape(QFrame::Box);
    setLineWidth(0);
    setAutoReplot(true);

    QwtScaleWidget* ax = axisWidget(QwtPlot::yLeft);
    if (ax) ax->setMargin(0);

    ax = axisWidget(QwtPlot::xBottom);
    if (ax) ax->setMargin(0);
}

Plot2D::~Plot2D() {}

QwtPlotCurve* Plot2D::addCurve(const double* xData, const double* yData, int size, QColor color, QString title)
{
    if (size <= 0) return (nullptr);
    QScopedPointer<QwtPlotCurve> pCurve(new QwtPlotCurve(title));
    pCurve->setYAxis(yLeft);
    pCurve->setXAxis(xBottom);
    pCurve->setStyle(QwtPlotCurve::Lines);
    pCurve->setSamples(xData, yData, size);
    pCurve->setPen(color, 2.);
    pCurve->attach(this);
    return (pCurve.take());
}

QwtPlotCurve* Plot2D::addCurve(const QVector<QPointF>& xyDatas, QColor color, QString title)
{
    if (xyDatas.empty()) return (nullptr);
    QScopedPointer<QwtPlotCurve> pCurve(new QwtPlotCurve(title));
    pCurve->setYAxis(yLeft);
    pCurve->setXAxis(xBottom);
    pCurve->setStyle(QwtPlotCurve::Lines);
    pCurve->setSamples(xyDatas);
    pCurve->setPen(color, 2.);
    pCurve->attach(this);
    return (pCurve.take());
}

QwtPlotCurve* Plot2D::addCurve(const QVector<double>& xData, const QVector<double>& yData, QColor color, QString title)
{
    if (xData.empty() || xData.size() != yData.size()) return (nullptr);
    QScopedPointer<QwtPlotCurve> pCurve(new QwtPlotCurve(title));
    pCurve->setYAxis(yLeft);
    pCurve->setXAxis(xBottom);
    pCurve->setStyle(QwtPlotCurve::Lines);
    pCurve->setSamples(xData, yData);
    pCurve->setPen(color, 2.);
    pCurve->attach(this);
    return (pCurve.take());
}

void Plot2D::resizeEvent(QResizeEvent* event)
{
    QwtPlot::resizeEvent(event);
}

QwtPlotZoomer* Plot2D::zoomer()
{
    Plot2DPrivate* d = d_func();
    return (d->m_zoomer.data());
}

QwtPlotZoomer* Plot2D::zoomerSecond()
{
    Plot2DPrivate* d = d_func();
    return (d->m_zoomerSecond.data());
}

QwtPlotGrid* Plot2D::grid()
{
    Plot2DPrivate* d = d_func();
    return (d->m_grid);
}

bool Plot2D::isEnableGrid() const
{
    Plot2DPrivate const* d = d_func();
    if (d->m_grid) return (d->m_grid->isVisible());
    return (false);
}

bool Plot2D::isEnableGridX() const
{
    Plot2DPrivate const* d = d_func();
    if (d->m_grid)
    {
        if (d->m_grid->isVisible()) return (d->m_grid->xEnabled());
    }
    return (false);
}

bool Plot2D::isEnableGridY() const
{
    Plot2DPrivate const* d = d_func();
    if (d->m_grid)
    {
        if (d->m_grid->isVisible()) return (d->m_grid->yEnabled());
    }
    return (false);
}

bool Plot2D::isEnableGridXMin() const
{
    Plot2DPrivate const* d = d_func();
    if (d->m_grid)
    {
        if (d->m_grid->isVisible()) return (d->m_grid->xMinEnabled());
    }
    return (false);
}

bool Plot2D::isEnableGridYMin() const
{
    Plot2DPrivate const* d = d_func();
    if (d->m_grid)
    {
        if (d->m_grid->isVisible()) return (d->m_grid->yMinEnabled());
    }
    return (false);
}

bool Plot2D::isEnablePanner() const
{
    Plot2DPrivate const* d = d_func();
    if (d->m_panner) return (d->m_panner->isEnabled());
    return (false);
}

bool Plot2D::isEnableLegend() const
{
    Plot2DPrivate const* d = d_func();
    if (d->m_legend) return (d->m_legend->isVisible());
    return (false);
}

bool Plot2D::isEnableLegendPanel() const
{
    Plot2DPrivate const* d = d_func();
    if (d->m_legendPanel) return (d->m_legendPanel->isVisible());
    return (false);
}

QwtPlotGrid* Plot2D::setupGrid(const QColor& color, qreal width, Qt::PenStyle style)
{
    Plot2DPrivate* d = d_func();
    bool bShouldAttachAgain(false);

    if (nullptr == d->m_grid)
    {
        d->m_grid = new QwtPlotGrid;
        bShouldAttachAgain = true;
    }
    d->m_grid->setMajorPen(color, width, style);
    d->m_grid->setMinorPen(color, 0, Qt::DotLine);
    if (bShouldAttachAgain) d->m_grid->attach(this);
    return (d->m_grid);
}

void Plot2D::deleteGrid()
{
    Plot2DPrivate* d = d_func();
    if (nullptr == d->m_grid) return;
    d->m_grid->detach();
    delete d->m_grid;
    d->m_grid = nullptr;
    replot();
}

void Plot2D::enableGrid(bool isShow)
{
    Plot2DPrivate* d = d_func();
    if (isShow)
    {
        if (nullptr == d->m_grid) setupGrid();
        d->m_grid->enableX(true);
        d->m_grid->enableY(true);
        d->m_grid->show();
        emit enableGridXChanged(isShow);
        emit enableGridYChanged(isShow);
        emit enableGridChanged(isShow);
        return;
    }
    else
    {
        if (nullptr == d->m_grid) return;
        d->m_grid->hide();
    }
    replot();
    emit enableGridChanged(isShow);
}

void Plot2D::enableGridX(bool enable)
{
    Plot2DPrivate* d = d_func();
    if (nullptr == d->m_grid) return;
    d->m_grid->enableX(enable);
    emit enableGridXChanged(enable);

    if (!enable) emit enableGridXMinChanged(false);
}

void Plot2D::enableGridY(bool enable)
{
    Plot2DPrivate* d = d_func();
    if (nullptr == d->m_grid) return;
    d->m_grid->enableY(enable);
    emit enableGridYChanged(enable);

    if (!enable) emit enableGridYMinChanged(false);
}

void Plot2D::enableGridXMin(bool enable)
{
    Plot2DPrivate* d = d_func();
    if (nullptr == d->m_grid) return;
    d->m_grid->enableXMin(enable);
    emit enableGridXMinChanged(enable);
}

void Plot2D::enableGridYMin(bool enable)
{
    Plot2DPrivate* d = d_func();
    if (nullptr == d->m_grid) return;
    d->m_grid->enableYMin(enable);
    emit enableGridYMinChanged(enable);
}

double Plot2D::axisXmin(int axisId) const
{
    QwtInterval inl = axisInterval(axisId);

    if (inl.isValid()) return (inl.minValue());
    axisId = ((axisId == QwtPlot::xBottom) ? QwtPlot::xTop : QwtPlot::xBottom);
    inl = axisInterval(axisId);
    if (inl.isValid()) return (inl.minValue());
    return (double());
}

double Plot2D::axisXmax(int axisId) const
{
    QwtInterval inl = axisInterval(axisId);

    if (inl.isValid()) return (inl.maxValue());
    axisId = ((axisId == QwtPlot::xBottom) ? QwtPlot::xTop : QwtPlot::xBottom);
    inl = axisInterval(axisId);
    if (inl.isValid()) return (inl.maxValue());
    return (double());
}

double Plot2D::axisYmin(int axisId) const
{
    QwtInterval inl = axisInterval(axisId);

    if (inl.isValid()) return (inl.minValue());
    axisId = ((axisId == QwtPlot::yLeft) ? QwtPlot::yRight : QwtPlot::yLeft);
    inl = axisInterval(axisId);
    if (inl.isValid()) return (inl.minValue());
    return (double());
}

double Plot2D::axisYmax(int axisId) const
{
    QwtInterval inl = axisInterval(axisId);

    if (inl.isValid()) return (inl.maxValue());
    axisId = ((axisId == QwtPlot::yLeft) ? QwtPlot::yRight : QwtPlot::yLeft);
    inl = axisInterval(axisId);
    if (inl.isValid()) return (inl.maxValue());
    return (double());
}

void Plot2D::setEnableAllEditor(bool enable)
{
    if (!isEnableZoomer()) enableZoomer(enable);
    if (!isEnablePanner()) enablePanner(enable);
}

void Plot2D::enablePanner(bool enable)
{
    Plot2DPrivate* d = d_func();
    if (enable)
    {
        if (nullptr == d->m_panner) setupPanner();
        d->m_panner->setEnabled(enable);
    }
    else
    {
        if (nullptr != d->m_panner) d->m_panner->setEnabled(enable);
    }
    emit enablePannerChanged(enable);
}

void Plot2D::setupPanner()
{
    Plot2DPrivate* d = d_func();
    if (nullptr == d->m_panner)
    {
        d->m_panner = new QwtPlotPanner(canvas());
        d->m_panner->setCursor(QCursor(Qt::ClosedHandCursor));
        d->m_panner->setMouseButton(Qt::RightButton);
    }
}

void Plot2D::deletePanner()
{
    Plot2DPrivate* d = d_func();
    if (nullptr != d->m_panner)
    {
        d->m_panner->setParent(nullptr);
        delete d->m_panner;
        d->m_panner = nullptr;
    }
}

void Plot2D::enableZoomer(bool enable)
{
    Plot2DPrivate* d = d_func();
    if (!enable)
    {
        if (d->m_zoomer.isNull()) return;
    }
    if (d->m_zoomer.isNull()) setupZoomer();
    enableZoomer(d->m_zoomer.data(), enable);
    enableZoomer(d->m_zoomerSecond.data(), enable);
    emit enableZoomerChanged(enable);
}

void Plot2D::enableZoomer(QwtPlotZoomer* zoomer, bool enable)
{
    if (nullptr == zoomer) return;
    if (enable)
    {
        zoomer->setEnabled(true);
        zoomer->setZoomBase(true);
        zoomer->setRubberBand(QwtPicker::RectRubberBand);
    }
    else
    {
        zoomer->setEnabled(false);
        zoomer->setRubberBand(QwtPicker::NoRubberBand);
        zoomer->setTrackerMode(QwtPicker::AlwaysOff);
    }
}

void Plot2D::setZoomBase()
{
    Plot2DPrivate* d = d_func();
    if (!d->m_zoomer.isNull()) d->m_zoomer->setZoomBase(true);
    if (!d->m_zoomerSecond.isNull()) d->m_zoomerSecond->setZoomBase(true);
}

void Plot2D::setupZoomer()
{
    Plot2DPrivate* d = d_func();
    if (d->m_zoomer.isNull())
    {
        d->m_zoomer.reset(new QwtPlotZoomer(xBottom, yLeft, canvas()));
        d->m_zoomer->setKeyPattern(QwtEventPattern::KeyRedo, Qt::Key_I, Qt::ShiftModifier);
        d->m_zoomer->setKeyPattern(QwtEventPattern::KeyUndo, Qt::Key_O, Qt::ShiftModifier);
        d->m_zoomer->setKeyPattern(QwtEventPattern::KeyHome, Qt::Key_Home);
        d->m_zoomer->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        d->m_zoomer->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
        d->m_zoomer->setTrackerMode(QwtPicker::AlwaysOff);
        d->m_zoomer->setZoomBase(false);
        d->m_zoomer->setMaxStackDepth(20);
    }
    if (nullptr == d->m_zoomerSecond)
    {
        d->m_zoomerSecond.reset(new QwtPlotZoomer(xTop, yRight, canvas()));
        d->m_zoomerSecond->setKeyPattern(QwtEventPattern::KeyRedo, Qt::Key_I, Qt::ShiftModifier);
        d->m_zoomerSecond->setKeyPattern(QwtEventPattern::KeyUndo, Qt::Key_O, Qt::ShiftModifier);
        d->m_zoomerSecond->setKeyPattern(QwtEventPattern::KeyHome, Qt::Key_Home);
        d->m_zoomerSecond->setMousePattern(QwtEventPattern::MouseSelect2, Qt::RightButton, Qt::ControlModifier);
        d->m_zoomerSecond->setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton);
        d->m_zoomerSecond->setTrackerMode(QwtPicker::AlwaysOff);
        d->m_zoomerSecond->setZoomBase(false);
        d->m_zoomerSecond->setMaxStackDepth(20);
    }
    QwtPlotMagnifier* magnifier = new QwtPlotMagnifier(canvas());
    magnifier->setMouseButton(Qt::NoButton);
}

void Plot2D::deleteZoomer()
{
    Plot2DPrivate* d = d_func();
    if (!d->m_zoomer.isNull()) d->m_zoomer.reset();
    if (!d->m_zoomerSecond.isNull()) d->m_zoomerSecond.reset();
}

void Plot2D::setZoomReset()
{
    Plot2DPrivate* d = d_func();
    if (!d->m_zoomer.isNull()) d->m_zoomer->zoom(0);
    if (!d->m_zoomerSecond.isNull()) d->m_zoomerSecond->zoom(0);
}

void Plot2D::zoomIn()
{
    Plot2DPrivate* d = d_func();
    if (d->m_zoomer.isNull()) setupZoomer();

    QRectF rect = d->m_zoomer->zoomRect();
    double w = rect.width() * 0.625;
    double h = rect.height() * 0.625;
    double x = rect.x() + (rect.width() - w) / 2.0;
    double y = rect.y() + (rect.height() - h) / 2.0;

    rect.setX(x);
    rect.setY(y);
    rect.setWidth(w);
    rect.setHeight(h);

    d->m_zoomer->zoom(rect);
}

void Plot2D::zoomOut()
{
    Plot2DPrivate* d = d_func();
    if (d->m_zoomer.isNull()) setupZoomer();

    QRectF rect = d->m_zoomer->zoomRect();
    double w = rect.width() * 1.6;
    double h = rect.height() * 1.6;
    double x = rect.x() - (w - rect.width()) / 2.0;
    double y = rect.y() - (h - rect.height()) / 2.0;

    rect.setX(x);
    rect.setY(y);
    rect.setWidth(w);
    rect.setHeight(h);
    d->m_zoomer->zoom(rect);
}

void Plot2D::zoomInCompatible()
{
    Plot2DPrivate* d = d_func();
    QwtInterval intv[axisCnt];

    ::dataRange(this, &intv[yLeft], &intv[yRight], &intv[xBottom], &intv[xTop]);
    if (!d->m_zoomer.isNull())
    {
        int axx = d->m_zoomer->xAxis();
        int axy = d->m_zoomer->yAxis();
        QRectF rect1;
        rect1.setRect(intv[axx].minValue(), intv[axy].minValue(), intv[axx].width(), intv[axy].width());
        d->m_zoomer->zoom(rect1);
    }
    if (!d->m_zoomerSecond.isNull())
    {
        int axx = d->m_zoomerSecond->xAxis();
        int axy = d->m_zoomerSecond->yAxis();
        QRectF rect1;
        rect1.setRect(intv[axx].minValue(), intv[axy].minValue(), intv[axx].width(), intv[axy].width());
        d->m_zoomerSecond->zoom(rect1);
    }
}

void Plot2D::setupLegend()
{
    Plot2DPrivate* d = d_func();
    if (nullptr == d->m_legend)
    {
        d->m_legend = new LegendItem();
        d->m_legend->attach(this);
    }
}

void Plot2D::enableLegend(bool enable)
{
    Plot2DPrivate* d = d_func();
    if (enable)
        if (d->m_legend)
            d->m_legend->setVisible(true);
        else
            setupLegend();
    else if (d->m_legend)
        d->m_legend->setVisible(false);
    emit enableLegendChanged(enable);
}

void Plot2D::enableLegendPanel(bool enable)
{
    if (enable)
        setuplegendPanel();
    else
        deletelegendPanel();
    emit enableLegendPanelChanged(enable);
}

void Plot2D::setuplegendPanel()
{
    Plot2DPrivate* d = d_func();
    if (d->m_legendPanel) return;
    d->m_legendPanel = new QwtLegend;
    d->m_legendPanel->setDefaultItemMode(QwtLegendData::Checkable);
    insertLegend(d->m_legendPanel, QwtPlot::RightLegend);
    connect(d->m_legendPanel, SIGNAL(checked(const QVariant&, bool, int)), SLOT(showItem(const QVariant&, bool)));

    QwtPlotItemList items = itemList(QwtPlotItem::Rtti_PlotCurve);

    for (int i = 0; i < items.size(); i++)
    {
        const QVariant itemInfo = itemToInfo(items[i]);
        QwtLegendLabel* legendLabel = qobject_cast<QwtLegendLabel*>(d->m_legendPanel->legendWidget(itemInfo));
        if (legendLabel) legendLabel->setChecked(items[i]->isVisible());
    }
}

void Plot2D::deletelegendPanel()
{
    Plot2DPrivate* d = d_func();
    insertLegend(nullptr);
    d->m_legendPanel = nullptr;
}

bool Plot2D::isEnableZoomer() const
{
    Plot2DPrivate const* d = d_func();
    if (d->m_zoomer) return (d->m_zoomer->isEnabled());
    if (d->m_zoomerSecond) return (d->m_zoomerSecond->isEnabled());
    return (false);
}

void Plot2D::showItem(const QVariant& itemInfo, bool on)
{
    QwtPlotItem* plotItem = infoToItem(itemInfo);

    if (plotItem) plotItem->setVisible(on);
}

QList<QwtPlotCurve*> Plot2D::getCurveList()
{
    QList<QwtPlotCurve*> curves;
    QwtPlotItemList items = itemList(QwtPlotItem::Rtti_PlotCurve);

    for (int i(0); i < items.size(); ++i)
        curves.append(static_cast<QwtPlotCurve*>(items[i]));
    return (curves);
}

void dataRange(const QwtPlot* chart, QwtInterval* yLeft, QwtInterval* yRight, QwtInterval* xBottom, QwtInterval* xTop)
{
    QwtInterval intv[QwtPlot::axisCnt];
    QwtPlotItemList itemList = chart->itemList();
    QwtPlotItemIterator it;
    for (it = itemList.begin(); it != itemList.end(); ++it)
    {
        const QwtPlotItem* item = *it;
        if (!item->isVisible()) continue;
        QRectF rect;
        if (item->rtti() == QwtPlotItem::Rtti_PlotCurve)
        {
            const QwtPlotCurve* p = static_cast<const QwtPlotCurve*>(item);
            rect = p->dataRect();
        }
        if (rect.width() >= 0.0) intv[item->xAxis()] |= QwtInterval(rect.left(), rect.right());
        if (rect.height() >= 0.0) intv[item->yAxis()] |= QwtInterval(rect.top(), rect.bottom());
    }
    if (yLeft) *yLeft = intv[QwtPlot::yLeft];
    if (yRight) *yRight = intv[QwtPlot::yRight];
    if (xTop) *xTop = intv[QwtPlot::xTop];
    if (xBottom) *xBottom = intv[QwtPlot::xBottom];
}
