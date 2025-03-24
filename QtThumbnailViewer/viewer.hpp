#pragma once

#include <QWidget>

namespace Ui
{
    class Viewer;
}

class Viewer: public QWidget
{
    Q_OBJECT
public:
    explicit Viewer(QWidget* parent = nullptr);
    ~Viewer();

    void loadImage(QImage img);
    QImage getImage() const;

    void setImages(QList<QImage> const& images, QStringList const& paths);
    static QList<QImage> loadImages(QStringList const& paths);

public slots:
    void render();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::Viewer* ui;
    QStringList img_paths;
};