#pragma once

#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

class QtAspectLabel: public QLabel
{
    Q_OBJECT
public:
    explicit QtAspectLabel(QWidget* parent = nullptr);
    ~QtAspectLabel();
    int heightForWidth(int width) const override;
    QSize sizeHint() const override;
    QPixmap scaledPixmap() const;
public slots:
    void setPixmap(QPixmap const&);
    void resizeEvent(QResizeEvent*) override;

private:
    QPixmap m_pix;
};