#include <QApplication>

#include "Plot2D.h"

// please refer to https://sourceforge.net/p/qwt/git/ci/develop/tree/examples/ for more functionalities
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Plot2D plot("test plot");
    plot.setStyleSheet("background-color: #19232D; \
                        border: 1px solid #32414B; \
                        color: #F0F0F0; \
                        border-radius: 4px;");
    double x[]{0, 1, 2, 3, 4, 5, 6};
    double y[]{6, 3, 5, 2, 4, 1, 0};
    plot.addCurve(x, y, 7, Qt::red, "test curve");
    plot.setAxisScale(QwtPlot::yLeft, 0, 10);
    plot.enableAxis(QwtPlot::yLeft);
    plot.enableAxis(QwtPlot::xBottom);
    plot.enableGrid();
    plot.setEnableAllEditor(true);
    plot.enableLegend();
    plot.enableLegendPanel();
    plot.show();

    return app.exec();
}