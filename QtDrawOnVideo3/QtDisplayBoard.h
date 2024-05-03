#pragma once

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsItem>

class QGraphicsView;

class QtDisplayBoard: public QWidget
{
    Q_OBJECT

public:
    explicit QtDisplayBoard(QWidget* parent = nullptr);
    ~QtDisplayBoard();

    [[nodiscard]] QSize sizeHint() const override;

    [[nodiscard]] QGraphicsPixmapItem* getPixmapItem() const;
    [[nodiscard]] QGraphicsScene* getScene() const;

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    QGraphicsPixmapItem* m_pixmap_item = nullptr;
    QGraphicsScene* m_scene = nullptr;
    QGraphicsView* m_view = nullptr;
};