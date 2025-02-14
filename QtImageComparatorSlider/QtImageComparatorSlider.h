#pragma once

#include <QSplitter>

class QtImageComparatorSlider: public QSplitter
{
    Q_OBJECT
public:
    explicit QtImageComparatorSlider(QWidget* parent = nullptr);
    ~QtImageComparatorSlider();

    void setLeftPixmap(QPixmap p);
    void setRightPixmap(QPixmap p);

protected:
    QSize sizeHint() const override;
    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent* ev) override;

private:
    class QtImageLabel;

    QPixmap left_pix, right_pix;
    QSize curr_sz;
    QtImageLabel* left_label = nullptr;
    QtImageLabel* right_label = nullptr;
};