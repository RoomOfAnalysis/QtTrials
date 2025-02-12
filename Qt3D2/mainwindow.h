#pragma once

#include <QWidget>

namespace Ui
{
    class MainWindow;
}

class Q3DScatterWidgetItem;
class QQuickWidget;
class QCustom3DItem;

class MainWindow final: public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    MainWindow(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;
    ~MainWindow() override;

    void loadMesh(QString path);
    void loadTexture(QString path);

private:
    void setup_connections();

private:
    Ui::MainWindow* ui = nullptr;
    Q3DScatterWidgetItem* m_graph = nullptr;
    QQuickWidget* m_quickWidget = nullptr;
    QCustom3DItem* m_item = nullptr;
};