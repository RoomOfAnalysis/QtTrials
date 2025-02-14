#include "QtImageComparatorSlider.h"

#include <QLabel>
#include <QPixmap>
#include <QPaintEvent>

class QtImageComparatorSlider::QtImageLabel: public QLabel
{
public:
    explicit QtImageLabel(QWidget* parent = nullptr): QLabel(parent)
    {
        setMinimumSize(1, 1);
        setScaledContents(false);
    }
    ~QtImageLabel() = default;

    void setPixmap(QPixmap const& p)
    {
        m_pix = p;
        QLabel::setPixmap(p);
    }

    void setLeft(bool is_left) { m_is_left = is_left; }

protected:
    void resizeEvent(QResizeEvent* ev) override
    {
        if (m_is_left)
            QLabel::setPixmap(m_pix.copy(0, 0, ev->size().width(), ev->size().height()));
        else
            QLabel::setPixmap(
                m_pix.copy(m_pix.width() - ev->size().width(), 0, ev->size().width(), ev->size().height()));
    }

private:
    QPixmap m_pix;
    bool m_is_left = true;
};

QtImageComparatorSlider::QtImageComparatorSlider(QWidget* parent)
    : QSplitter(parent), left_label(new QtImageLabel(this)), right_label(new QtImageLabel(this))
{
    right_label->setLeft(false);
    addWidget(left_label);
    addWidget(right_label);
    setOrientation(Qt::Orientation::Horizontal);
    setOpaqueResize(true);

    curr_sz = size();
}

QtImageComparatorSlider::~QtImageComparatorSlider() = default;

void QtImageComparatorSlider::setLeftPixmap(QPixmap p)
{
    left_pix = p;
}

void QtImageComparatorSlider::setRightPixmap(QPixmap p)
{
    right_pix = p.isNull() ? left_pix : p;
}

QSize QtImageComparatorSlider::sizeHint() const
{
    return left_pix.size();
}

void QtImageComparatorSlider::resizeEvent(QResizeEvent* ev)
{
    auto [pw, ph] = left_pix.size();
    auto r = float(pw) / ph;

    QWidget::resizeEvent(ev);
    auto [width, height] = ev->size();
    auto width_n = qMin(width, int(height * r));
    auto height_n = qMin(height, int(width / r));

    curr_sz = QSize(width_n, height_n);

    auto hm = round(width - width_n) / 2;
    auto vm = round(height - height_n) / 2;
    setContentsMargins(hm, vm, hm, vm);
}

void QtImageComparatorSlider::paintEvent(QPaintEvent* ev)
{
    if (left_pix.isNull())
    {
        QSplitter::paintEvent(ev);
        return;
    }

    auto lp = left_pix.scaled(curr_sz, Qt::KeepAspectRatio);
    auto rp = right_pix.scaled(curr_sz, Qt::KeepAspectRatio);

    auto szs = sizes();
    auto lw = szs[0];
    auto rw = szs[1];

    left_label->setPixmap(lp.copy(QRect(0, 0, lw, curr_sz.height())));
    right_label->setPixmap(rp.copy(QRect(rp.width() - rw, 0, rw, curr_sz.height())));

    QSplitter::paintEvent(ev);
}
