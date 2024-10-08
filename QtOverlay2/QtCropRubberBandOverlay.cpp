#include "QtCropRubberBandOverlay.h"

#include <QRubberBand>
#include <QLayout>
#include <QSizeGrip>
#include <QMouseEvent>
#include <QMenu>
#include <QFileDialog>

#include "../QtImageViewer/ImageOps.h"

QtCropRubberBandOverlay::QtCropRubberBandOverlay(QtOverlayWidgetContainer* parent): QtOverlayWidget(parent)
{
    setWindowFlags(Qt::SubWindow);
    setFocusPolicy(Qt::ClickFocus);
    setPosition(QtOverlayWidgetPosition::CENTER);
    setStyleSheet("background-color:transparent");

    QVBoxLayout* main = new QVBoxLayout(this);
    main->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* top = new QHBoxLayout();
    top->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* bottom = new QHBoxLayout();
    bottom->setContentsMargins(0, 0, 0, 0);

    QSizeGrip* grip_tl = new QSizeGrip(this);
    QSizeGrip* grip_tr = new QSizeGrip(this);
    QSizeGrip* grip_bl = new QSizeGrip(this);
    QSizeGrip* grip_br = new QSizeGrip(this);

    top->addWidget(grip_tl, 0, Qt::AlignTop | Qt::AlignLeft);
    top->addWidget(grip_tr, 1, Qt::AlignTop | Qt::AlignRight);
    bottom->addWidget(grip_bl, 0, Qt::AlignBottom | Qt::AlignLeft);
    bottom->addWidget(grip_br, 1, Qt::AlignBottom | Qt::AlignRight);

    main->addLayout(top);
    main->addLayout(bottom);

    m_rubberband = new QRubberBand(QRubberBand::Rectangle, this);
    m_rubberband->setStyleSheet("background-color: rgba(152, 245, 249, 50%)");
    m_rubberband->show();

    if (parent) setContainerSize(parent->size());
}

QtCropRubberBandOverlay::~QtCropRubberBandOverlay() = default;

void QtCropRubberBandOverlay::show()
{
    setFocus();

    QtOverlayWidget::show();
    adjustSize();
    recalculateGeometry();
}

void QtCropRubberBandOverlay::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::MiddleButton)
    {
        setCursor(Qt::CursorShape::OpenHandCursor);
        m_last_mouse_pos = event->pos();
    }
    else if (event->buttons() & Qt::RightButton)
    {
        QMenu context_menu(this);
        context_menu.setStyleSheet("background-color: white");
        QAction action_crop_and_save("CropAndSave", &context_menu);
        connect(&action_crop_and_save, &QAction::triggered, [this]() {
            if (auto* p = parent(); p)
            {
                // FIXME: stupid trick... should use coordinates mapping and crop the real image...
                m_rubberband->hide();
                auto img = cropped(static_cast<QWidget*>(p)->grab().toImage(), geometry());
                m_rubberband->show();
                auto filepath = QFileDialog::getSaveFileName(this, tr("Save File"), "cropped.png",
                                                             tr("Images (*.png *.xpm *.jpg)"));
                if (!filepath.isEmpty() && !img.save(filepath))
                    qFatal() << "failed to save cropped image to" << filepath;
            }
        });
        context_menu.addAction(&action_crop_and_save);
        context_menu.exec(cursor().pos());
    }
    event->accept();
}

void QtCropRubberBandOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::RightButton)
    {
        auto geom = geometry();
        geom.setTopLeft(mapToGlobal(geom.topLeft()));
        geom.setBottomRight(mapToGlobal(geom.bottomRight()));
        emit selectionChanged(geom);
    }
    setCursor(Qt::CursorShape::ArrowCursor);
}

void QtCropRubberBandOverlay::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() & Qt::MiddleButton)
    {
        auto curr_pos = event->pos();
        move(pos() + curr_pos - m_last_mouse_pos);
        m_last_mouse_pos = curr_pos;
    }
    event->accept();
}

void QtCropRubberBandOverlay::resizeEvent(QResizeEvent*)
{
    m_rubberband->resize(size());
    // global
    auto geom = geometry();
    geom.setTopLeft(mapToGlobal(geom.topLeft()));
    geom.setBottomRight(mapToGlobal(geom.bottomRight()));
    emit selectionChanged(geom);
}