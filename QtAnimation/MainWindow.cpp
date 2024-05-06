#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QTimer>

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
        auto* label = new QLabel(ui->stackedWidget->currentWidget());
        label->setStyleSheet("background: white");
        label->setText("loading 2");
        label->resize(label->parentWidget()->size());
        label->show();
        label->activateWindow();
        label->raise();

        QTimer::singleShot(1000, [this, label]() {
            delete label;
            ui->stackedWidget->setCurrentIndex(0);
        });
    });
}