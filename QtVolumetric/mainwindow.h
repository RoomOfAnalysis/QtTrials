#pragma once

#include <QWidget>

namespace Ui
{
    class MainWindow;
}

class Vol;
class Q3DScatterWidgetItem;
class QQuickWidget;

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

private:
    void setup_connections();

private:
    Ui::MainWindow* ui = nullptr;
    Q3DScatterWidgetItem* m_graph = nullptr;
    QQuickWidget* m_quickWidget = nullptr;
    Vol* m_vol = nullptr;
};