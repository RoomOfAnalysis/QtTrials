#include "QtImageComparatorSlider.h"

#include <QLabel>
#include <QPixmap>
#include <QPaintEvent>

QtImageComparatorSlider::QtImageComparatorSlider(QWidget* parent)
    : QSplitter(parent), label1(new QLabel(this)), label2(new QLabel(this))
{
    label1->setMinimumSize(1, 1);
    label1->setScaledContents(false);
    label2->setMinimumSize(1, 1);
    label2->setScaledContents(false);
    addWidget(label1);
    addWidget(label2);
    setOrientation(Qt::Orientation::Horizontal);
    setOpaqueResize(true);

    curr_sz = size();
}

QtImageComparatorSlider::~QtImageComparatorSlider() = default;

void QtImageComparatorSlider::setPixmap1(QPixmap p)
{
    pix1 = p;
    recalculateGeometry();
}

void QtImageComparatorSlider::setPixmap2(QPixmap p)
{
    pix2 = p;
}

QSize QtImageComparatorSlider::sizeHint() const
{
    return pix1.size();
}

void QtImageComparatorSlider::resizeEvent(QResizeEvent* ev)
{
    QSplitter::resizeEvent(ev);
    recalculateGeometry();
}

void QtImageComparatorSlider::paintEvent(QPaintEvent* ev)
{
    if (pix1.isNull())
    {
        QSplitter::paintEvent(ev);
        return;
    }

    auto lp = pix1.scaled(curr_sz, Qt::KeepAspectRatio);
    auto rp = pix2.scaled(curr_sz, Qt::KeepAspectRatio);

    auto szs = sizes();
    auto lw = szs[0];
    auto rw = szs[1];

    if (orientation() == Qt::Orientation::Horizontal)
    {
        label1->setPixmap(lp.copy(QRect(0, 0, lw, curr_sz.height())));
        if (!pix2.isNull())
        {
            auto rp = pix2.scaled(curr_sz, Qt::KeepAspectRatio);
            label2->setPixmap(rp.copy(QRect(rp.width() - rw, 0, rw, curr_sz.height())));
        }
    }
    else
    {
        label1->setPixmap(lp.copy(QRect(0, 0, curr_sz.width(), lw)));
        if (!pix2.isNull())
        {
            auto rp = pix2.scaled(curr_sz, Qt::KeepAspectRatio);
            label2->setPixmap(rp.copy(QRect(0, curr_sz.height() - rw, curr_sz.width(), rw)));
        }
    }

    QSplitter::paintEvent(ev);
}

void QtImageComparatorSlider::recalculateGeometry()
{
    if (pix1.isNull())
    {
        curr_sz = size();
        return;
    }

    auto const& [pw, ph] = pix1.size();
    auto r = float(pw) / ph;

    auto const& [width, height] = size();
    auto width_n = qMin(width, int(height * r));
    auto height_n = qMin(height, int(width / r));

    curr_sz = QSize(width_n, height_n);

    auto hm = round(width - width_n) / 2;
    auto vm = round(height - height_n) / 2;
    setContentsMargins(hm, vm, hm, vm);
}
