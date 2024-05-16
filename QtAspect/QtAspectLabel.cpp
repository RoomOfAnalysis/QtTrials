#include "QtAspectLabel.h"

QtAspectLabel::QtAspectLabel(QWidget* parent): QLabel(parent)
{
    setMinimumSize(1, 1);
    setScaledContents(false);
}

QtAspectLabel::~QtAspectLabel() = default;

int QtAspectLabel::heightForWidth(int width) const
{
    return m_pix.isNull() ? height() : ((qreal)m_pix.height() * width) / m_pix.width();
}

QSize QtAspectLabel::sizeHint() const
{
    int w = width();
    return QSize(w, heightForWidth(w));
}

QPixmap QtAspectLabel::scaledPixmap() const
{
    return m_pix.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void QtAspectLabel::setPixmap(QPixmap const& p)
{
    m_pix = p;
    QLabel::setPixmap(scaledPixmap());
}

void QtAspectLabel::resizeEvent(QResizeEvent*)
{
    if (!m_pix.isNull()) QLabel::setPixmap(scaledPixmap());
}