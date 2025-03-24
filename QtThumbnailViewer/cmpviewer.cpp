#include "cmpviewer.hpp"
#include "ui_cmpviewer.h"

#include "mainwindow.hpp"

#include <QMouseEvent>

CMPViewer::CMPViewer(QWidget* parent): QWidget(parent), ui(new Ui::CMPViewer)
{
    ui->setupUi(this);
    ui->generatedImgViewer->hide();

    ui->originImgViewer->setFocusPolicy(Qt::ClickFocus);
    ui->originImgViewer->setStyleSheet("QWidget:focus{border-color: #E27521;}");
    ui->generatedImgViewer->setFocusPolicy(Qt::ClickFocus);
    ui->generatedImgViewer->setStyleSheet("QWidget:focus{border-color: #E27521;}");

    ui->originImgViewer->setProperty("index", 0);
    ui->originImgViewer->installEventFilter(this);
    ui->generatedImgViewer->setProperty("index", 1);
    ui->generatedImgViewer->installEventFilter(this);

    viewers[0] = ui->originImgViewer;
    viewers[1] = ui->generatedImgViewer;

    ui->stackedWidget->setCurrentIndex(0);
    connect(ui->icsBtn, &QPushButton::pressed, [this]() {
        ui->imageComapratorSlider->setPixmap1(QPixmap::fromImage(curr_img));
        ui->imageComapratorSlider->setPixmap2(QPixmap::fromImage(gen_img));
        ui->stackedWidget->setCurrentIndex(1);
    });
    connect(ui->syncCb, &QCheckBox::checkStateChanged, [this](Qt::CheckState cs) {
        if (cs == Qt::CheckState::Checked)
        {
            sync = true;
            if (ui->generatedImgViewer->isHidden()) return;
            ui->originImgViewer->loadImage(curr_img);
            ui->generatedImgViewer->loadImage(gen_img);
        }
        else
            sync = false;
    });
    connect(ui->sbsBtn, &QPushButton::pressed, [this]() { ui->stackedWidget->setCurrentIndex(0); });
    connect(ui->uhsCb, &QCheckBox::checkStateChanged, [this](Qt::CheckState cs) {
        ui->imageComapratorSlider->setOrientation(cs == Qt::CheckState::Checked ? Qt::Orientation::Horizontal :
                                                                                  Qt::Orientation::Vertical);
    });
}

CMPViewer::~CMPViewer()
{
    delete ui;
}

QImage CMPViewer::getOriginImage() const
{
    return curr_img;
}

QImage CMPViewer::getGeneratedImage() const
{
    return gen_img;
}

void CMPViewer::setOriginImage(QImage img)
{
    curr_img = img;
    ui->originImgViewer->loadImage(curr_img);
    if (!ui->generatedImgViewer->isHidden() && sync) ui->generatedImgViewer->loadImage(gen_img);
    ui->imageComapratorSlider->setPixmap1(QPixmap::fromImage(curr_img));
}

void CMPViewer::setGeneratedImage(QImage img)
{
    gen_img = img;
    ui->generatedImgViewer->loadImage(gen_img);
    if (ui->generatedImgViewer->isHidden())
    {
        if (sync) ui->originImgViewer->loadImage(curr_img);
        ui->generatedImgViewer->show();
    }
    ui->imageComapratorSlider->setPixmap2(QPixmap::fromImage(gen_img));
}

bool CMPViewer::eventFilter(QObject* obj, QEvent* event)
{
    static bool recursion_pretector = false;
    if (sync)
    {
        if (recursion_pretector) return false;
        if (event->type() == QEvent::Type::MouseButtonPress || event->type() == QEvent::Type::MouseMove ||
            event->type() == QEvent::Type::Wheel)
        {
            recursion_pretector = true;
            QApplication::sendEvent(obj == ui->originImgViewer ? ui->generatedImgViewer : ui->originImgViewer, event);
            recursion_pretector = false;
        }
    }

    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) selectViewer(obj->property("index").toInt());
    }
    return QObject::eventFilter(obj, event);
}

void CMPViewer::selectViewer(int index)
{
    curr_idx = index;

    auto* mw = MainWindow::GetInstance();
    mw->SetHist(qobject_cast<Image2DViewer*>(viewers[curr_idx])->getOriginImage());
}

void CMPViewer::hideGeneratedViewer()
{
    ui->generatedImgViewer->hide();
}
