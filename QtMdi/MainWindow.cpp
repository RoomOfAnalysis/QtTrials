#include "MainWindow.h"

#include <QListView>
#include <QMdiArea>
#include <QMenuBar>
#include <QFileDialog>
#include <QLabel>
#include <QMdiSubWindow>
#include <QListWidget>
#include <QLayout>

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), m_list(new QListWidget(this)), m_mdi(new QMdiArea(this))
{
    auto* menu = menuBar()->addMenu("File");
    auto* open_action = new QAction("Open", this);
    menu->addAction(open_action);
    connect(open_action, SIGNAL(triggered(bool)), this, SLOT(openImage()));

    m_list->setViewMode(QListView::IconMode);
    m_list->setFlow(QListView::TopToBottom);
    m_list->setMovement(QListView::Static);
    m_list->setResizeMode(QListView::Adjust);
    m_list->setIconSize(QSize(200, 150));
    m_list->setModelColumn(0);

    m_mdi->setViewMode(QMdiArea::SubWindowView);
    m_mdi->setActivationOrder(QMdiArea::CreationOrder);

    auto* layout = new QHBoxLayout();
    layout->addWidget(m_list);
    layout->addWidget(m_mdi);
    layout->setStretch(0, 1);
    layout->setStretch(1, 9);
    setCentralWidget(new QWidget(this));
    centralWidget()->setLayout(layout);
}

MainWindow::~MainWindow() = default;

void MainWindow::openImage()
{
    if (auto file =
            QFileDialog::getOpenFileName(this, "Open Image", QDir::currentPath(), tr("Image (*.png *.bmp *.jpg)"));
        !file.isEmpty() && !m_imgs.contains(file))
    {
        QFileInfo fi(file);
        auto file_name = fi.fileName();
        m_imgs.insert(file, QPixmap(file));
        auto icon = QIcon(m_imgs[file].scaled(QSize(200, 150), Qt::KeepAspectRatio));
        auto* item = new QListWidgetItem(icon, file_name);
        item->setData(Qt::UserRole, file);
        item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
        m_list->addItem(item);

        if (auto w = m_list->sizeHintForColumn(0) + 2 * m_list->frameWidth(); w != m_list->width())
            m_list->setFixedWidth(w);

        auto* img_viewer = new QLabel();
        auto* sub_window = m_mdi->addSubWindow(img_viewer);
        sub_window->setWindowTitle(file_name);
        sub_window->setWindowIcon(icon);
        sub_window->setAttribute(Qt::WA_DeleteOnClose);
        sub_window->setContentsMargins({0, 0, 0, 0});
        img_viewer->setPixmap(m_imgs[file].scaled(QSize(400, 300), Qt::KeepAspectRatio));
        sub_window->adjustSize();
        sub_window->show();
    }
}