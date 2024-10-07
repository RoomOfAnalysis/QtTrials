#include <QApplication>
#include <qwt/qwt_plot.h>
#include <qwt/qwt_plot_curve.h>
#include <qwt/qwt_plot_zoomer.h>

// please refer to https://sourceforge.net/p/qwt/git/ci/develop/tree/examples/ for more functionalities
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QwtPlot plot;
    QwtPlotZoomer zoomer(plot.canvas());
    QwtPlotCurve curve;
    double x[]{0, 1, 2, 3, 4, 5, 6};
    double y[]{6, 3, 5, 2, 4, 1, 0};
    curve.setSamples(x, y, 7);
    curve.attach(&plot);
    plot.setAxisScale(QwtPlot::yLeft, 0, 10);
    plot.show();

    return app.exec();
}