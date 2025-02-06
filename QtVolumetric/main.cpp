#include <QApplication>
#include <Q3DScatterWidgetItem>
#include <QQuickWidget>
#include <QQmlComponent>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Q3DScatterWidgetItem* graph = new Q3DScatterWidgetItem;
    auto quickWidget = new QQuickWidget;
    // FIXME: always failed here with ERROR: QQmlComponent: Component is not ready
    // even the [official example](https://github.com/qt/qtgraphs/blob/dev/examples/graphs/3d/widgetvolumetric/main.cpp) failed with the same reason...
    // Q3DScatterWidgetItem [doc example](https://doc.qt.io/qt-6/q3dscatterwidgetitem.html) also failed with the same reason
    // graph->setWidget(quickWidget);

    qDebug() << quickWidget->status();
    qDebug() << quickWidget->engine();
    qDebug() << quickWidget->engine()->baseUrl();
    const QString qmlData = QLatin1StringView(R"QML(
        import QtQuick;
        import QtGraphs;

        Scatter3D
        {
            anchors.fill: parent;
        }
    )QML");
    auto* component = new QQmlComponent(quickWidget->engine(), nullptr);
    component->setData(qmlData.toUtf8(), QUrl());
    qDebug() << component->status();  // QQmlComponent::Ready
    qDebug() << component->isReady(); // true
    qDebug() << component->create();  // QQmlComponent: Component is not ready -- WTF!
    qDebug() << component->errors();

    //auto w = new MainWindow();
    //w->show();

    return app.exec();
}
