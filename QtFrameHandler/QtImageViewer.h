#pragma once

#include <QWidget>
#include <QImage>
#include <QPainter>

class QtImageViewer: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QImage image READ image WRITE setImage USER true)

public:
    explicit QtImageViewer(QWidget* parent = nullptr): QWidget(parent)
    {
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet("background-color: black;");
    }
    ~QtImageViewer() = default;

    Q_SLOT void setImage(const QImage& img)
    {
        if (!painted) return;
        if (m_img.size() == img.size() && m_img.format() == img.format() && m_img.bytesPerLine() == img.bytesPerLine())
            std::copy_n(img.bits(), img.sizeInBytes(), m_img.bits());
        else
            m_img = img.copy();
        painted = false;
        update();
    }
    [[nodiscard]] QImage image() const { return m_img; }

private:
    void paintEvent(QPaintEvent*)
    {
        QPainter p(this);
        if (!m_img.isNull())
        {
            setAttribute(Qt::WA_OpaquePaintEvent);
            auto img = m_img.scaled(width(), height(), Qt::KeepAspectRatio);
            p.drawImage((width() - img.width()) / 2, (height() - img.height()) / 2, img);
            painted = true;
        }
    }

private:
    bool painted = true;
    QImage m_img;
};