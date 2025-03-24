#pragma once

#include <QWidget>

namespace Ui
{
    class CMPViewer;
}

class CMPViewer: public QWidget
{
    Q_OBJECT
public:
    explicit CMPViewer(QWidget* parent = nullptr);
    ~CMPViewer();

    QImage getOriginImage() const;
    QImage getGeneratedImage() const;

    void setOriginImage(QImage);
    void setGeneratedImage(QImage);

    void selectViewer(int index);

    void hideGeneratedViewer();

signals:
    void imgGenerated(QString img_path) const;

public slots:
    virtual void genImg() {}

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

    Ui::CMPViewer* ui;
    QImage curr_img{}, gen_img{};
    int curr_idx = -1;
    QWidget* viewers[2]{nullptr, nullptr};
    bool sync = false;
};