#pragma once

#include <QWidget>

class ImageWidget: public QWidget
{
    Q_OBJECT
public:
    explicit ImageWidget(QWidget* parent = nullptr);
    ~ImageWidget();
    bool empty() const;
    QImage image() const;
    void setImage(const QImage& i);
    qreal rotation() const;
    void setRotation(qreal r);
    QPoint mapToImage(QPoint p);
    QSize imageSize() const;

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent* event) override;

private:
    QImage m_image;
    qreal m_rotation = 0;
};