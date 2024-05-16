#pragma once

#include <QWidget>

class QtAspectWidget: public QWidget
{
    Q_OBJECT
public:
    QtAspectWidget(QWidget* parent = nullptr, double aspect_ratio = /*width / height*/ 1.);
    ~QtAspectWidget();

    // override resizeEvent or simply use eventFilter
    void resizeEvent(QResizeEvent* event) override;

private:
    double m_aspect_ratio{1.};
};