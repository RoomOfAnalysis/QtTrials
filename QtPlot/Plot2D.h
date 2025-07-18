#pragma once

#include <qwt/QwtPlot>
#include <qwt/QwtPlotCurve>
#include <qwt/QwtPlotZoomer>
#include <qwt/QwtPlotGrid>

class Plot2D: public QwtPlot
{
    Q_OBJECT

private:
    friend class Plot2DPrivate;
    QScopedPointer<Plot2DPrivate> d_ptr;
    Plot2DPrivate* d_func() { return (d_ptr.data()); }
    const Plot2DPrivate* d_func() const { return (d_ptr.data()); }

public:
    Plot2D(QString title = "", QWidget* parent = nullptr);
    virtual ~Plot2D();

    QwtPlotCurve* addCurve(const double* xData, const double* yData, int size, QColor color, QString title);
    QwtPlotCurve* addCurve(const QVector<QPointF>& xyDatas, QColor color, QString title);
    QwtPlotCurve* addCurve(const QVector<double>& xData, const QVector<double>& yData, QColor color, QString title);

    QList<QwtPlotCurve*> getCurveList();

    double axisXmin(int axisId = QwtPlot::xBottom) const;
    double axisXmax(int axisId = QwtPlot::xBottom) const;
    double axisYmin(int axisId = QwtPlot::yLeft) const;
    double axisYmax(int axisId = QwtPlot::yLeft) const;

    virtual void setEnableAllEditor(bool enable);

public slots:
    void enableZoomer(bool enable = true);

    void setZoomBase();

    void setZoomReset();

    // x 0.625
    void zoomIn();

    // x 1.6
    void zoomOut();

    void zoomInCompatible();

    void enableGrid(bool isShow = true);
    void enableGridX(bool enable = true);
    void enableGridY(bool enable = true);
    void enableGridXMin(bool enable = true);
    void enableGridYMin(bool enable = true);

    void enablePanner(bool enable = true);

    void enableLegend(bool enable = true);
    void enableLegendPanel(bool enable = true);

    void showItem(const QVariant& itemInfo, bool on);

signals:
    void enableZoomerChanged(bool enable);
    void enableGridChanged(bool enable);
    void enableGridXChanged(bool enable);
    void enableGridYChanged(bool enable);
    void enableGridXMinChanged(bool enable);
    void enableGridYMinChanged(bool enable);
    void enablePannerChanged(bool enable);
    void enableLegendChanged(bool enable);
    void enableLegendPanelChanged(bool enable);

public:
    bool isEnableZoomer() const;

    bool isEnableGrid() const;
    bool isEnableGridX() const;
    bool isEnableGridY() const;
    bool isEnableGridXMin() const;
    bool isEnableGridYMin() const;
    bool isEnablePanner() const;
    bool isEnableLegend() const;
    bool isEnableLegendPanel() const;

protected:
    virtual void resizeEvent(QResizeEvent*);

public:
    QwtPlotZoomer* zoomer();
    QwtPlotZoomer* zoomerSecond();

    QwtPlotGrid* grid();

protected:
    QwtPlotGrid* setupGrid(const QColor& color = Qt::gray, qreal width = 1.0, Qt::PenStyle style = Qt::DotLine);
    void deleteGrid();

    void setupZoomer();
    void deleteZoomer();
    void enableZoomer(QwtPlotZoomer* zoomer, bool enable);

    void setupPanner();
    void deletePanner();

    void setupLegend();
    void setuplegendPanel();
    void deletelegendPanel();
};
