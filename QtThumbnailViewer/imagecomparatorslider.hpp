#pragma once

#include <QSplitter>

class QLabel;

class ImageComparatorSlider: public QSplitter
{
    Q_OBJECT
public:
    explicit ImageComparatorSlider(QWidget* parent = nullptr);
    ~ImageComparatorSlider();

    void setPixmap1(QPixmap p);
    void setPixmap2(QPixmap p);

protected:
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent* ev) override;

private:
    void recalculateGeometry();

private:
    QPixmap pix1, pix2;
    QSize curr_sz;
    QLabel* label1 = nullptr;
    QLabel* label2 = nullptr;
};