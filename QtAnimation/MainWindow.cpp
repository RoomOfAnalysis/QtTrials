#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QTimer>

#include "waitingspinnerwidget.h"

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setup_connections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup_connections()
{
    connect(ui->actionpage1, &QAction::triggered, this, [this] {
        // simple label as animation widget
        // or use a loading gif?
        auto* label = new QLabel(ui->stackedWidget->currentWidget());
        label->setStyleSheet("background: blue");
        label->setText("loading 1");
        label->resize(label->parentWidget()->size());
        label->show();
        label->activateWindow();
        label->raise();

        QTimer::singleShot(1000, [this, label]() {
            delete label;
            ui->stackedWidget->setCurrentIndex(0);
        });
    });
    connect(ui->actionpage2, &QAction::triggered, this, [this] {
        auto* spinner = new WaitingSpinnerWidget(ui->stackedWidget->currentWidget());
        spinner->setRoundness(70.0);
        spinner->setMinimumTrailOpacity(15.0);
        spinner->setTrailFadePercentage(70.0);
        spinner->setNumberOfLines(12);
        spinner->setLineLength(10);
        spinner->setLineWidth(5);
        spinner->setInnerRadius(10);
        spinner->setRevolutionsPerSecond(1);
        spinner->setColor(QColor(81, 4, 71));
        spinner->setText("loading 2");
        spinner->setTextColor(Qt::cyan);
        spinner->start();

        QTimer::singleShot(1000, [this, spinner]() {
            spinner->stop();
            delete spinner;
            ui->stackedWidget->setCurrentIndex(1);
        });
    });
}