#include "MainWindow.h"

#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);

    setupConnections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::on_file_open);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::on_file_save);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::on_file_reset);

    connect(ui->rect_btn, &QPushButton::clicked, [this]() { ui->board->shape() = Board::Shape::RECT; });
    connect(ui->circle_btn, &QPushButton::clicked, [this]() { ui->board->shape() = Board::Shape::CIRCLE; });
    connect(ui->ellipse_btn, &QPushButton::clicked, [this]() { ui->board->shape() = Board::Shape::ELLIPSE; });

    connect(ui->undo_btn, &QPushButton::clicked, this, &MainWindow::on_undo);
    connect(ui->redo_btn, &QPushButton::clicked, this, &MainWindow::on_redo);
}

void MainWindow::on_file_open()
{
    ui->board->openImg();
}

void MainWindow::on_file_save()
{
    ui->board->saveImg();
}

void MainWindow::on_file_reset()
{
    ui->board->reset();
}

void MainWindow::on_redo()
{
    ui->board->redo();
}

void MainWindow::on_undo()
{
    ui->board->undo();
}
