#pragma once

#include <QLabel>

class MagnificationGlass: public QLabel
{
	Q_OBJECT
public:
    explicit MagnificationGlass(QWidget* parent = nullptr);
    ~MagnificationGlass() = default;

    bool is_active() const { return m_active; }
    void set_active(bool flag) { m_active = flag; }

    void magnify(const QImage& fromImage, const QPoint& fromPosition, QSize scopeSize = {50, 50}, double magnification = 2.);

private:
    bool m_active{false};
};