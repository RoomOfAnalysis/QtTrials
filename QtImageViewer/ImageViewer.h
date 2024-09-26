#pragma once

#include <QMainWindow>

class Image;
class ImageView;

namespace Ui
{
    class ImageViewer;
}

class ImageViewer: public QMainWindow
{
    Q_OBJECT
public:
    explicit ImageViewer(QWidget* parent = nullptr);
    ~ImageViewer();

private slots:
    void on_file_open();

private:
    void setup_connections();

private:
    Ui::ImageViewer* ui;
    std::unique_ptr<Image> m_image = nullptr;
    ImageView* m_image_view = nullptr;
};