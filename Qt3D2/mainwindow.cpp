#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <Q3DScatterWidgetItem>
#include <QQuickWidget>
#include <QCustom3DItem>
#include <QFileDialog>

MainWindow::MainWindow(QWidget* parent): QWidget(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_graph = new Q3DScatterWidgetItem();
    m_quickWidget = new QQuickWidget();
    m_graph->setWidget(m_quickWidget);
    m_graph->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_graph->widget()->setFocusPolicy(Qt::StrongFocus);

    m_item = new QCustom3DItem();
    m_graph->addCustomItem(m_item);

    new QGridLayout(ui->display);
    ui->display->layout()->addWidget(m_quickWidget);

    setup_connections();
}

MainWindow::~MainWindow()
{
    delete m_item;
    delete m_graph;
    delete m_quickWidget;
}

void MainWindow::loadMesh(QString path)
{
    // https://doc.qt.io/qt-6/qcustom3ditem.html#meshFile-prop
    // The item in the file must be in mesh format. The other types can be converted by Balsam asset import tool. The mesh files are recommended to include vertices, normals, and UVs.

    m_item->setMeshFile(path);
}

void MainWindow::loadTexture(QString path)
{
    m_item->setTextureFile(path);
}

void MainWindow::setup_connections()
{
    connect(ui->loadMeshBtn, &QPushButton::pressed, [this]() {
        auto path = QFileDialog::getOpenFileName(this, tr("Load Mesh"), QDir::homePath(), tr("*obj;;*stl;;*.ply"));
        if (path.isEmpty()) return;
        loadMesh(path);
    });

    connect(ui->loadTextureBtn, &QPushButton::pressed, [this]() {
        auto path =
            QFileDialog::getOpenFileName(this, tr("Load Mesh"), QDir::homePath(), tr("*jpg;;*jpeg;;*.png;;*.bmp"));
        if (path.isEmpty()) return;
        loadTexture(path);
    });
}