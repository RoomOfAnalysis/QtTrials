#include <QApplication>
#include <Q3DScatterWidgetItem>
#include <QQuickWidget>
#include <QQmlComponent>

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    //Q3DScatterWidgetItem* graph = new Q3DScatterWidgetItem;
    //auto quickWidget = new QQuickWidget;
    //// FIXME: always failed here with ERROR: QQmlComponent: Component is not ready
    //// even the [official example](https://github.com/qt/qtgraphs/blob/dev/examples/graphs/3d/widgetvolumetric/main.cpp) failed with the same reason...
    //// Q3DScatterWidgetItem [doc example](https://doc.qt.io/qt-6/q3dscatterwidgetitem.html) also failed with the same reason
    //graph->setWidget(quickWidget);

    //qDebug() << quickWidget->status();
    //qDebug() << quickWidget->engine();
    //qDebug() << quickWidget->engine()->baseUrl();
    //const QString qmlData = QLatin1StringView(R"QML(
    //    import QtQuick;
    //    import QtGraphs;

    //    Scatter3D
    //    {
    //        anchors.fill: parent;
    //    }
    //)QML");
    //auto* component = new QQmlComponent(quickWidget->engine(), nullptr);
    //component->setData(qmlData.toUtf8(), QUrl());
    //qDebug() << component->status();  // QQmlComponent::Ready
    //qDebug() << component->isReady(); // true
    //qDebug() << component->create();  // QQmlComponent: Component is not ready -- WTF!
    //qDebug() << component->errors();

    //QQmlComponent repeaterComponent(quickWidget->engine());
    //repeaterComponent.setData("import QtQuick3D; Repeater3D{}", QUrl());
    //qDebug() << repeaterComponent.status(); // QQmlComponent::Error
    //qDebug() << repeaterComponent.errors(); // QList(<Unknown File>:1:1: module "QtQuick3D" plugin "qquick3dplugin" not found)
    //// so the problem is here!
    //// after adding qquick3dplugin.dll
    //qDebug() << repeaterComponent.isReady(); // true
    //qDebug() << repeaterComponent.create();  // QQuick3DRepeater(0x11aad85ee30)

    // FIXME: qt volumetric example works well now, but mine still failed after `Vol::loadVolumeData`
    auto w = new MainWindow();
    w->show();

    return app.exec();
}
